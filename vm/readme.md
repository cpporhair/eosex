# Table of Content

* [总览](##总览)
* [Web Assembly介绍](##Web Assembly介绍)
* [EOS虚拟机架构介绍](##EOS虚拟机架构介绍)
* [WABT虚拟机](##chainbase分析)
* [API注册](##chainbase分析)
* [初始化虚拟机](##chainbase分析)
* [setcode](##chainbase分析)
* [注入opcode](##chainbase分析)
* [执行智能合约](##chainbase分析)
* [并发,可扩展的智能合约解释器](##chainbase分析)

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
    
EOS在`wasm_interface.cpp`里，向3个虚拟机解释器，注册了API函数,比如:

  ``` cpp
  ...
  REGISTER_INTRINSICS(memory_api,
     (memcpy,                 int(int, int, int)  )
     (memmove,                int(int, int, int)  )
     (memcmp,                 int(int, int, int)  )
     (memset,                 int(int, int, int)  )
  );
  ...
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
这个map的KEY是智能合约的ID。合约的ID,是在setcode里根据合约的二进制代码作hash256运算得出的.

```cpp
   ...
   fc::sha256 code_id; /// default ID == 0

   if( act.code.size() > 0 ) {
     code_id = fc::sha256::hash( act.code.data(), (uint32_t)act.code.size() );
     wasm_interface::validate(context.control, act.code);
   }
   ...
```

当调用智能合约时,EOS会调用`get_instantiated_module`函数

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
  
  ## WABT虚拟机
  
  
  WABT是WebAssembly推出的开源二进制工具包.
  源码地址在https://github.com/WebAssembly/wabt
  
  WABT包含下列工具:
  
  ```
  wat2wasm        将"WebAssembly text format"转换到"WebAssembly binary format"
  wasm2wat        将"WebAssembly binary format"转换到"WebAssembly text format"
  wasm-objdump    类似objdump
  wasm-interp     命令行形式的wasm二进制文件解释器.wasm
  wat-desugar     命令行形式的wasm文本文件解释器.wat/wast
  wasm2c          wasm转换到c文件
  ```

我们参考 src/test-interp.cc 的一段代码
```cpp
class HostMemoryTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    //注册模块:EOS传入合约ID
    interp::HostModule* host_module = env_.AppendHostModule("host");
    //创建执行器
    executor_ = MakeUnique<interp::Executor>(&env_);
    //注册内存(因为测试的WASM使用了内存,EOS不支持合约使用节点的内存,所以没有注册内存)
    //对应wast里的(import "host" "mem" (memory $mem 1))
    std::pair<interp::Memory*, Index> pair =
        host_module->AppendMemoryExport("mem", Limits(1));

    using namespace std::placeholders;

    //注册函数fill_buf
    host_module->AppendFuncExport(
        "fill_buf", {{Type::I32, Type::I32}, {Type::I32}},
        std::bind(&HostMemoryTest::FillBufCallback, this, _1, _2, _3, _4));
    //注册函数fill_buf
    host_module->AppendFuncExport(
        "buf_done", {{Type::I32, Type::I32}, {}},
        std::bind(&HostMemoryTest::BufDoneCallback, this, _1, _2, _3, _4));
    memory_ = pair.first;
  }

  virtual void TearDown() {
    executor_.reset();
  }

  Result LoadModule(const std::vector<uint8_t>& data) {
    Errors errors;
    ReadBinaryOptions options;
    //读取wasm,EOS使用这个函数读取传来的合约
    return ReadBinaryInterp(&env_, data.data(), data.size(), options, &errors,
                            &module_);
  }

  std::string string_data;

  interp::Result FillBufCallback(const interp::HostFunc* func,
                                 const interp::FuncSignature* sig,
                                 const interp::TypedValues& args,
                                 interp::TypedValues& results) {
    // (param $ptr i32) (param $max_size i32) (result $size i32)
    EXPECT_EQ(2u, args.size());
    EXPECT_EQ(Type::I32, args[0].type);
    EXPECT_EQ(Type::I32, args[1].type);
    EXPECT_EQ(1u, results.size());
    EXPECT_EQ(Type::I32, results[0].type);

    uint32_t ptr = args[0].get_i32();
    uint32_t max_size = args[1].get_i32();
    uint32_t size = std::min(max_size, uint32_t(string_data.size()));

    EXPECT_LT(ptr + size, memory_->data.size());

    std::copy(string_data.begin(), string_data.begin() + size,
              memory_->data.begin() + ptr);

    results[0].set_i32(size);
    return interp::Result::Ok;
  }

  interp::Result BufDoneCallback(const interp::HostFunc* func,
                                 const interp::FuncSignature* sig,
                                 const interp::TypedValues& args,
                                 interp::TypedValues& results) {
    // (param $ptr i32) (param $size i32)
    EXPECT_EQ(2u, args.size());
    EXPECT_EQ(Type::I32, args[0].type);
    EXPECT_EQ(Type::I32, args[1].type);
    EXPECT_EQ(0u, results.size());

    uint32_t ptr = args[0].get_i32();
    uint32_t size = args[1].get_i32();

    EXPECT_LT(ptr + size, memory_->data.size());

    string_data.resize(size);
    std::copy(memory_->data.begin() + ptr, memory_->data.begin() + ptr + size,
              string_data.begin());

    return interp::Result::Ok;
  }

  interp::Environment env_;
  interp::Memory* memory_;
  interp::DefinedModule* module_;
  std::unique_ptr<interp::Executor> executor_;
};

}  // end of anonymous namespace

TEST_F(HostMemoryTest, Rot13) {
  // 这里是 WAST
  // (import "host" "mem" (memory $mem 1))
  // (import "host" "fill_buf" (func $fill_buf (param i32 i32) (result i32)))
  // (import "host" "buf_done" (func $buf_done (param i32 i32)))
  //
  // (func $rot13c (param $c i32) (result i32)
  //   (local $uc i32)
  //
  //   ;; No change if < 'A'.
  //   (if (i32.lt_u (get_local $c) (i32.const 65))
  //     (return (get_local $c)))
  //
  //   ;; Clear 5th bit of c, to force uppercase. 0xdf = 0b11011111
  //   (set_local $uc (i32.and (get_local $c) (i32.const 0xdf)))
  //
  //   ;; In range ['A', 'M'] return |c| + 13.
  //   (if (i32.le_u (get_local $uc) (i32.const 77))
  //     (return (i32.add (get_local $c) (i32.const 13))))
  //
  //   ;; In range ['N', 'Z'] return |c| - 13.
  //   (if (i32.le_u (get_local $uc) (i32.const 90))
  //     (return (i32.sub (get_local $c) (i32.const 13))))
  //
  //   ;; No change for everything else.
  //   (return (get_local $c))
  // )
  //
  // (func (export "rot13")
  //   (local $size i32)
  //   (local $i i32)
  //
  //   ;; Ask host to fill memory [0, 1024) with data.
  //   (call $fill_buf (i32.const 0) (i32.const 1024))
  //
  //   ;; The host returns the size filled.
  //   (set_local $size)
  //
  //   ;; Loop over all bytes and rot13 them.
  //   (block $exit
  //     (loop $top
  //       ;; if (i >= size) break
  //       (if (i32.ge_u (get_local $i) (get_local $size)) (br $exit))
  //
  //       ;; mem[i] = rot13c(mem[i])
  //       (i32.store8
  //         (get_local $i)
  //         (call $rot13c
  //           (i32.load8_u (get_local $i))))
  //
  //       ;; i++
  //       (set_local $i (i32.add (get_local $i) (i32.const 1)))
  //       (br $top)
  //     )
  //   )
  //
  //   (call $buf_done (i32.const 0) (get_local $size))
  // )
  
  // data 是 WASM
  std::vector<uint8_t> data = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x14, 0x04, 0x60,
      0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x60, 0x02, 0x7f, 0x7f, 0x00, 0x60, 0x01,
      0x7f, 0x01, 0x7f, 0x60, 0x00, 0x00, 0x02, 0x2d, 0x03, 0x04, 0x68, 0x6f,
      0x73, 0x74, 0x03, 0x6d, 0x65, 0x6d, 0x02, 0x00, 0x01, 0x04, 0x68, 0x6f,
      0x73, 0x74, 0x08, 0x66, 0x69, 0x6c, 0x6c, 0x5f, 0x62, 0x75, 0x66, 0x00,
      0x00, 0x04, 0x68, 0x6f, 0x73, 0x74, 0x08, 0x62, 0x75, 0x66, 0x5f, 0x64,
      0x6f, 0x6e, 0x65, 0x00, 0x01, 0x03, 0x03, 0x02, 0x02, 0x03, 0x07, 0x09,
      0x01, 0x05, 0x72, 0x6f, 0x74, 0x31, 0x33, 0x00, 0x03, 0x0a, 0x74, 0x02,
      0x39, 0x01, 0x01, 0x7f, 0x20, 0x00, 0x41, 0xc1, 0x00, 0x49, 0x04, 0x40,
      0x20, 0x00, 0x0f, 0x0b, 0x20, 0x00, 0x41, 0xdf, 0x01, 0x71, 0x21, 0x01,
      0x20, 0x01, 0x41, 0xcd, 0x00, 0x4d, 0x04, 0x40, 0x20, 0x00, 0x41, 0x0d,
      0x6a, 0x0f, 0x0b, 0x20, 0x01, 0x41, 0xda, 0x00, 0x4d, 0x04, 0x40, 0x20,
      0x00, 0x41, 0x0d, 0x6b, 0x0f, 0x0b, 0x20, 0x00, 0x0f, 0x0b, 0x38, 0x01,
      0x02, 0x7f, 0x41, 0x00, 0x41, 0x80, 0x08, 0x10, 0x00, 0x21, 0x00, 0x02,
      0x40, 0x03, 0x40, 0x20, 0x01, 0x20, 0x00, 0x4f, 0x04, 0x40, 0x0c, 0x02,
      0x0b, 0x20, 0x01, 0x20, 0x01, 0x2d, 0x00, 0x00, 0x10, 0x02, 0x3a, 0x00,
      0x00, 0x20, 0x01, 0x41, 0x01, 0x6a, 0x21, 0x01, 0x0c, 0x00, 0x0b, 0x0b,
      0x41, 0x00, 0x20, 0x00, 0x10, 0x01, 0x0b,
  };

  ASSERT_EQ(Result::Ok, LoadModule(data));

  string_data = "Hello, WebAssembly!";

//调用函数rot13
  ASSERT_EQ(interp::Result::Ok,
            executor_->RunExportByName(module_, "rot13", {}).result);

  ASSERT_EQ("Uryyb, JroNffrzoyl!", string_data);

  ASSERT_EQ(interp::Result::Ok,
            executor_->RunExportByName(module_, "rot13", {}).result);

  ASSERT_EQ("Hello, WebAssembly!", string_data);
}
```

这段代码基本说明了WABT虚拟机运行wasm模块的过程,代码比较清晰.我加了注释.之后结合EOS的源码相关部分,对每个细节进行介绍.

## EOS的API注册.

EOS在 "wasm_interface.cpp"里.使用REGISTER_INTRINSICS和REGISTER_INJECTED_INTRINSICS宏,向3个wasm解释器注册了大量的API.
这两个宏基于BOOST_PP_SEQ_FOR_EACH系列宏实现.

对于REGISTER_INTRINSIC,展开后是如下代码(仅截取WABT部分)

```cpp
REGISTER_INTRINSICS(producer_api,
   (get_active_producers,      int(int, int) )
);

...
REGISTER_INTRINSICS 展开后
...

static eosio::chain::webassembly::wabt_runtime::intrinsic_registrator
    __wabt_intrinsic_fn164(
        "env",
        "get_active_producers",
        eosio::chain::webassembly::wabt_runtime::wabt_function_type_provider<
            int(int, int)>::type(),
        eosio::chain::webassembly::wabt_runtime::
            intrinsic_function_invoker_wrapper<
                decltype(&producer_api::get_active_producers)>::type::
                fn<&producer_api::get_active_producers>());
                
                
                
REGISTER_INJECTED_INTRINSICS(call_depth_api,
   (call_depth_assert,  void()               )
);

...
REGISTER_INJECTED_INTRINSICS 展开后
...

static eosio::chain::webassembly::wabt_runtime::intrinsic_registrator
    __wabt_intrinsic_fn2(
        "eosio_injection",
        "call_depth_assert",
        eosio::chain::webassembly::wabt_runtime::wabt_function_type_provider<
            void()>::type(),
        eosio::chain::webassembly::wabt_runtime::
            intrinsic_function_invoker_wrapper<
                decltype(&call_depth_api::call_depth_assert)>::type::
                fn<&call_depth_api::call_depth_assert>());
```
这2个宏的区别在于注册时的模块名称不同

REGISTER_INTRINSICS注册到env里去
REGISTER_INJECTED_INTRINSICS注册到eosio_injection里去

这2个模块的区别在于env是提供给智能合约的API.eosio_injection则是eos本身用于注入操作码调用的的API.比如call_depth_assert是为了防止无限递归的函数

对于WABT来说,这段代码构造了一个wabt_runtime::intrinsic_registrator对象,构造函数传入了相关的信息
    
    struct intrinsic_registrator {
       using intrinsic_fn = TypedValue(*)(wabt_apply_instance_vars&, const TypedValues&);
    
       struct intrinsic_func_info {
          FuncSignature sig;
          intrinsic_fn func;
       }; 
    
       static auto& get_map(){
          static map<string, map<string, intrinsic_func_info>> _map;
          return _map;
       };
    
       intrinsic_registrator(const char* mod, const char* name, const FuncSignature& sig, intrinsic_fn fn) {
          get_map()[string(mod)][string(name)] = intrinsic_func_info{sig, fn};
       }
    };
EOS中,这段代码只是把需要注册的API函数按照模块名收集到一个map<string, map<string, intrinsic_func_info>>当中去,
EOS在每一个智能合约初始化的时候才会注册API.

在wabt_runtime::instantiate_module这个函数里,通过以下代码注册

```cpp
       ...
       std::unique_ptr<interp::Environment> env = std::make_unique<interp::Environment>();
       for(auto it = intrinsic_registrator::get_map().begin() ; it != intrinsic_registrator::get_map().end(); ++it) {
          interp::HostModule* host_module = env->AppendHostModule(it->first);
          for(auto itf = it->second.begin(); itf != it->second.end(); ++itf) {
             host_module->AppendFuncExport(itf->first, itf->second.sig, [fn=itf->second.func](const auto* f, const auto* fs, const auto& args, auto& res) {
                TypedValue ret = fn(*static_wabt_vars, args);
                if(ret.type != Type::Void)
                   res[0] = ret;
                return interp::Result::Ok;
             });
          }
       }
       ...
```
EOS在执行智能合约时,如果发现这个合约没有被缓存,那么则会调用instantiate_module函数来初始化合约的模块.intrinsic_registrator::get_map()中的每一个模块.注册该模块所有的API.



## 初始化虚拟机.

```cpp
   void wasm_interface::apply( const digest_type& code_id, const shared_string& code, apply_context& context ) {
      my->get_instantiated_module(code_id, code, context.trx_context)->apply(context);
   }
```
    
当EOS调用wasm_interface::apply的时候,会从缓存当中查找对应的智能合约的虚拟机模块.如果没有查找到,则会初始化虚拟机

```cpp
      ......
      std::unique_ptr<wasm_instantiated_module_interface>& get_instantiated_module( const digest_type& code_id,
                                                                                    const shared_string& code,
                                                                                    transaction_context& trx_context )
      {
         //查找智能合约的虚拟机模块
         auto it = instantiation_cache.find(code_id);
         if(it == instantiation_cache.end()) {
            //这里会停止掉CPU时间计费函数
            auto timer_pause = fc::make_scoped_exit([&](){
               //恢复CPU时间计费
               trx_context.resume_billing_timer();
            });
            trx_context.pause_billing_timer();
            
            //准备注入相关的模块
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
            
            //opcode注入,防止死循环等恶意代码

            wasm_injections::wasm_binary_injection injector(module);
            injector.inject();

            //准备注入的结果
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
            //调用虚拟机构造模块
            it = instantiation_cache.emplace(code_id, runtime_interface->instantiate_module((const char*)bytes.data(), bytes.size(), parse_initial_memory(module))).first;
         }
         return it->second;
      }
   ......
   ...
   ...
    std::unique_ptr<wasm_instantiated_module_interface> wabt_runtime::instantiate_module(const char* code_bytes, size_t code_size, std::vector<uint8_t> initial_memory) {
       std::unique_ptr<interp::Environment> env = std::make_unique<interp::Environment>();
       for(auto it = intrinsic_registrator::get_map().begin() ; it != intrinsic_registrator::get_map().end(); ++it) {
          interp::HostModule* host_module = env->AppendHostModule(it->first);
          for(auto itf = it->second.begin(); itf != it->second.end(); ++itf) {
             host_module->AppendFuncExport(itf->first, itf->second.sig, [fn=itf->second.func](const auto* f, const auto* fs, const auto& args, auto& res) {
                TypedValue ret = fn(*static_wabt_vars, args);
                if(ret.type != Type::Void)
                   res[0] = ret;
                return interp::Result::Ok;
             });
          }
       }
    
       interp::DefinedModule* instantiated_module = nullptr;
       wabt::Errors errors;
       //读入智能合约的代码(注入修正过的代码)
       wabt::Result res = ReadBinaryInterp(env.get(), code_bytes, code_size, read_binary_options, &errors, &instantiated_module);
       EOS_ASSERT( Succeeded(res), wasm_execution_error, "Error building wabt interp: ${e}", ("e", wabt::FormatErrorsToString(errors, Location::Type::Binary)) );
       
       return std::make_unique<wabt_instantiated_module>(std::move(env), initial_memory, instantiated_module);
    }
   ...
```

## 发布合约

eos源生注册了一些API用于注册帐号,发布合约等功能
```cpp
    ...
    SET_APP_HANDLER( eosio, eosio, newaccount );
    SET_APP_HANDLER( eosio, eosio, setcode );
    SET_APP_HANDLER( eosio, eosio, setabi );
    SET_APP_HANDLER( eosio, eosio, updateauth );
    SET_APP_HANDLER( eosio, eosio, deleteauth );
    SET_APP_HANDLER( eosio, eosio, linkauth );
    SET_APP_HANDLER( eosio, eosio, unlinkauth );
    ...
    
    以发布合约为例,宏展开后的函数是
    set_apply_handler("eosio", "eosio", "setcode", &apply_eosio_setcode);
```
eos使用apply_eosio_setcode用来发布合约
```cpp
    void apply_eosio_setcode(apply_context& context) {
       const auto& cfg = context.control.get_global_properties().configuration;
    
       auto& db = context.db;
       auto  act = context.act.data_as<setcode>();
       context.require_authorization(act.account);
    
       EOS_ASSERT( act.vmtype == 0, invalid_contract_vm_type, "code should be 0" );
       EOS_ASSERT( act.vmversion == 0, invalid_contract_vm_version, "version should be 0" );
    
       fc::sha256 code_id; /// default ID == 0
    
       if( act.code.size() > 0 ) {
         code_id = fc::sha256::hash( act.code.data(), (uint32_t)act.code.size() );
         wasm_interface::validate(context.control, act.code);
       }
    
       const auto& account = db.get<account_object,by_name>(act.account);
    
       int64_t code_size = (int64_t)act.code.size();
       int64_t old_size  = (int64_t)account.code.size() * config::setcode_ram_bytes_multiplier;
       int64_t new_size  = code_size * config::setcode_ram_bytes_multiplier;
    
       EOS_ASSERT( account.code_version != code_id, set_exact_code, "contract is already running this version of code" );
    
       db.modify( account, [&]( auto& a ) {
          /** TODO: consider whether a microsecond level local timestamp is sufficient to detect code version changes*/
          // TODO: update setcode message to include the hash, then validate it in validate
          a.last_code_update = context.control.pending_block_time();
          a.code_version = code_id;
          a.code.resize( code_size );
          if( code_size > 0 )
             memcpy( a.code.data(), act.code.data(), code_size );
    
       });
    
       const auto& account_sequence = db.get<account_sequence_object, by_name>(act.account);
       db.modify( account_sequence, [&]( auto& aso ) {
          aso.code_sequence += 1;
       });
    
       if (new_size != old_size) {
          context.add_ram_usage( act.account, new_size - old_size );
       }
    }
```

在apply_eosio_setcode中,EOS验证了合约的发布账号,计算了合约的hash256作为合约的ID.检查合约的版本(ID),
并且把合约的数据入库.最后根据新合约的大小.更新账号占用的RAM.

## opcode注入

EOS初始化的时候会对智能合约代码进行注入.修改虚拟机的opcode.加入CPU时间检查等等安全措施.

注入的代码是在wasm_binary_injection类里实现的.

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
在inject中,首先注册了checktime这个ABI,然后对合约的每个函数的opcode进行注入

op->visit( { _module, &pre_code, &fd, pre_decoder.index() } );

在这个调用里,对174个相关的操作码进行了注入.相关的注入代码如下:

```cpp
    template <typename ... Mutators>
    struct instr_base : instr {
       bool is_post() override { return propagate_post_injection<Mutators...>::value; }
       bool is_kill() override { return propagate_should_kill<Mutators...>::value; }
       virtual void visit( visitor_arg&& arg ) override {
          for ( auto m : { Mutators::accept... } ) {
             m(this, arg);
          }
       } 
    };
    
    // construct the instructions
    
    BOOST_PP_SEQ_FOR_EACH( CONSTRUCT_OP_HAS_DATA, voidtype,         BOOST_PP_SEQ_SUBSEQ( WASM_OP_SEQ, 0, 133 ) )
    BOOST_PP_SEQ_FOR_EACH( CONSTRUCT_OP_HAS_DATA, blocktype,        BOOST_PP_SEQ_SUBSEQ( WASM_OP_SEQ, 133, 3 ) )
    BOOST_PP_SEQ_FOR_EACH( CONSTRUCT_OP_HAS_DATA, uint32_t,         BOOST_PP_SEQ_SUBSEQ( WASM_OP_SEQ, 136, 11 ) )
    BOOST_PP_SEQ_FOR_EACH( CONSTRUCT_OP_HAS_DATA, memarg,           BOOST_PP_SEQ_SUBSEQ( WASM_OP_SEQ, 147, 23 ) )
    BOOST_PP_SEQ_FOR_EACH( CONSTRUCT_OP_HAS_DATA, uint64_t,         BOOST_PP_SEQ_SUBSEQ( WASM_OP_SEQ, 170, 2 ) )
    BOOST_PP_SEQ_FOR_EACH( CONSTRUCT_OP_HAS_DATA, branchtabletype,  BOOST_PP_SEQ_SUBSEQ( WASM_OP_SEQ, 172, 1 ) )
```

EOS里使用了大量的BOOST_PP_SEQ_FOR_EACH宏,具体可以参见BOOST的相关文档.
以loop为例,展开后是这样的

```cpp
            template <typename... Mutators>
            struct loop final : instr_base<Mutators...> {
                uint16_t code = loop_code;
                blocktype field;
                uint16_t get_code() override { return loop_code; }
                int skip_ahead() override {
                    return field_specific_params<blocktype>::skip_ahead;
                }
                void unpack(char* opcode) override {
                    field_specific_params<blocktype>::unpack(opcode, field);
                }
                void pack(instruction_stream* stream) override {
                    stream->set(2, (const char*)&code);
                    field_specific_params<blocktype>::pack(stream, field);
                }
                std::string to_string() override {
                    return std::string("loop") +
                           field_specific_params<blocktype>::to_string(field);
                }
            };
```
其中模板参数Mutators,是一系列实现了accept接口的类,控制对操作码进行哪些注入操作.以最复杂的call_depth_check(检查函数调用的深度,防止无限递归)为例

```cpp
    static void accept( wasm_ops::instr* inst, wasm_ops::visitor_arg& arg ) {
         if ( global_idx == -1 ) {
            arg.module->globals.defs.push_back({{ValueType::i32, true}, {(I32) eosio::chain::wasm_constraints::maximum_call_depth}});
         }

         global_idx = arg.module->globals.size()-1;

         int32_t assert_idx;
         injector_utils::add_import<ResultType::none>(*(arg.module), "call_depth_assert", assert_idx);

         wasm_ops::op_types<>::call_t call_assert;
         wasm_ops::op_types<>::get_global_t get_global_inst; 
         wasm_ops::op_types<>::set_global_t set_global_inst;

         wasm_ops::op_types<>::i32_eqz_t eqz_inst; 
         wasm_ops::op_types<>::i32_const_t const_inst; 
         wasm_ops::op_types<>::i32_add_t add_inst;
         wasm_ops::op_types<>::end_t end_inst;
         wasm_ops::op_types<>::if__t if_inst; 
         wasm_ops::op_types<>::else__t else_inst; 

         call_assert.field = assert_idx;
         get_global_inst.field = global_idx;
         set_global_inst.field = global_idx;
         const_inst.field = -1;

#define INSERT_INJECTED(X)       \
         X.pack(arg.new_code);   \

         INSERT_INJECTED(get_global_inst); //inject
         INSERT_INJECTED(eqz_inst);//inject
         INSERT_INJECTED(if_inst);//inject
         INSERT_INJECTED(call_assert);//inject
         INSERT_INJECTED(else_inst);//inject
         INSERT_INJECTED(const_inst);//inject
         INSERT_INJECTED(get_global_inst);//inject
         INSERT_INJECTED(add_inst);//inject
         INSERT_INJECTED(set_global_inst);//inject
         INSERT_INJECTED(end_inst);//inject

         /* print the correct call type */
         if ( inst->get_code() == wasm_ops::call_code ) {
            wasm_ops::op_types<>::call_t* call_inst = reinterpret_cast<wasm_ops::op_types<>::call_t*>(inst);
            call_inst->pack(arg.new_code);//inject
         }
         else {
            wasm_ops::op_types<>::call_indirect_t* call_inst = reinterpret_cast<wasm_ops::op_types<>::call_indirect_t*>(inst);
            call_inst->pack(arg.new_code);//inject
         }

         const_inst.field = 1;
         INSERT_INJECTED(get_global_inst); //inject
         INSERT_INJECTED(const_inst);//inject
         INSERT_INJECTED(add_inst);//inject
         INSERT_INJECTED(set_global_inst);//inject

#undef INSERT_INJECTED
      }
   }; 
```

其中我注释了//inject的都是具体注入的代码.

未完待续
