# Table of Content

* [总览](##总览)
* [环境](##环境)
* [Web Assembly介绍](##chain分析)
* [WABT虚拟机](##chainbase分析)
* [WABT虚拟机](##chainbase分析)
* [WABT虚拟机](##chainbase分析)
* [WABT虚拟机](##chainbase分析)
* [WABT虚拟机](##chainbase分析)
* [WABT虚拟机](##chainbase分析)

## 总览
  
  本部分主要致力于EOS中智能合约相关部分的源码分析，并在此基础上实现并发和水平扩展的智能合约解释器.
  
## Web Assembly介绍
  Web Assembly(wasm)是一种字节码，C/C++,javascript等语言都可以被相关的编译器编译成wasm模块并由wasm解释器执行.
  
## EOS虚拟机架构介绍
EOS在1.3.0版本中添加了**WABT虚拟机**作为新的智能合约解释器。目前，EOS一共使用了3种解释器

    WAVM
    BINARYEN
    WABT
    
EOS使用配置项“wasm-runtime”来管理虚拟机解释器的选择。目前默认为BINARYEN。不过官网说明WABT更加快。从代码上看，WABT的代码
也更加清晰和严谨，因此我们之后会通过WABT来分析EOS智能合约的执行过程。

EOS执行智能合约的主要步骤是


    1.启动时注册API函数
    2.接收到智能合约的调用请求时，初始化虚拟机。
    3.如果是第一次运行，修改智能合约的opcode部分（加入CPU占用，内存占用限制。修改循环调用部分）。
    4.如果不是第一次运行，把内存清空。
    5.执行智能合约。如果本合约调用了另一个智能合约，记录下合约。
    6.重复第2步，执行调用的只能合约
    7.智能合约的调用结果，会放在EOS的fork_db里,并在新的区块到来时入链或者回滚.
    
EOS在`wasm_interface.cpp`里，向3个虚拟机解释器，注册了API函数

  ``` cpp
  REGISTER_INTRINSICS(memory_api,
     (memcpy,                 int(int, int, int)  )
     (memmove,                int(int, int, int)  )
     (memcmp,                 int(int, int, int)  )
     (memset,                 int(int, int, int)  )
  );


  ```

REGISTER_INTRINSICS是EOS自定义的宏，其中引用了经典BOOST_PP_SEQ_FOR_EACH系列宏，具体可以参考BOOST相关文档。
以memset为例，REGISTER_INTRINSICS宏展开是这样的。

``` cpp
  static Intrinsics::Function __intrinsic_fn507(
      "env"
      "."
      "memset",
      eosio::chain::webassembly::wavm::wasm_function_type_provider<
          int(int, int, int)>::type(),
      (void*)eosio::chain::webassembly::wavm::intrinsic_function_invoker_wrapper<
          int(int, int, int),
          decltype(&memory_api::memset)>::type::fn<&memory_api::memset>());
  static eosio::chain::webassembly::binaryen::intrinsic_registrator
      __binaryen_intrinsic_fn508(
          "env"
          "."
          "memset",
          eosio::chain::webassembly::binaryen::intrinsic_function_invoker_wrapper<
              decltype(&memory_api::memset)>::type::fn<&memory_api::memset>());
  static eosio::chain::webassembly::wabt_runtime::intrinsic_registrator
      __wabt_intrinsic_fn509(
          "env",
          "memset",
          eosio::chain::webassembly::wabt_runtime::wabt_function_type_provider<
              int(int, int, int)>::type(),
          eosio::chain::webassembly::wabt_runtime::
              intrinsic_function_invoker_wrapper<decltype(
                  &memory_api::memset)>::type::fn<&memory_api::memset>());

  ```

其中__intrinsic_fn507是向WAVM注册，__binaryen_intrinsic_fn508是向binaryen注册，__wabt_intrinsic_fn509是向wabt注册
在之后的章节里，会详细的讲述API的注册过程。


EOS会把调用过的虚拟机上下文保存在

`map<digest_type, std::unique_ptr<wasm_instantiated_module_interface>> instantiation_cache`


结构里
这个map的KEY是智能合约的ID。当调用智能合约时,EOS会调用`get_instantiated_module`函数

``` cpp
  std::unique_ptr<wasm_instantiated_module_interface>& get_instantiated_module( const digest_type& code_id,
                                                                                      const shared_string& code,
                                                                                      transaction_context& trx_context )
        {
           auto it = instantiation_cache.find(code_id);
           if(it == instantiation_cache.end()) {
              auto timer_pause = fc::make_scoped_exit([&](){
                 trx_context.resume_billing_timer();
              });
              trx_context.pause_billing_timer();
              IR::Module module;
              try {
                 Serialization::MemoryInputStream stream((const U8*)code.data(), code.size());
                 WASM::serialize(stream, module);
                 module.userSections.clear();
              } catch(const Serialization::FatalSerializationException& e) {
                 EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
              } catch(const IR::ValidationException& e) {
                 EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
              }
  
              wasm_injections::wasm_binary_injection injector(module);
              injector.inject();
  
              std::vector<U8> bytes;
              try {
                 Serialization::ArrayOutputStream outstream;
                 WASM::serialize(outstream, module);
                 bytes = outstream.getBytes();
              } catch(const Serialization::FatalSerializationException& e) {
                 EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
              } catch(const IR::ValidationException& e) {
                 EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
              }
              it = instantiation_cache.emplace(code_id, runtime_interface->instantiate_module((const char*)bytes.data(), bytes.size(), parse_initial_memory(module))).first;
           }
           return it->second;
        }

  ```
  
  该函数查找当前的智能合约是否缓存在内存中.如果没有,则初始化智能合约的虚拟机模块.
  在之后的章节里，会详细的讲述虚拟机初始化过程。
  
  
  在上边的代码中.
  
  ```cpp
  wasm_injections::wasm_binary_injection injector(module);
  injector.inject();
  ```
  
  是对智能合约的opcode进行注入.加入了CPU时间,RAM,NET的占用等控制.这个是EOS合约执行的基础.
  
  ```cpp
              void inject() {
              _module_injectors.inject( *_module );
              // inject checktime first
              injector_utils::add_import<ResultType::none>( *_module, u8"checktime", checktime_injection::chktm_idx );
  
              for ( auto& fd : _module->functions.defs ) {
                 wasm_ops::EOSIO_OperatorDecoderStream<pre_op_injectors> pre_decoder(fd.code);
                 wasm_ops::instruction_stream pre_code(fd.code.size()*2);
  
                 while ( pre_decoder ) {
                    auto op = pre_decoder.decodeOp();
                    if (op->is_post()) {
                       op->pack(&pre_code);
                       op->visit( { _module, &pre_code, &fd, pre_decoder.index() } );
                    }
                    else {
                       op->visit( { _module, &pre_code, &fd, pre_decoder.index() } );
                       if (!(op->is_kill()))
                          op->pack(&pre_code);
                    }
                 }
                 fd.code = pre_code.get();
              }
              for ( auto& fd : _module->functions.defs ) {
                 wasm_ops::EOSIO_OperatorDecoderStream<post_op_injectors> post_decoder(fd.code);
                 wasm_ops::instruction_stream post_code(fd.code.size()*2);
  
                 wasm_ops::op_types<>::call_t chktm; 
                 chktm.field = injector_utils::injected_index_mapping.find(checktime_injection::chktm_idx)->second;
                 chktm.pack(&post_code);
  
                 while ( post_decoder ) {
                    auto op = post_decoder.decodeOp();
                    if (op->is_post()) {
                       op->pack(&post_code);
                       op->visit( { _module, &post_code, &fd, post_decoder.index() } );
                    }
                    else {
                       op->visit( { _module, &post_code, &fd, post_decoder.index() } );
                       if (!(op->is_kill()))
                          op->pack(&post_code);
                    }
                 }
                 fd.code = post_code.get();
              }
           }
  ```
  
  这个函数里对loop和call等opcode做了注入处理.在之后的章节里，会详细的讲述注入过程。