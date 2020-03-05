# eosex
分布式智能合约并发执行系统。参考EOS源码开发

VM: 虚拟机实现

block_chain 链存储实现

eosio_database 合约数据库实现

network P2P网络实现

# EOS 虚拟机源码分析

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

INSERT_INJECTED宏展开后"get_global_inst.pack(arg.new_code)"实现注入.call_depth_check注入了一系列代码,检查合约的函数是否出现了无限递归调用.

EOS对以下函数进行了注入

```cpp
_cached_ops[error_code] = cached_error.get();
_cached_ops[end_code] = cached_end.get();
_cached_ops[unreachable_code] = cached_unreachable.get();
_cached_ops[nop_code] = cached_nop.get();
_cached_ops[else__code] = cached_else_.get();
_cached_ops[return__code] = cached_return_.get();
_cached_ops[drop_code] = cached_drop.get();
_cached_ops[select_code] = cached_select.get();
_cached_ops[i32_eqz_code] = cached_i32_eqz.get();
_cached_ops[i32_eq_code] = cached_i32_eq.get();
_cached_ops[i32_ne_code] = cached_i32_ne.get();
_cached_ops[i32_lt_s_code] = cached_i32_lt_s.get();
_cached_ops[i32_lt_u_code] = cached_i32_lt_u.get();
_cached_ops[i32_gt_s_code] = cached_i32_gt_s.get();
_cached_ops[i32_gt_u_code] = cached_i32_gt_u.get();
_cached_ops[i32_le_s_code] = cached_i32_le_s.get();
_cached_ops[i32_le_u_code] = cached_i32_le_u.get();
_cached_ops[i32_ge_s_code] = cached_i32_ge_s.get();
_cached_ops[i32_ge_u_code] = cached_i32_ge_u.get();
_cached_ops[i64_eqz_code] = cached_i64_eqz.get();
_cached_ops[i64_eq_code] = cached_i64_eq.get();
_cached_ops[i64_ne_code] = cached_i64_ne.get();
_cached_ops[i64_lt_s_code] = cached_i64_lt_s.get();
_cached_ops[i64_lt_u_code] = cached_i64_lt_u.get();
_cached_ops[i64_gt_s_code] = cached_i64_gt_s.get();
_cached_ops[i64_gt_u_code] = cached_i64_gt_u.get();
_cached_ops[i64_le_s_code] = cached_i64_le_s.get();
_cached_ops[i64_le_u_code] = cached_i64_le_u.get();
_cached_ops[i64_ge_s_code] = cached_i64_ge_s.get();
_cached_ops[i64_ge_u_code] = cached_i64_ge_u.get();
_cached_ops[f32_eq_code] = cached_f32_eq.get();
_cached_ops[f32_ne_code] = cached_f32_ne.get();
_cached_ops[f32_lt_code] = cached_f32_lt.get();
_cached_ops[f32_gt_code] = cached_f32_gt.get();
_cached_ops[f32_le_code] = cached_f32_le.get();
_cached_ops[f32_ge_code] = cached_f32_ge.get();
_cached_ops[f64_eq_code] = cached_f64_eq.get();
_cached_ops[f64_ne_code] = cached_f64_ne.get();
_cached_ops[f64_lt_code] = cached_f64_lt.get();
_cached_ops[f64_gt_code] = cached_f64_gt.get();
_cached_ops[f64_le_code] = cached_f64_le.get();
_cached_ops[f64_ge_code] = cached_f64_ge.get();
_cached_ops[i32_clz_code] = cached_i32_clz.get();
_cached_ops[i32_ctz_code] = cached_i32_ctz.get();
_cached_ops[i32_popcnt_code] = cached_i32_popcnt.get();
_cached_ops[i32_add_code] = cached_i32_add.get();
_cached_ops[i32_sub_code] = cached_i32_sub.get();
_cached_ops[i32_mul_code] = cached_i32_mul.get();
_cached_ops[i32_div_s_code] = cached_i32_div_s.get();
_cached_ops[i32_div_u_code] = cached_i32_div_u.get();
_cached_ops[i32_rem_s_code] = cached_i32_rem_s.get();
_cached_ops[i32_rem_u_code] = cached_i32_rem_u.get();
_cached_ops[i32_and_code] = cached_i32_and.get();
_cached_ops[i32_or_code] = cached_i32_or.get();
_cached_ops[i32_xor_code] = cached_i32_xor.get();
_cached_ops[i32_shl_code] = cached_i32_shl.get();
_cached_ops[i32_shr_s_code] = cached_i32_shr_s.get();
_cached_ops[i32_shr_u_code] = cached_i32_shr_u.get();
_cached_ops[i32_rotl_code] = cached_i32_rotl.get();
_cached_ops[i32_rotr_code] = cached_i32_rotr.get();
_cached_ops[i64_clz_code] = cached_i64_clz.get();
_cached_ops[i64_ctz_code] = cached_i64_ctz.get();
_cached_ops[i64_popcnt_code] = cached_i64_popcnt.get();
_cached_ops[i64_add_code] = cached_i64_add.get();
_cached_ops[i64_sub_code] = cached_i64_sub.get();
_cached_ops[i64_mul_code] = cached_i64_mul.get();
_cached_ops[i64_div_s_code] = cached_i64_div_s.get();
_cached_ops[i64_div_u_code] = cached_i64_div_u.get();
_cached_ops[i64_rem_s_code] = cached_i64_rem_s.get();
_cached_ops[i64_rem_u_code] = cached_i64_rem_u.get();
_cached_ops[i64_and_code] = cached_i64_and.get();
_cached_ops[i64_or_code] = cached_i64_or.get();
_cached_ops[i64_xor_code] = cached_i64_xor.get();
_cached_ops[i64_shl_code] = cached_i64_shl.get();
_cached_ops[i64_shr_s_code] = cached_i64_shr_s.get();
_cached_ops[i64_shr_u_code] = cached_i64_shr_u.get();
_cached_ops[i64_rotl_code] = cached_i64_rotl.get();
_cached_ops[i64_rotr_code] = cached_i64_rotr.get();
_cached_ops[f32_abs_code] = cached_f32_abs.get();
_cached_ops[f32_neg_code] = cached_f32_neg.get();
_cached_ops[f32_ceil_code] = cached_f32_ceil.get();
_cached_ops[f32_floor_code] = cached_f32_floor.get();
_cached_ops[f32_trunc_code] = cached_f32_trunc.get();
_cached_ops[f32_nearest_code] = cached_f32_nearest.get();
_cached_ops[f32_sqrt_code] = cached_f32_sqrt.get();
_cached_ops[f32_add_code] = cached_f32_add.get();
_cached_ops[f32_sub_code] = cached_f32_sub.get();
_cached_ops[f32_mul_code] = cached_f32_mul.get();
_cached_ops[f32_div_code] = cached_f32_div.get();
_cached_ops[f32_min_code] = cached_f32_min.get();
_cached_ops[f32_max_code] = cached_f32_max.get();
_cached_ops[f32_copysign_code] = cached_f32_copysign.get();
_cached_ops[f64_abs_code] = cached_f64_abs.get();
_cached_ops[f64_neg_code] = cached_f64_neg.get();
_cached_ops[f64_ceil_code] = cached_f64_ceil.get();
_cached_ops[f64_floor_code] = cached_f64_floor.get();
_cached_ops[f64_trunc_code] = cached_f64_trunc.get();
_cached_ops[f64_nearest_code] = cached_f64_nearest.get();
_cached_ops[f64_sqrt_code] = cached_f64_sqrt.get();
_cached_ops[f64_add_code] = cached_f64_add.get();
_cached_ops[f64_sub_code] = cached_f64_sub.get();
_cached_ops[f64_mul_code] = cached_f64_mul.get();
_cached_ops[f64_div_code] = cached_f64_div.get();
_cached_ops[f64_min_code] = cached_f64_min.get();
_cached_ops[f64_max_code] = cached_f64_max.get();
_cached_ops[f64_copysign_code] = cached_f64_copysign.get();
_cached_ops[i32_wrap_i64_code] = cached_i32_wrap_i64.get();
_cached_ops[i32_trunc_s_f32_code] = cached_i32_trunc_s_f32.get();
_cached_ops[i32_trunc_u_f32_code] = cached_i32_trunc_u_f32.get();
_cached_ops[i32_trunc_s_f64_code] = cached_i32_trunc_s_f64.get();
_cached_ops[i32_trunc_u_f64_code] = cached_i32_trunc_u_f64.get();
_cached_ops[i64_extend_s_i32_code] = cached_i64_extend_s_i32.get();
_cached_ops[i64_extend_u_i32_code] = cached_i64_extend_u_i32.get();
_cached_ops[i64_trunc_s_f32_code] = cached_i64_trunc_s_f32.get();
_cached_ops[i64_trunc_u_f32_code] = cached_i64_trunc_u_f32.get();
_cached_ops[i64_trunc_s_f64_code] = cached_i64_trunc_s_f64.get();
_cached_ops[i64_trunc_u_f64_code] = cached_i64_trunc_u_f64.get();
_cached_ops[f32_convert_s_i32_code] = cached_f32_convert_s_i32.get();
_cached_ops[f32_convert_u_i32_code] = cached_f32_convert_u_i32.get();
_cached_ops[f32_convert_s_i64_code] = cached_f32_convert_s_i64.get();
_cached_ops[f32_convert_u_i64_code] = cached_f32_convert_u_i64.get();
_cached_ops[f32_demote_f64_code] = cached_f32_demote_f64.get();
_cached_ops[f64_convert_s_i32_code] = cached_f64_convert_s_i32.get();
_cached_ops[f64_convert_u_i32_code] = cached_f64_convert_u_i32.get();
_cached_ops[f64_convert_s_i64_code] = cached_f64_convert_s_i64.get();
_cached_ops[f64_convert_u_i64_code] = cached_f64_convert_u_i64.get();
_cached_ops[f64_promote_f32_code] = cached_f64_promote_f32.get();
_cached_ops[i32_reinterpret_f32_code] = cached_i32_reinterpret_f32.get();
_cached_ops[i64_reinterpret_f64_code] = cached_i64_reinterpret_f64.get();
_cached_ops[f32_reinterpret_i32_code] = cached_f32_reinterpret_i32.get();
_cached_ops[f64_reinterpret_i64_code] = cached_f64_reinterpret_i64.get();
_cached_ops[grow_memory_code] = cached_grow_memory.get();
_cached_ops[current_memory_code] = cached_current_memory.get();
_cached_ops[block_code] = cached_block.get();
_cached_ops[loop_code] = cached_loop.get();
_cached_ops[if__code] = cached_if_.get();
_cached_ops[br_code] = cached_br.get();
_cached_ops[br_if_code] = cached_br_if.get();
_cached_ops[call_code] = cached_call.get();
_cached_ops[call_indirect_code] = cached_call_indirect.get();
_cached_ops[get_local_code] = cached_get_local.get();
_cached_ops[set_local_code] = cached_set_local.get();
_cached_ops[tee_local_code] = cached_tee_local.get();
_cached_ops[get_global_code] = cached_get_global.get();
_cached_ops[set_global_code] = cached_set_global.get();
_cached_ops[i32_const_code] = cached_i32_const.get();
_cached_ops[f32_const_code] = cached_f32_const.get();
_cached_ops[i32_load_code] = cached_i32_load.get();
_cached_ops[i64_load_code] = cached_i64_load.get();
_cached_ops[f32_load_code] = cached_f32_load.get();
_cached_ops[f64_load_code] = cached_f64_load.get();
_cached_ops[i32_load8_s_code] = cached_i32_load8_s.get();
_cached_ops[i32_load8_u_code] = cached_i32_load8_u.get();
_cached_ops[i32_load16_s_code] = cached_i32_load16_s.get();
_cached_ops[i32_load16_u_code] = cached_i32_load16_u.get();
_cached_ops[i64_load8_s_code] = cached_i64_load8_s.get();
_cached_ops[i64_load8_u_code] = cached_i64_load8_u.get();
_cached_ops[i64_load16_s_code] = cached_i64_load16_s.get();
_cached_ops[i64_load16_u_code] = cached_i64_load16_u.get();
_cached_ops[i64_load32_s_code] = cached_i64_load32_s.get();
_cached_ops[i64_load32_u_code] = cached_i64_load32_u.get();
_cached_ops[i32_store_code] = cached_i32_store.get();
_cached_ops[i64_store_code] = cached_i64_store.get();
_cached_ops[f32_store_code] = cached_f32_store.get();
_cached_ops[f64_store_code] = cached_f64_store.get();
_cached_ops[i32_store8_code] = cached_i32_store8.get();
_cached_ops[i32_store16_code] = cached_i32_store16.get();
_cached_ops[i64_store8_code] = cached_i64_store8.get();
_cached_ops[i64_store16_code] = cached_i64_store16.get();
_cached_ops[i64_store32_code] = cached_i64_store32.get();
_cached_ops[i64_const_code] = cached_i64_const.get();
_cached_ops[f64_const_code] = cached_f64_const.get();
_cached_ops[br_table_code] = cached_br_table.get();
```

未完待续

# EOS网络通信分析

eos网络通信主要分为两部分，一是p2p通信，即各个nodeos节点之间的通信，二是客户端与服务器之间REST API的通信，即cleos与nodeos的通信，EOS采用插件机制，前者主要在net_plugin中实现，后者主要在http_plugin中实现。

## p2p网络

### plugin初始化

nodeos启动的时候，会注册并初始化相应插件，需要注册的插件可以通过config.ini配置文件配置，也可以通过启动参数配置，nodeos main函数中通过initialize<...>()，调用各个插件的plugin_initialize()方法，实现插件的初始化，插件初始化：
```
if(!app().initialize<chain_plugin, http_plugin, net_plugin, producer_plugin>(argc, argv))
         return INITIALIZE_FAIL;
```
其中，网络部分主要看net_plugin和http_plugin，先分析net_plugin中实现的功能。
在net_plugin的plugin_initialize()中，首先会读取配置的各个参数，比如sync-fetch-span(一次同步区块的数量)，p2p-listen-endpoint(p2p本地监听端点)，p2p-peer-address(要连接的其他nodeos地址)等。然后通过chain_plugin获取chain_id，chain_id在p2p通信的时候会用到，判断是不是同链之间的通信。
```
	my->chain_plug = app().find_plugin<chain_plugin>();
    my->chain_id = app().get_plugin<chain_plugin>().get_chain_id();
```

### plugin启动

nodeos mian函数中，初始化插件后，会调用app().startup()启动各个已注册的插件，分别调用每个插件的plugin_startup()完成插件启动
```
void application::startup() {
   try {
      for (auto plugin : initialized_plugins)
         plugin->startup();
   } catch(...) {
      shutdown();
      throw;
   }
}
class plugin : public abstract_plugin {
	...
	virtual void startup() override {
        if(_state == initialized) {
           _state = started;
           static_cast<Impl*>(this)->plugin_requires([&](auto& plug){ plug.startup(); });
           static_cast<Impl*>(this)->plugin_startup();
           app().plugin_started(*this);
        }
        assert(_state == started); // if initial state was not initialized, final state cannot be started
     }
}
```
net_plugin在plugin_startup()中会做几项工作，先整体说一下，后面再具体分析：
* 1 监听本地p2p端口，接受连接并等待读取消息
```
void net_plugin::plugin_startup() {
	if( my->acceptor ) {
        my->acceptor->open(my->listen_endpoint.protocol());
        my->acceptor->set_option(tcp::acceptor::reuse_address(true));
        my->acceptor->bind(my->listen_endpoint);
        my->acceptor->listen();
        ilog("starting listener, max clients is ${mc}",("mc",my->max_client_count));
        my->start_listen_loop();
    }
    ...
}
void net_plugin_impl::start_listen_loop( ) {
	acceptor->async_accept( *socket, [socket,this]( boost::system::error_code ec ) {
    	...
    	connection_ptr c = std::make_shared<connection>( socket );
        connections.insert( c );
        start_session( c );
    });
}
```
* 2 绑定区块/交易广播信号槽
```
{
    chain::controller&cc = my->chain_plug->chain();
    cc.accepted_block_header.connect( boost::bind(&net_plugin_impl::accepted_block_header, my.get(), _1));
    cc.accepted_block.connect(  boost::bind(&net_plugin_impl::accepted_block, my.get(), _1));
    cc.irreversible_block.connect( boost::bind(&net_plugin_impl::irreversible_block, my.get(), _1));
    cc.accepted_transaction.connect( boost::bind(&net_plugin_impl::accepted_transaction, my.get(), _1));
    cc.applied_transaction.connect( boost::bind(&net_plugin_impl::applied_transaction, my.get(), _1));
    cc.accepted_confirmation.connect( boost::bind(&net_plugin_impl::accepted_confirmation, my.get(), _1));
}
my->incoming_transaction_ack_subscription = app().get_channel<channels::transaction_ack>().subscribe(boost::bind(&net_plugin_impl::transaction_ack, my.get(), _1));
```
* 3 定时监控nodeos connections连接和定时清理本地过期交易
```
my->start_monitors();
void net_plugin_impl::start_monitors() {
    connector_check.reset(new boost::asio::steady_timer( app().get_io_service()));
    transaction_check.reset(new boost::asio::steady_timer( app().get_io_service()));
    start_conn_timer();
    start_txn_timer();
}
```
* 4 主动连接本地配置的其他p2p-addr。
```
for( auto seed_node : my->supplied_peers ) {
	connect( seed_node );
}
```
---


### 节点间通信

下面分析下两个nodeos节点间通信流程(A的p2p-addr为192.168.1.101:9876，B的p2p-addr为192.168.1.102:9876)

#### p2p连接

在前面已经介绍了net_plugin启动时的几项工作，其中第4条主动连接其他p2p-addr节点，从这里开始分析。
假设节点B已经启动，已监听本地p2p端口；节点A在config.ini中配置了节点B的p2p-addr，即A的config.ini部分配置如下:
```
p2p-listen-endpoint = 0.0.0.0:9876
p2p-peer-address = 192.168.1.102:9876
```
A在启动时会调用net_plugin::connect()方法主动连接B，首先判断是否已经连接，如果没有连接会把host("192.168.1.102:9876")作为参数赋值给connection的成员变量peer_addr 构造connection_ptr，然后调用net_plugin_impl::connect()方法，在这里会去解析peer-addr，然后调用boost的asio库方法连接B，如果连接成功，再调用net_plugin_impl::start_session()--> net_plugin_impl::start_read_message()，开始异步读取消息。
```
string net_plugin::connect( const string& host ) {
    if( my->find_connection( host ) )
     return "already connected";

    connection_ptr c = std::make_shared<connection>(host);
    my->connections.insert( c );
    my->connect( c );
    return "added connection";
}
```
```
void net_plugin_impl::connect( connection_ptr c, tcp::resolver::iterator endpoint_itr ) {
	c->socket->async_connect( current_endpoint, [weak_conn, endpoint_itr, this] ( const boost::system::error_code& err ) {
        if( !err && c->socket->is_open() ) {
           if (start_session( c )) {
              c->send_handshake ();
           }
        }
        ...
    });
}
```
```
bool net_plugin_impl::start_session( connection_ptr con ) {
	...
    start_read_message( con );
}
```
```
void net_plugin_impl::start_read_message( connection_ptr conn ) {
	...
    boost::asio::async_read(...);
}
```
我们看到，如果A连接成功B开始一个session后，会接着调用c->send_handshake()给B发送一个handshake信息过去，这里就开始了A与B的p2p通信。

#### p2p消息发送与接收

	两个nodeos之间，是通过发送几种不同类型的message来实现信息交换，同步区块/交易，以及断连原因等，这些消息类型有handshake_message，go_away_message，time_message，notice_message，request_message，sync_request_message，signed_block，packed_transaction等。
    
##### handshake_message
	下面从发送接收handshake_message握手信息来入手分析p2p各种消息的通信流程。
	当A成功连接B后，会调用send_handshake()，在该方法中会生成一个handshake_message，然后会将该message放入connection的消息队列中，再通过boost::asio::async_write()发送给B。handshake_message中带有A的本地信息，包括chain_id(识别链)，node_id(识别peers，防止自己连自己)，公钥/签名，本地区块头号，区块头ID，不可逆区块号，不可逆区块ID等信息
```
struct handshake_message {
    uint16_t                   network_version = 0;
    chain_id_type              chain_id;
    fc::sha256                 node_id;
    chain::public_key_type     key;
    tstamp                     time;
    fc::sha256                 token;
    chain::signature_type      sig;
    string                     p2p_address;
    uint32_t                   last_irreversible_block_num = 0;
    block_id_type              last_irreversible_block_id;
    uint32_t                   head_num = 0;
    block_id_type              head_id;
    string                     os;
    string                     agent;
    int16_t                    generation;
};
```
前面我们假设了B已经启动，当A成功连接到B，并给B发送handshake_message后，B通过boost::asio::async_read()，读取到A给B发送的消息，然后处理解析消息，通过msgHandler调用net_plugin_impl::handle_message(c, msg)处理不同类型的消息，在net_plugin_impl中重载了handle_message()方法，当A发给B的是handshake_message，会调用net_plugin_impl::handle_message( connection_ptr c, const handshake_message &msg) 方法。在该方法中会判断handshake_message的有效性，比较chain_id，node_id，network_version以及判断是否分叉等，如果是正确有效的handshake_message，则会调用sync_master->recv_handshake(c,msg)，比较两个节点的区块信息，判断是否需要同步区块等。
```
void sync_manager::recv_handshake (connection_ptr c, const handshake_message &msg) {
	//获取本地(B)和peer(A)两节点的不可逆号
	controller& cc = chain_plug->chain();
    uint32_t lib_num = cc.last_irreversible_block_num( );
    uint32_t peer_lib = msg.last_irreversible_block_num;
    //获取B区块头号和头ID
    uint32_t head = cc.head_block_num( );
    block_id_type head_id = cc.head_block_id();
    //下面是比较两个节点信息，然后决定B节点给A节点发送什么类型的message
    if (head_id == msg.head_id) {
    	//头块ID相同，发送一个known_trx.mode = catch_up的notice_message，带着本地缓存交易的数目
        notice_message note;...
        return;
    }
    //如果B的头号小于A的不可逆号，B给A发送一个同步区块的请求信息
    if (head < peer_lib) {
    	start_sync( c, peer_lib);
        return;
    }
    //如果本地不可逆号大于消息的头号，则B给A发一个notice_message消息，告诉A此时B的不可逆号，让A来向B请求同步
    if (lib_num > msg.head_num ) {
    	if (msg.generation > 1 || c->protocol_version > proto_base) {
            notice_message note;
            note.known_trx.pending = lib_num;
            note.known_trx.mode = last_irr_catch_up;
            note.known_blocks.mode = last_irr_catch_up;
            note.known_blocks.pending = head;
            c->enqueue( note );
         }
         c->syncing = true;
         return;
    }
    //如果本地头号小于等于消息的头号，则B给A发送一个request_message，mode为 catch_up消息，请求A不可逆号与头号之间的区块，否则发送一个notice_message catch_up消息，带着本地头ID
    if (head <= msg.head_num ) {
        verify_catchup (c, msg.head_num, msg.head_id);
        return;
    } else {
    	if (msg.generation > 1 ||  c->protocol_version > proto_base) {
            notice_message note;
            note.known_trx.mode = none;
            note.known_blocks.mode = catch_up;
            note.known_blocks.pending = head;
            note.known_blocks.ids.push_back(head_id);
            c->enqueue( note );
         }
         c->syncing = true;
         return;
    }
}
```
同样的，B给A发送消息，A接受消息，采用相同的流程，B通过把message放入消息队列然后发送给A，A收到信息后，判断message type，调用不同的handle_message()方法，处理消息，后面会对每个消息类型的处理作具体分析。

##### request_message

request_message处理，先贴源码，从源码分析
```
void net_plugin_impl::handle_message( connection_ptr c, const request_message &msg) {
      switch (msg.req_blocks.mode) {
      case catch_up :
         peer_ilog(c,  "received request_message:catch_up");
         c->blk_send_branch( );
         break;
      case normal :
         peer_ilog(c, "received request_message:normal");
         c->blk_send(msg.req_blocks.ids);
         break;
      default:;
      }

      switch (msg.req_trx.mode) {
      case catch_up :
         c->txn_send_pending(msg.req_trx.ids);
         break;
      case normal :
         c->txn_send(msg.req_trx.ids);
         break;
      case none :
         if(msg.req_blocks.mode == none)
            c->stop_send();
         break;
      default:;
      }
}
```
当nodeos(B)生成一个request_message请求的时候要带上请求的mode，当nodeos(A)收到request_message后，会根据mode来判断对方的意图，来作相应的处理。比如A收到msg.req_blocks.mode为catch_up时，会调用blk_send_branch()将本地分支的不可逆区块与头区块之间的signed_block发送给B；而当msg.req_blocks.mode为normal时，A会调用c->blk_send(msg.req_blocks.ids)，遍历msg.req_blocks.ids中的block id，根据id查询本地是否存在，存在的话就将该signed_block发送给B。其他msg.req_trx.mode也有相应的处理，略过。

##### notice_message

```
void net_plugin_impl::handle_message( connection_ptr c, const notice_message &msg) {
      request_message req;
      bool send_req = false;
      ...
      switch (msg.known_blocks.mode) {
      case none : {
         if (msg.known_trx.mode != normal) {
            return;
         }
         break;
      }
      case last_irr_catch_up:
      case catch_up: {
         sync_master->recv_notice(c,msg);
         break;
      }
      case normal : {
         dispatcher->recv_notice (c, msg, false);
         break;
      }
      default:
      }
      if( send_req) {
         c->enqueue(req);
      }
   }
```
同request_message一样，notice_message也有mode，也会根据不同的mode处理不同的请求。比如A收到的msg.known_blocks.mode为last_irr_catch_up，则A会在sync_master->recv_notice(c,msg)方法中调用sync_manager::start_sync()，向B请求同步区块；而当msg.known_blocks.mode为catch_up时，则A会根据B发送过来的msg判断分支情况，再生成一个相应mode的request_message发送给B。这地方会有点绕，需要几个message结合起来理解一下。

##### sync_request_message

前面分析中提到，根据不同的情况B会发送不同的message给A，其中一种情况就是B在交流中发现自己落伍了，想要跟上大部队的步伐，于是需要从A那里获取区块，此时B调用sync_manager::start_sync()-->sync_manager::request_next_chunk()-->connection::request_sync_blocks()，发送一个sync_request_message给A，并将B的同步状态设为lib_catchup。
sync_request_message比较简单，只有两个参数:start_block/end_block，分别表示要同步块的开始号和结束号，比如想要同步的区块范围是101-1000，而一次同步100个，则start_block/end_block分别设为101和200。
当A收到sync_request_message后，会调用handle_message()-->enqueue_sync_block()从本地查询num为101的signed_block，查询到了就放入队列，然后异步发送给B，发送完后继续调用enqueue_sync_block()，直至200(end_block)结束。
当B收到block_number为200(end_block)的signed_block后，会根据情况判断继续向A请求201-300的块呢，还是结束此次同步，后面会分析。
```
struct sync_request_message {
      uint32_t start_block;
      uint32_t end_block;
   };
void net_plugin_impl::handle_message( connection_ptr c, const sync_request_message &msg) {
      if( msg.end_block == 0) {
         c->peer_requested.reset();
         c->flush_queues();
      } else {
         c->peer_requested = sync_state( msg.start_block,msg.end_block,msg.start_block-1);
         c->enqueue_sync_block();
      }
}
bool connection::enqueue_sync_block() {
      controller& cc = app().find_plugin<chain_plugin>()->chain();
      if (!peer_requested)
         return false;
      uint32_t num = ++peer_requested->last;
      bool trigger_send = num == peer_requested->start_block;
      if(num == peer_requested->end_block) {
         peer_requested.reset();
      }
      try {
         signed_block_ptr sb = cc.fetch_block_by_number(num);
         if(sb) {
            enqueue( *sb, trigger_send);
            return true;
         }
      } catch ( ... ) {
         wlog( "write loop exception" );
      }
      return false;
}
```

##### signed_block

接着上面分析，当B向A发送了sync_request_message，而A在本地查询到signed_block并发给B后，B怎么处理呢。同样的，会交给handle_message()来处理，继续看关键源码。
```
void net_plugin_impl::handle_message( connection_ptr c, const signed_block &msg) {
      controller &cc = chain_plug->chain();
      block_id_type blk_id = msg.id();
      uint32_t blk_num = msg.block_num();
      
      try {
         if( cc.fetch_block_by_id(blk_id)) {
            sync_master->recv_block(c, blk_id, blk_num);
            return;
         }
      } catch( ...) {
      }

      dispatcher->recv_block(c, blk_id, blk_num);

      go_away_reason reason = fatal_other;
      try {
         signed_block_ptr sbp = std::make_shared<signed_block>(msg);
         chain_plug->accept_block(sbp); //, sync_master->is_active(c));
         reason = no_reason;
      } catch( ...) {
      }

      update_block_num ubn(blk_num);
      if( reason == no_reason ) {
         ...
         sync_master->recv_block(c, blk_id, blk_num);
      }
      else {
         sync_master->rejected_block(c, blk_num);
      }
   }
```
从源码可以看到，先在本地查询是否已经存在此ID的block，已经存在的话就调用sync_master->recv_block()，然后return。没有查询到就会调用dispatcher->recv_block()，将收到的block的信息保存到（或者修改）本地缓存。然后，将该signed_block交给chain_plug->accept_block()进行验证等后续操作，这里不深入讲解。如果验证的时候没有抛异常，就会调用sync_master->recv_block()，否则sync_master->rejected_block()。
看一下sync_manager::recv_block()方法的作用，在前面介绍的请求同步块的方法start_sync()中，B已经将同步状态设为lib_catchup，当处于该同步状态时，recv_block()会检测收到的块是不是想要的下一个块，比如B想要的是666，而A发来的是665或者888，那就说明A是来捣乱的，果断关闭A的连接。如果是B想要的，那就判断是不是最终要请求的区块(比如1000)，如果是最终的1000，则更改同步状态为in_sync，并向其他节点广播handshake_message，再来一遍握手交流；如果不是最终的1000，比如是200，则会继续请求下一组区块，比如201-300。
再说下sync_master->rejected_block()的作用，当检测到A发送过来的块是有问题的，说明A还是来捣乱的，那么就在rejected_block()方法中关闭A的连接，设置同步状态为in_sync，并广播handshake_message与其他节点再来一次亲密的握手交流。

##### packed_transaction

关于p2p消息，最后再来讲一下packed_transaction处理，同样看一下handle_message()方法的关键部分。
```
void net_plugin_impl::handle_message( connection_ptr c, const packed_transaction &msg) {
      transaction_id_type tid = msg.id();
      if(local_txns.get<by_id>().find(tid) != local_txns.end()) {
         return;
      }
      dispatcher->recv_transaction(c, tid);
      uint64_t code = 0;
      chain_plug->accept_transaction(msg, [=](const static_variant<fc::exception_ptr, transaction_trace_ptr>& result) {
         if (result.contains<fc::exception_ptr>()) {
         } else {
            auto trace = result.get<transaction_trace_ptr>();
            if (!trace->except) {
               dispatcher->bcast_transaction(msg);
               return;
            }
         }
         dispatcher->rejected_transaction(tid);
      });
   }
```
从源码中看到，首先会查找本地多索引缓存(local_txns)是否已经存在此transaction，已经存在的话就return，未查找到就dispatcher->recv_transaction()，在received_transactions(vector容器)中缓存一下该交易信息，然后会chain_plug->accept_transaction()验证交易的合法性，如果未出现异常，则dispatcher->bcast_transaction(msg)，将该交易保存到本地多索引容器中，并向其他未处于同步状态，也没有广播过此交易信息的连接广播此packed_transaction，其他节点收到后同样流程处理该packed_transaction。

从上面分析来看，不同nodeos之间会通过不同类型的message来表达自己的意图，告知对方自己的区块信息，并请求对方的信息，从而实现彼此之间的通信，同步链的区块/交易数据。

另外，上面介绍packed_transaction的时候，有说到handle_message()方法中验证交易合法后，该节点会向其他节点广播packed_transaction。EOS除了在handle_message()处理中进行交易的广播外，还采用boost signal绑定的方式，在验证交易或者区块合法后，通过emit方法将该交易或者区块向其他节点进行广播。比如nodeos将cleos发送过来transaction进行验证，验证通过后，会在本地缓存该交易并向其他节点广播这条交易，而正在出块的生产者，负责把该时段内的交易打包到区块中，生成新的区块，然后将该区块也广播出去，让其他的出块节点进行验证，区块验证采用DPoS共识+BFT算法，简单的说就是，如果验证通过的节点超过总出块节点的2/3，则该块就会变为不可逆区块。

----------------------
## HTTP网络

EOS客户端和服务器端的通信采用REST API方式，服务器端的每个资源或者合约执行对应一个唯一的URL地址，客户端将URL地址封装成http请求发送到服务器，请求对应的资源或者执行相应的操作。

### 客户端(cleos)

客户端是通过cleos命令行工具创建主命令、子命令和选项，并解析用户参数，然后调用对应命令的回调函数与nodeos/wallet进行交互，以转账为例，分析一下通信流程。
首先，假设已经设置了eosio.token合约
```
// Transfer subcommand
string con = "eosio.token";
auto transfer = app.add_subcommand("transfer", localized("Transfer EOS from account to account"), false);
transfer->add_option("sender", sender, localized("The account sending EOS"))->required();
transfer->add_option("recipient", recipient, localized("The account receiving EOS"))->required();
transfer->add_option("amount", amount, localized("The amount of EOS to send"))->required();
transfer->add_option("memo", memo, localized("The memo for the transfer"));
transfer->add_option("--contract,-c", con, localized("The contract which controls the token"));

add_standard_transaction_options(transfer, "sender@active");
transfer->set_callback([&] {
  signed_transaction trx;
  if (tx_force_unique && memo.size() == 0) {
     // use the memo to add a nonce
     memo = generate_nonce_string();
     tx_force_unique = false;
  }
  send_actions({create_transfer(con,sender, recipient, to_asset(amount), memo)});
});
```
上面代码中，主要分析send_actions({create_transfer(con,sender, recipient, to_asset(amount), memo)});
create_transfer生成一个chain::action对象，包括转账信息/权限等，然后send_actions()方法中调用push_actions()，将actions放入signed_transaction对象中，并调用push_transaction()方法，完成交易的打包，再调用call方法与服务器通信。
```
void send_actions(...) {
	auto result = push_actions( move(actions), extra_kcpu, compression);
}
fc::variant push_actions(...) {
   signed_transaction trx;
   trx.actions = std::forward<decltype(actions)>(actions);
   return push_transaction(trx, extra_kcpu, compression);
}
fc::variant push_transaction(...) {
	auto info = get_info();
    trx.expiration = info.head_block_time + tx_expiration;
    trx.set_reference_block(ref_block_id);
    trx.max_cpu_usage_ms = tx_max_net_usage;
    trx.max_net_usage_words = (tx_max_net_usage + 7)/8;
    if (!tx_skip_sign) {
        auto required_keys = determine_required_keys(trx);
        sign_transaction(trx, required_keys, info.chain_id);
    }
    if (!tx_dont_broadcast) {
    	//push_txn_func: "/v1/chain/push_transaction"
    	return call(push_txn_func, packed_transaction(trx, compression));
    } else {
    	return fc::variant(trx);
    }
}
fc::variant call( const std::string& url, const std::string& path, const T& v ) {
	return eosio::client::http::do_http_call( *cp, fc::variant(v), print_request, print_response );
}
fc::variant do_http_call(...){
	...
    re = do_txrx(socket, request, status_code);
}
```
在上面push_transaction()方法中，首先通过get_info()与服务器交互获取当前链信息，然后生成交易过期时间，设置引用的块号，max_cpu_usage_ms等，如果需要签名的话，调用sign_transaction()，与wallet交互进行签名，然后调用call方法，传入参数push_txn_func("/v1/chain/push_transaction")等。在call方法中调用do_http_call()完成对http请求的封装，然后调用do_txrx()方法发送封装好的http请求，并等待服务器返回结果。

### 服务器(nodeos)

首先服务器启动的时候，会调用chain_api_plugin的plugin_startup()，将url_handler注册到http_plugin_impl的map<string,url_handler>  url_handlers中，并监听http连接请求。
```
void chain_api_plugin::plugin_startup() {
   app().get_plugin<http_plugin>().add_api({
      CHAIN_RO_CALL(get_info, 200l),
      CHAIN_RO_CALL(get_block, 200),
      CHAIN_RO_CALL(get_block_header_state, 200),
      CHAIN_RO_CALL(get_account, 200),
      CHAIN_RO_CALL(get_code, 200),
      CHAIN_RO_CALL(get_abi, 200),
      CHAIN_RO_CALL(get_table_rows, 200),
      CHAIN_RO_CALL(get_currency_balance, 200),
      CHAIN_RO_CALL(get_currency_stats, 200),
      CHAIN_RO_CALL(get_producers, 200),
      CHAIN_RO_CALL(abi_json_to_bin, 200),
      CHAIN_RO_CALL(abi_bin_to_json, 200),
      CHAIN_RO_CALL(get_required_keys, 200),
      CHAIN_RW_CALL_ASYNC(push_block, chain_apis::read_write::push_block_results, 202),
      CHAIN_RW_CALL_ASYNC(push_transaction, chain_apis::read_write::push_transaction_results, 202),
      CHAIN_RW_CALL_ASYNC(push_transactions, chain_apis::read_write::push_transactions_results, 202)
   });
}
void add_api(const api_description& api) {
   for (const auto& call : api) 
      add_handler(call.first, call.second);
}
void http_plugin::add_handler(const string& url, const url_handler& handler) {
  app().get_io_service().post([=](){
    my->url_handlers.insert(std::make_pair(url,handler));
  });
}
```
当有客户端连接服务器，服务器通过http_plugin接收客户端的http请求，然后解析出请求的URL地址（resource）和消息内容（body），然后调用url_handlers中对应的回调函数进行处理，并将结果返回给cleos客户端。在转账这个例子中，会调用push_transaction()方法完成交易信息的验证/缓存/广播等后续处理，并将结果返回给客户端
```
void handle_http_request(...){
	auto body = con->get_request_body();
    auto resource = con->get_uri()->get_resource();
    auto handler_itr = url_handlers.find( resource );
    if( handler_itr != url_handlers.end()) {
      con->defer_http_response();
      handler_itr->second( resource, body, [con]( auto code, auto&& body ) {
         con->set_body( std::move( body ));
         con->set_status( websocketpp::http::status_code::value( code ));
         con->send_http_response();
      } );
    } 
}

```

# EOS 存储分析

## chain分析

### blockchain基本数据结构

- 区块结构:  
  
  1. block_header,定义在：libraries/chain/include/eosio/chain/block_header.hpp第7行

  ``` cpp
  struct block_header {
    block_timestamp_type             timestamp;            //区块产生时间
    account_name                     producer;             //区块生产者
    uint16_t                         confirmed = 1;        //dpos确认数
    block_id_type                    previous;             //前一个区块的头的hash值
    checksum256_type                 transaction_mroot;    //区块包含的transactions的merkel树根
    checksum256_type                 action_mroot;         //区块包含的actions的merkel树根，这些actions实际包含在transactions中
    uint32_t                         schedule_version = 0;
    optional<producer_schedule_type> new_producers;
    extension_type                   header_extension;
  }
  ```
  
  2. signed_block_header,定义在：libraries/chain/include/eosio/chain/block_header.hpp第45行

  ``` cpp
  struct signed_block_header : public block_header {
	  signature_type producer_signature; //生产者的签名
  }
  ```

  3. signed_block,定义在：libraries/chain/include/eosio/chain/block.hpp 57行

  ``` cpp
  struct signed_block : public signed_block_header {
	  vector<transacton_receipt> transactions; //区块包含的transactions执行后得到的回执
	  extrension_type            block_extensions;
  }
  ```

  4. transaction_receipt_header,定义在：libraries/chain/include/eosio/chain/block.hpp 12行

  ``` cpp
  struct transaction_receipt_header {
	  enum status_enum {
		  executed   = 0,   //transaction成功执行，没有错误发生
		  soft_fail  = 1,   //
		  hard_fail  = 2,
		  delayed    = 3,
		  expired    = 4
	  };

	  fc::enum_type<uint8_t,status_enum> status;
	  uint32_t                           cpu_usage_us;   //总CPU使用时间，单位为微秒
	  fc::unsigned_int                   net_usage_words;//总网络使用量
  }
  ```

  5. transaction_receipt,定义在：libraries/chain/include/eosio/chain/block.hpp 33行

  ``` cpp
  struct transaction_receipt : public transaction_receipt_header {
    fc::static_variant<transaction_id_type,packed_transaction> trx; //已经执行过的transactions
  }
  ```

  6. transaction_header,定义在：libraries/chain/include/eosio/chain/transaction.hpp 30行

  ``` cpp
  struct transaction_header {
    time_point_sec               expiration;               //过期时间
    uint16_t                     ref_block_num = 0U;       //用于TaPos验证
    uint32_t                     ref_block_prefix = 0UL;   //用于TaPos验证
    fc::unsigned_int             max_net_usage_words = 0UL;
    uint8_t                      max_cpu_usage_ms = 0;
    fc::unsigned_int             delay_sec;
  }
  ```

  7. transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 54行

  ``` cpp
  struct transaction : public transaction_header {
    vector<action>                 context_free_actions; //上下文无关的actions
    vector<action>                 actions;
    extension_type                 transaction_extensions;
  }
  ```

  8. signed_transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 78行

  ``` cpp
  struct signed_transaction : public transaction {
    vector<signature>         signatures;
    vector<bytes>             context_free_data; //和context_free_action一一对应
  }
  ```

  9. packed_transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 98行

  ``` cpp
  struct packed_transaction {
    enum compression_type {
      none  = 0,
      zlib  = 1
    }

    vector<signature_type>                  signatures;
    fc::enum_type<uint8_t,compression_type> compression;
    bytes                                   packed_context_free_data;
    bytes                                   packed_trx;
  }
  ```

  10. deferred_transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 157行
  
  ``` cpp
  struct deferred_transaction : public signed_transaction {
    uint128_t                  sender_id;
    account_name               sender;
    account_name               payer;
    time_point_sec             execute_after;
  }
  ```

  11. action，定义在：libraries/chain/include/eosio/chain/action.hpp 60行
   
   ``` cpp
   struct action {
       account_name             account;
       action_name              name;
       vector<permission_level> authorization;
       bytes                    data;
   }
   ```

  12. pending_state,定义在：libraries/chain/controller.cpp 91行,这是区块生产过程和区块同步过程中一个非常关键的数据结构
   
   ``` cpp
   struct pending_state {
       maybe_session            _db_session; //数据库session，主要涉及undo,squash,push相关操作，使数据库undo_state处于正确状态
       block_state_ptr          _pending_block_state;
       vector<action_receipt>   _actions;   //transactions在执行过程中生成的action_receipt，会打包到区块中(finalize_block)
       controller::block_status _block_status;
   }
   ```

  13. block_header_state,定义在：libraries/chain/include/eosio/chain/block_header_state.hpp 11行
   这个结构定义了验证transaction所需的头部信息，以及生成一个新的block所需的信息
   ``` cpp
   struct block_header_state {
       block_id_type             id;//最近的block_id
       uint32_t                  block_num = 0;//最近的block的高度/值
       signed_block_header       header;       //最近的block header;
       uint32_t                  dpos_proposed_irreversible_blocknum = 0;//最新的被提出dpos不可逆的区块高度/值，需要dpos计算确认
       uint32_t                  dpos_irreversible_blocknum = 0;//最新的dpos不可逆区块高度/值，这个是已经确认了的
       uint32_t                  bft_irreversible_block = 0;    //bft不可逆区块高度/值
       uint32_t                  pending_schedule_lib_num;      //
       digest_type               pending_schedule_hash;
       producer_schedule_type    pending_schedule;
       producer_schedule_type    active_schedule;
       incremental_merkel        block_root_merkle;
       flat_map<account_name,uint32_t> producer_to_last_produced;
       flat_map<account_name,uint32_t> procuer_to_last_implied_irb;
       public_key_type                 block_signing_key;        //当前生产者的签名
       vector<uint8_t>                 confirm_count;
       vector<header_confirmation>     confirmations;
   }
   ```

  14. block_state,定义在：libraries/chain/include/eosio/chain/block_state.hpp 14行
   ``` cpp
   struct block_state : public block_header_state {
       signed_block_ptr              block;            //前一个block指针
       bool                          validated = false;
       bool                          in_current_chain = false;
   }
   ```
  15. 

以上为EOS区块的关键数据结构，下面的分析都是围绕着以上的数据结构来进行的。数据结构之间的关系如下：  ![image](diagram/block_data_struct.png)

### producer_plugin

  producer_plugin实现了区块生产和区块同步的调用功能。  
  头文件定义在：plugins/producer_plugin/include/eosio/producer_plugin/producer_plugin.hpp  
  实现文件定义在：plugins/producer_plugin/producer_plugin.cpp  
  
  开始插件系统会调用producer_plugin::set_program_options函数进行相关程序项的设置：
  1. 生成config.ini文件(如果该文件不存在的话)
  2. 读取配置  
  调用producer_plugin::plugin_initialize函数进行初始化工作：  
  1. 初始化配置
  2. 设置信号函数

  调用producer_plugin::plugin_start函数，主要完成的功能如下：
  1. 设置信号函数：  
   ``` cpp
    my->_accepted_block_connection.emplace(chain.accepted_block.connect( [this]( const auto& bsp ){ my->on_block( bsp ); } ));
    my->_irreversible_block_connection.emplace(chain.irreversible_block.connect( [this]( const auto& bsp ){ my->on_irreversible_block( bsp->block ); } ));
   ```
  2. 获取最新的不可逆的区块号
  3. 进入生产区块的调度 producer_plugin_impl::schedule_production_loop

  producer_plugin_impl::schedule_production_loop:  
  1. 取消前一次的_timer操作：  
   ``` cpp
   chain::controller& chain = app().get_plugin<chain_plugin>().chain();
   _timer.cancel();
   std::weak_ptr<producer_plugin_impl> weak_this = shared_from_this();
   ```
  2. 调用 result = start_block(bool &last_block),函数定义在plugins/producer_plugin/producer_plugin.cpp 882行:  
   在该函数中：  
   * 首先会取得chain::controller的引用chain，判断chain当前的数据库模式是否为db_read_mode::READ_ONLY,如果是则返回状态start_block_result::waiting;如果不是则将当前的_pending_block_mode设为pending_block_mode::producing：  
   ``` cpp
   chain::controller& chain = app().get_plugin<chain_plugin>().chain();

   if( chain.get_read_mode() == chain::db_read_mode::READ_ONLY )
      return start_block_result::waiting;
   ```  
   * 计算当前节点是否为生产节点，获取当前被调度的生产者的watermark和signature:  
   ``` cpp
   last_block = ((block_timestamp_type(block_time).slot % config::producer_repetitions) == config::producer_repetitions - 1);
   const auto& scheduled_producer = hbs->get_scheduled_producer(block_time);
   auto currrent_watermark_itr = _producer_watermarks.find(scheduled_producer.producer_name);
   auto signature_provider_itr = _signature_providers.find(scheduled_producer.block_signing_key);
   auto irreversible_block_age = get_irreversible_block_age();
   ```
   * 进行一系列的条件判断:  
     检查当前节点是否被允许生产、被调度的生产者是否在生产队列中等：
   ``` cpp
   if( !_production_enabled ) {
      _pending_block_mode = pending_block_mode::speculating;
   } else if( _producers.find(scheduled_producer.producer_name) == _producers.end()) {
      _pending_block_mode = pending_block_mode::speculating;
   } else if (signature_provider_itr == _signature_providers.end()) {
      elog("Not producing block because I don't have the private key for ${scheduled_key}", ("scheduled_key", scheduled_producer.block_signing_key));
      _pending_block_mode = pending_block_mode::speculating;
   } else if ( _pause_production ) {
      elog("Not producing block because production is explicitly paused");
      _pending_block_mode = pending_block_mode::speculating;
   } else if ( _max_irreversible_block_age_us.count() >= 0 && irreversible_block_age >= _max_irreversible_block_age_us ) {
      elog("Not producing block because the irreversible block is too old [age:${age}s, max:${max}s]", ("age", irreversible_block_age.count() / 1'000'000)( "max", _max_irreversible_block_age_us.count() / 1'000'000 ));
      _pending_block_mode = pending_block_mode::speculating;
   }
   ```
   * 调用controller::abort_block
   * 调用controller::start_block 这两个函数在后文详述。
   * 在调用controller::start_block之后，在controller_impl中就会生成一个全新的pending包含了最新生成的区块头部信息  
     然后需要对新块进行transaction打包：  
     * 清理过期的transaction:  
       ```cpp
       // remove all persisted transactions that have now expired
      auto& persisted_by_id = _persistent_transactions.get<by_id>();
      auto& persisted_by_expiry = _persistent_transactions.get<by_expiry>();
      while(!persisted_by_expiry.empty() && persisted_by_expiry.begin()->expiry <= pbs->header.timestamp.to_time_point()) {
         persisted_by_expiry.erase(persisted_by_expiry.begin());
      }
       ```
     * 
  3. 判断start_block返回值：  
   * result == failed  
     start pending block 失败，稍后再试.启动定时器_timer，等待50ms再次进入schedule_production_loop
     ```cpp
      if (result == start_block_result::failed) {
      elog("Failed to start a pending block, will try again later");
      _timer.expires_from_now( boost::posix_time::microseconds( config::block_interval_us  / 10 ));

      // we failed to start a block, so try again later?
      //启动定时器，待会儿再试
      _timer.async_wait([weak_this,cid=++_timer_corelation_id](const boost::system::error_code& ec) {
         auto self = weak_this.lock();
         if (self && ec != boost::asio::error::operation_aborted && cid == self->_timer_corelation_id) {
            self->schedule_production_loop();
         }
      });
     }
     ```
   * result == waiting
    调用producer_plugin_impl::schedule_delayed_production
     ```cpp
     if (result == start_block_result::waiting){

         //这里检查生产者队列是否为空和是否被允许生产
      if (!_producers.empty() && !production_disabled_by_policy()) {
         fc_dlog(_log, "Waiting till another block is received and scheduling Speculative/Production Change");
         schedule_delayed_production_loop(weak_this, calculate_pending_block_time());
      } else {
         fc_dlog(_log, "Waiting till another block is received");
         // nothing to do until more blocks arrive
      }

     }
     ```
   * _pending_block_mode == producint && result == successed  
     启动定时器,在若干毫秒之后调用producer_plugin_impl::maybe_produce_block进行区块生产的完成工作，在这个时间段内当前节点收到的所有transaction都会被打进这个区块中。
   ```cpp
   if (_pending_block_mode == pending_block_mode::producing) {

      // we succeeded but block may be exhausted
      static const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
      if (result == start_block_result::succeeded) {
         // ship this block off no later than its deadline
         _timer.expires_at(epoch + boost::posix_time::microseconds(chain.pending_block_time().time_since_epoch().count() + (last_block ? _last_block_time_offset_us : _produce_time_offset_us)));
         fc_dlog(_log, "Scheduling Block Production on Normal Block #${num} for ${time}", ("num", chain.pending_block_state()->block_num)("time",chain.pending_block_time()));
      } else {
         auto expect_time = chain.pending_block_time() - fc::microseconds(config::block_interval_us);
         // ship this block off up to 1 block time earlier or immediately
         if (fc::time_point::now() >= expect_time) {
            _timer.expires_from_now( boost::posix_time::microseconds( 0 ));
         } else {
            _timer.expires_at(epoch + boost::posix_time::microseconds(expect_time.time_since_epoch().count()));
         }
         fc_dlog(_log, "Scheduling Block Production on Exhausted Block #${num} immediately", ("num", chain.pending_block_state()->block_num));
      }

      _timer.async_wait([&chain,weak_this,cid=++_timer_corelation_id](const boost::system::error_code& ec) {
         auto self = weak_this.lock();
         if (self && ec != boost::asio::error::operation_aborted && cid == self->_timer_corelation_id) {
            auto res = self->maybe_produce_block();
            fc_dlog(_log, "Producing Block #${num} returned: ${res}", ("num", chain.pending_block_state()->block_num)("res", res) );
         }
      });
   }
   ```
  4. producer_plugin_impl::maybe_produce_block,这个函数会调用producer_plugin_impl::produce_block完成区块生产：
   ```cpp
   //确保在异常退出候，scheudle_production_loop依然能够正常进行下去
   auto reschedule = fc::make_scoped_exit([this]{
      schedule_production_loop();
   });

   try {
       //完成区块的finalize_block,区块签名，更新fork_db
      produce_block();
      return true;
   } catch ( const guard_exception& e ) {
      app().get_plugin<chain_plugin>().handle_guard_exception(e);
      return false;
   } catch ( boost::interprocess::bad_alloc& ) {
      raise(SIGUSR1);
      return false;
   } FC_LOG_AND_DROP();

   fc_dlog(_log, "Aborting block due to produce_block error");
   chain::controller& chain = app().get_plugin<chain_plugin>().chain();
   chain.abort_block();
   return false;
   ```

  5. producer_plugin_impl::produce_block函数主要完成区块生产的主要工作包括：  
  * finalize_block:  
    更新资源限制  
    设置action merkle树根  
    设置transaction merkle树根
    ...在controller中有更详细说明
  * sign_block
   对block进行签名，防止被篡改
  * commit_block
   将新产生的区块加到数据库中，并将该区块广播出去。在controller有详细叙述

   ```cpp
   EOS_ASSERT(_pending_block_mode == pending_block_mode::producing, producer_exception, "called produce_block while not actually producing");
   chain::controller& chain = app().get_plugin<chain_plugin>().chain();
   const auto& pbs = chain.pending_block_state();
   const auto& hbs = chain.head_block_state();
   EOS_ASSERT(pbs, missing_pending_block_state, "pending_block_state does not exist but it should, another plugin may have corrupted it");
   auto signature_provider_itr = _signature_providers.find( pbs->block_signing_key );

   EOS_ASSERT(signature_provider_itr != _signature_providers.end(), producer_priv_key_not_found, "Attempting to produce a block for which we don't have the private key");

   //idump( (fc::time_point::now() - chain.pending_block_time()) );
   //完成块
   chain.finalize_block();
   //对块进行签名
   chain.sign_block( [&]( const digest_type& d ) {
      auto debug_logger = maybe_make_debug_time_logger();
      return signature_provider_itr->second(d);
   } );
   //提交块到数据库
   chain.commit_block();
   auto hbt = chain.head_block_time();
   //idump((fc::time_point::now() - hbt));

   block_state_ptr new_bs = chain.head_block_state();
   _producer_watermarks[new_bs->header.producer] = chain.head_block_num();

   ilog("Produced block ${id}... #${n} @ ${t} signed by ${p} [trxs: ${count}, lib: ${lib}, confirmed: ${confs}]",
        ("p",new_bs->header.producer)("id",fc::variant(new_bs->id).as_string().substr(0,16))
        ("n",new_bs->block_num)("t",new_bs->header.timestamp)
        ("count",new_bs->block->transactions.size())("lib",chain.last_irreversible_block_num())("confs", new_bs->header.confirmed));
   ```

   至此producer_plugin中区块的生产流程已经介绍完毕，更详细的分析会在controller中体现出来。  总体时序如下：  

    ![image](diagram/producer_sequence.png)

   区块同步流程：




### controller
producer_plugin在区块生产的过程中扮演着调度的角色，而实际工作是放在controller中来完成的，下面将纤细分析controller在区块生成过程中所扮演的角色功能：  
上文说到在producer_plugin_impl::start_block函数中会调用controller::abort_block和controller::start_block两个函数，这里需要展示一下controller相关数据结构,controller的功能主要是在controller_impl中实现的，这里只列举关键部分：
```cpp
struct controller {
    enum class block_status {
        irreversible = 0, //区块已经被应用,且不可逆
        validated = 1,    //区块已经被可信任的生产者签名，并已经应用但还不是不可逆状态
        complete = 2,     //区块已经被可信任的生产者签名，但是还没有被应用，状态为可逆
        incomplete = 3    //区块正在生产过程
    }；

    //信号量集合
    signal<void(const signed_block_ptr&)>         pre_accepted_block;
    signal<void(const block_state_ptr&)>          accepted_block_header;
    signal<void(const block_state_ptr&)>          accepted_block;
    signal<void(const block_state_ptr&)>          irreversible_block;
    signal<void(const transaction_metadata_ptr&)> accepted_transaction;
    signal<void(const transaction_trace_ptr&)>    applied_transaction;
    signal<void(const header_confirmation&)>      accepted_confirmation;
    signal<void(const int&)>                      bad_alloc;

    private:
        std::unique_ptr<controller_impl>   my;
};

struct controller_impl {
    controller&                  self;
    chainbase::database          db;   // state db,主要是存储合约执行后的各种状态信息
    chainbase::database          reversible_blocks; //用来存储已经成功应用但是还是可逆状态
    block_log                    blog;
    optional<pending_state>      pending;   //保存正在生成的block信息，该结构在上文已经列出
    block_state_ptr              head;      //上一次block state信息，该结构在上文已经列出
    fork_database                fork_db;
    wasm_interface               wasmif;
    resource_limits_manager      resource_limits;
    authorization_manager        authorization;
    ...
    /**
    *  Transactions that were undone by pop_block or abort_block, transactions
    *  are removed from this list if they are re-applied in other blocks. Producers
    *  can query this list when scheduling new transactions into blocks.
    */

    /**transaction的撤销由pop_block或abort_block来完成。如果有其他块重新应用了这些事物，则需要从该列表中将其删除。
    * 当新transaction被调度成块是，用户可以查询列表。
    * 从后面的分析中可以看到，abort_block并没有完成撤销工作
    */
    map<digest_type,transaction_metadata_ptr> unapplied_transactions;
    .
    .
    .
}

```
controller的初始化工作是由chain_plugin::plugin_initialize函数来完成的：检查白名单、黑名单、灰名单，数据库目录、检查点、及命令行参数的检查，主要功能定义在：plugins/chain_plugin/chain_plugin.cpp 314行。  
在chain_plugin中还负责相关channel的初始化工作。  
然后chain_plugin::plugin_start函数会将controller启动,定义在：plugins/chain_plugin/chain_plugin.cpp 633行：
```cpp
try {
   try {
       //controller启动
      my->chain->startup();
   } catch (const database_guard_exception& e) {
      log_guard_exception(e);
      // make sure to properly close the db
      my->chain.reset();
      throw;
   }

   if(!my->readonly) {
      ilog("starting chain in read/write mode");
   }

   ilog("Blockchain started; head block is #${num}, genesis timestamp is ${ts}",
        ("num", my->chain->head_block_num())("ts", (std::string)my->chain_config->genesis.initial_timestamp));

   my->chain_config.reset();
} FC_CAPTURE_AND_RETHROW()
```
在controller::startup中会调用controller_impl::add_index:  
这个函数主要为controller_impl::reversible_block和db添加索引：  
```cpp
      //为reversible block建立索引
      reversible_blocks.add_index<reversible_block_index>();

      db.add_index<account_index>();
      db.add_index<account_sequence_index>();

      db.add_index<table_id_multi_index>();
      db.add_index<key_value_index>();
      db.add_index<index64_index>();
      db.add_index<index128_index>();
      db.add_index<index256_index>();
      db.add_index<index_double_index>();
      db.add_index<index_long_double_index>();

      db.add_index<global_property_multi_index>();
      db.add_index<dynamic_global_property_multi_index>();
      db.add_index<block_summary_multi_index>();
      db.add_index<transaction_multi_index>();
      db.add_index<generated_transaction_multi_index>();

      authorization.add_indices();
      resource_limits.add_indices();
```
上述结构在后文有详细说明；然后进行fork_db的初始化工作，设置controller_impl::head,使其处于正确的状态为后续的区块生产做准备工作，到这里区块的初始化基本完成了，下面就到了区块生产的环节了。  

从上文我们知道producer_plugin::start_block最后会调用controller::abort_block和start_block两个函数,这两个函数最终会调用controller_impl::abort_block和controller_impl::start_block两个函数：  
controller_impl::abort_block重置controller_impl::pending信息，使pending处于全新状态:  
```cpp
if( pending ) {
    
    //这里只是将_pending_block_state中的transaction重新放到unapplied_transactions中，并没有做撤销工作
    if ( read_mode == db_read_mode::SPECULATIVE ) {
    for( const auto& t : pending->_pending_block_state->trxs )
        unapplied_transactions[t->signed_id] = t;
    }
    pending.reset();
}
```  
controller_impl::start_block函数接受三个参数：1.即将要产生的区块的时间戳when，2.区块确认数量confirm_block_count,3.区块当前的状态status:  
  * 判断controller_impl::pending是否为初始状态，否则抛出异常
    ```cpp
    EOS_ASSERT( !pending, block_validate_exception, "pending block already exists" );
    ```
  * 建立db session
    ```cpp
     if (!self.skip_db_sessions(s)) {
         EOS_ASSERT( db.revision() == head->block_num, database_exception, "db revision is not on par with head block",
                     ("db.revision()", db.revision())("controller_head_block", head->block_num)("fork_db_head_block", fork_db.head()->block_num) );

         pending.emplace(maybe_session(db));
      } else {
         pending.emplace(maybe_session());
      }
    ```
  * 根据最近的controller_impl::head生成新的pending
    ```cpp
    pending->_block_status = s;

    //这里会调用block_head::block_head(const block_header_state& prev, block_timestamp_type when)
    //然后调用block_state_head::generate_next根据传进来的时间戳when生成新的block_header_state(新块)
    //应为当前节点是正在出块的节点，所以在generate_next不需要对块进行完整性验证
    //在同步块的时候则需要调用next函数，并做完整性验证后面详述
    //generate_next代码定义在 libraries/chain/block_header_state.cpp 36行
    pending->_pending_block_state = std::make_shared<block_state>( *head, when ); // promotes pending schedule (if any) to active
    pending->_pending_block_state->in_current_chain = true;
    ```
  * 将出块action打进transaction并执行,然后清理过期的transactions更新生产者授权
    ```cpp
        try {
            auto onbtrx = std::make_shared<transaction_metadata>( get_on_block_transaction() );
            onbtrx->implicit = true;
            auto reset_in_trx_requiring_checks = fc::make_scoped_exit([old_value=in_trx_requiring_checks,this](){
                  in_trx_requiring_checks = old_value;
               });
            in_trx_requiring_checks = true;
            push_transaction( onbtrx, fc::time_point::maximum(), self.get_global_properties().configuration.min_transaction_cpu_usage, true );
         } catch( const boost::interprocess::bad_alloc& e  ) {
            elog( "on block transaction failed due to a bad allocation" );
            throw;
         } catch( const fc::exception& e ) {
            wlog( "on block transaction failed, but shouldn't impact block generation, system contract needs update" );
            edump((e.to_detail_string()));
         } catch( ... ) {
         }

         clear_expired_input_transactions();
         update_producers_authority();
    ```
  至此controller_impl::start_block函数分析完毕，其主要功能就是根据当前head生成新块，并将出块action打进transaction中。
  在controller_impl::start_block函数执行完毕候，控制权就交还给producer_plugin_impl::start_block了,在上文有对应的分析，producer_plugin_impl::start_block最终会把控制权交给producer_plugin_impl::schedule_production_loop，在这个函数中会启动一个定时器，在延迟一段时间之后会调用proudcer_plugin_impl::maybe_produce_block，这个函数会调用producer_plugin_impl::produce_block这在上文都有分析到，在producer_plugin_impl::produce_block中会调用:  
  controller::finalize_block,controller::sign_block和controller::commit_block三个函数来完成区块生产，区块签名，区块上链过程，下面来一次分析这三个函数：  
  * controller::finalize_block  
   这个函数主要是完成资源更新包括生产该区块所使用的cpu资源，带宽资源；设置action merkle树根；设置transaction merkle树根，创建block summary信息:
   ```cpp
   resource_limits.process_account_limit_updates();
    const auto& chain_config = self.get_global_properties().configuration;
    uint32_t max_virtual_mult = 1000;
    uint64_t CPU_TARGET = EOS_PERCENT(chain_config.max_block_cpu_usage, chain_config.target_block_cpu_usage_pct);
    resource_limits.set_block_parameters(
        { CPU_TARGET, chain_config.max_block_cpu_usage, config::block_cpu_usage_average_window_ms / config::block_interval_ms, max_virtual_mult, {99, 100}, {1000, 999}},
        {EOS_PERCENT(chain_config.max_block_net_usage, chain_config.target_block_net_usage_pct), chain_config.max_block_net_usage, config::block_size_average_window_ms / config::block_interval_ms, max_virtual_mult, {99, 100}, {1000, 999}}
    );
    resource_limits.process_block_usage(pending->_pending_block_state->block_num);

    //设置action merkle树根
    set_action_merkle();

    //设置transaction merkle树根
    set_trx_merkle();

    auto p = pending->_pending_block_state;
    p->id = p->header.id();

    //根据block id生成 summary信息并放到数据库中
    create_block_summary(p->id);
   ```
  * controller::sign_block  
   根据当前生产者提供的私钥签名函数对当前区块进行签名，并对做一次签名验证。
   ```cpp
    auto p = pending->_pending_block_state;
    p->sign( signer_callback );
    static_cast<signed_block_header&>(*p->block) = p->header;
   ```
   block_header_state::sign(上面p->sign)定义如下：
   ```cpp
    auto d = sig_digest();
    header.producer_signature = signer( d );
    EOS_ASSERT( block_signing_key == fc::crypto::public_key( header.producer_signature, d ), wrong_signing_key, "block is signed with unexpected key" );
   ```
  * controller::commit_block
   在详细分析这个函数之前需要先来分析一下fork_database这个类，它的结构如下：
   ```cpp
    struct by_block_id;
    struct by_block_num;
    struct by_lib_block_num;
    struct by_prev;

    //建立一个基于block_state_ptr的多索引容器
    //by_block_id以block id为索引
    //by_block_num 以区块高度为索引
    //by_lib_block_num以最近的区块不可逆高度为索引
    //by_prev以前一个block id为索引
    typedef multi_index_container<
        block_state_ptr,
        indexed_by<
          hashed_unique< tag<by_block_id>, member<block_header_state, block_id_type, &block_header_state::id>, std::hash<block_id_type>>,
          ordered_non_unique< tag<by_prev>, const_mem_fun<block_header_state, const block_id_type&, &block_header_state::prev> >,
          ordered_non_unique< tag<by_block_num>,
              composite_key< block_state,
                member<block_header_state,uint32_t,&block_header_state::block_num>,
                member<block_state,bool,&block_state::in_current_chain>
              >,
              composite_key_compare< std::less<uint32_t>, std::greater<bool> >
          >,
          ordered_non_unique< tag<by_lib_block_num>,
              composite_key< block_header_state,
                  member<block_header_state,uint32_t,&block_header_state::dpos_irreversible_blocknum>,
                  member<block_header_state,uint32_t,&block_header_state::bft_irreversible_blocknum>,
                  member<block_header_state,uint32_t,&block_header_state::block_num>
              >,
              composite_key_compare< std::greater<uint32_t>, std::greater<uint32_t>, std::greater<uint32_t> >
          >
        >
    > fork_multi_index_type;
    struct fork_database_impl {
      fork_multi_index_type    index;
      block_state_ptr          head; //区块头
      fc::path                 datadir; //存储路径
    }

    class fork_database {
    public:
      //这里列举关键函数,详细定义参见 libraries/chain/include/eosio/chain/fork_database.hpp

      //根据区块id获取block_state信息
      block_state_ptr get_block(const block_id_type &id) const;
      //根据区块高度从当前链中获取block_state信息
      block_state_ptr get_block_in_current_chain_by_num(uint32_t num) const;

      //提供一个“有效的”区块状态，有可能以此建立分支
      void set(block_state_ptr s);
      
      block_state_ptr add(signed_block_ptr b,bool trust = false);

      block_state_ptr add(block_state_ptr next_block);
      void remove(const block_id_type &id);
      void add(const header_confirmation &c);
      const block_state_ptr &head() const;

      //根据两个头block，获取两个分支（两个分支有共同的祖先，即两个头部的previous的值相同）
      pair<branch_type,branch_type> fetch_branch_from(const block_id_type &first,const block_id_type &second) const;

      //若该区块为invalid,将会从数据库中删除。若为valid，在发射irreversible信号后，所有比LIB大的block将会被修正
      void set_validity(const block_state_ptr &h,bool valid);
      void mark_in_current_chain(const block_state_ptr &h,bool in_current_chain);
      void prune(const block_state_ptr&);

      signal<void(block_state_ptr)>    irreversible;

    private:
      void set_bft_irreversible(block_id_type id);
      unique_ptr<for_database_impl> my;
    }

   ```
  回到controller_impl::commit_block,该接受一个bool参数，该参数表示是否需要将controller_impl::pending->_pending_block_state加入fork_database,如果是则将pending->_pending_block_state->validated设为true,然后调用fork_database::add(block_state_ptr)将该块加入数据库，然后会根据当前的block_state进行数据库数据修正(后文fork_database部分有详细分析),然后检查是否正在重演该区块，如果否则将其加入可以缓存reversible_blocks,发射accept_block信号,该信号会调用net_plugin_impl::accept_block，函数，这些信号量的设置定义在plugins/net_plugin/net_plugin.cpp 3017行：
  ```cpp
    chain::controller&cc = my->chain_plug->chain();
    {
        cc.accepted_block_header.connect( boost::bind(&net_plugin_impl::accepted_block_header, my.get(), _1));
        cc.accepted_block.connect(  boost::bind(&net_plugin_impl::accepted_block, my.get(), _1));
        cc.irreversible_block.connect( boost::bind(&net_plugin_impl::irreversible_block, my.get(), _1));
        cc.accepted_transaction.connect( boost::bind(&net_plugin_impl::accepted_transaction, my.get(), _1));
        cc.applied_transaction.connect( boost::bind(&net_plugin_impl::applied_transaction, my.get(), _1));
        cc.accepted_confirmation.connect( boost::bind(&net_plugin_impl::accepted_confirmation, my.get(), _1));
    }
  ```
  commit_block关键代码如下：
  ```cpp
    try {
        if (add_to_fork_db) {
          pending->_pending_block_state->validated = true;
          auto new_bsp = fork_db.add(pending->_pending_block_state);
          emit(self.accepted_block_header, pending->_pending_block_state);

          //更新head到最新生成的区块头
          head = fork_db.head();
          EOS_ASSERT(new_bsp == head, fork_database_exception, "committed block did not become the new head in fork database");
        }

        if( !replaying ) {
          reversible_blocks.create<reversible_block_object>( [&]( auto& ubo ) {
              ubo.blocknum = pending->_pending_block_state->block_num;
              ubo.set_block( pending->_pending_block_state->block );
          });
        }

        emit( self.accepted_block, pending->_pending_block_state );
    } catch (...) {
        // dont bother resetting pending, instead abort the block
        reset_pending_on_exit.cancel();
        abort_block();
        throw;
    }
  ```
  至此controller_impl::commit_block工作完成。控制权回到producer_plugin_impl::produce_block,一次block生产调度就完成了，然后进入下一次调度。

  fork_database分析：  
  结构如下：
  ```cpp

    struct by_block_id;
    struct by_block_num;
    struct by_lib_block_num;
    struct by_prev;

    //建立一个基于block_state_ptr的多索引容器
    //by_block_id以block id为索引
    //by_block_num 以区块高度为索引，组合键<block_num,in_current_chain>,降序
    //by_lib_block_num以最近的区块不可逆高度为索引，组合键<dpos_irreversible_blocknum,bft_irreversible_blocknum,block_num>，升序
    //by_prev以前一个block id为索引
    typedef multi_index_container<
        block_state_ptr,
        indexed_by<
          hashed_unique< tag<by_block_id>, member<block_header_state, block_id_type, &block_header_state::id>, std::hash<block_id_type>>,
          ordered_non_unique< tag<by_prev>, const_mem_fun<block_header_state, const block_id_type&, &block_header_state::prev> >,
          ordered_non_unique< tag<by_block_num>,
              composite_key< block_state,
                member<block_header_state,uint32_t,&block_header_state::block_num>,
                member<block_state,bool,&block_state::in_current_chain>
              >,
              composite_key_compare< std::less<uint32_t>, std::greater<bool> >
          >,
          ordered_non_unique< tag<by_lib_block_num>,
              composite_key< block_header_state,
                  member<block_header_state,uint32_t,&block_header_state::dpos_irreversible_blocknum>,
                  member<block_header_state,uint32_t,&block_header_state::bft_irreversible_blocknum>,
                  member<block_header_state,uint32_t,&block_header_state::block_num>
              >,
              composite_key_compare< std::greater<uint32_t>, std::greater<uint32_t>, std::greater<uint32_t> >
          >
        >
    > fork_multi_index_type;
    struct fork_database_impl {
      fork_multi_index_type    index;
      block_state_ptr          head; //区块头
      fc::path                 datadir; //存储路径
    }

    class fork_database {
    public:
      //这里列举关键函数,详细定义参见 libraries/chain/include/eosio/chain/fork_database.hpp

      //根据区块id获取block_state信息
      block_state_ptr get_block(const block_id_type &id) const;
      //根据区块高度从当前链中获取block_state信息
      block_state_ptr get_block_in_current_chain_by_num(uint32_t num) const;

      //提供一个“有效的”区块状态，有可能以此建立分支
      void set(block_state_ptr s);
      
      block_state_ptr add(signed_block_ptr b,bool trust = false);

      block_state_ptr add(block_state_ptr next_block);
      void remove(const block_id_type &id);
      void add(const header_confirmation &c);
      const block_state_ptr &head() const;

      //根据两个头block，获取两个分支（两个分支有共同的祖先，即两个头部的previous的值相同）
      pair<branch_type,branch_type> fetch_branch_from(const block_id_type &first,const block_id_type &second) const;

      //若该区块为invalid,将会从数据库中删除。若为valid，在发射irreversible信号后，所有比LIB大的block将会被修正
      void set_validity(const block_state_ptr &h,bool valid);
      void mark_in_current_chain(const block_state_ptr &h,bool in_current_chain);
      void prune(const block_state_ptr&);

      signal<void(block_state_ptr)>    irreversible;

    private:
      void set_bft_irreversible(block_id_type id);
      unique_ptr<for_database_impl> my;
    }

  ```
  下面一次解释每个函数的实现：  
  1. void fork_database::set(block_state_ptr s)
   ```cpp
    //将s插入多索引容器中
    auto result = my->index.insert( s );
      EOS_ASSERT( s->id == s->header.id(), fork_database_exception, 
                  "block state id (${id}) is different from block state header id (${hid})", ("id", string(s->id))("hid", string(s->header.id())) );

         //FC_ASSERT( s->block_num == s->header.block_num() );

      EOS_ASSERT( result.second, fork_database_exception, "unable to insert block state, duplicate state detected" );


      //更新head状态
      if( !my->head ) {
         my->head =  s;
      } else if( my->head->block_num < s->block_num ) {
         my->head =  s;
      }
   ```
  2. 
  3. 
  4. 


  transaction执行，涉及到的关键数据结构如下：
  ```cpp

    struct action_receipt {
      account_name        receiver;                //执行该action的account
      digest_type         act_digest;
      uint64_t            global_sequence = 0;
      uint64_t            recv_sequence = 0;
      flat_map<account_name,uint64_t> auth_sequence;
      fc::unsigned_int    code_sequence;
      fc::unsigned_int    abi_sequence;
    };

    struct base_action_trace {
      action_receipt      receipt;
      action              act;
      fc::microseconds    elapsed;
      uint64_t            cpu_usage = 0;
      string              console;
      uint64_t            total_cpu_usage = 0;
      transaction_id_type trx_id;
    }

    struct action_trace : public base_action_trace {
      vector<action_trace> inline_traces;
    }

    struct transaction_trace {
      transaction_id_type                      id;
      fc::optional<transaction_receipt_header> receipt;
      fc::microseconds                         elapsed;
      uint64_t                                 net_usage;
      bool                                     scheduled = false;
      vector<action_trace>                     action_traces;
      transaction_trace_ptr                    failed_dtrx_trace;
      fc::optional<fc::exception>              except;
      std::exception_ptr                       except_ptr;
    }

  ```

  一个transaction是由一个或多个action组成的，这些action如果又一个失败了，那么该transaction也就失败了，已经执行过的action需要回滚。每个transaction必须在30ms内完成，如果一个包含了多个action且这些action执行时间总和超过30ms，则整个transaction失败。

## chainbase分析

### database基本数据结构
  和数据库相关的数据结构均派生自 struct object,结构如下：
  ```cpp
  template<typename T>
    class oid {
    public:
        oid( int64_t i = 0 ):_id{i}{}
        oid& operator++() {
            ++_id;
            return *this;
        }

        friend bool operator < ( const oid& a,const oid& b ) {
            return a._id < b._id;
        }

        friend bool operator > ( const oid& a,const oid& b ) {
            return a._id > b._id;
        }

        friend bool operator == ( const oid& a,const oid& b ) {
            return a._id == b._id;
        }

        friend bool operator != ( const oid& a,const oid& b ) {
            return a._id != b._id;
        }

        friend std::ostream& operator << ( std::ostream& s,const oid& id ) {
            s << boost::core::demangle( typeid( oid<T> ).name() ) << '(' << id._id << ')';
            return s;
        }

        int64_t _id;
    };
    template<uint16_t TypeNumber,typename Derived>
    struct object {
      typedef oid<Derived> id_type;
      static const uint16_t type_id = TypeNumber; //类型标识
    };
  ```

  数据库的索引是通过元编程来实现的，每一种数据类型都有一个唯一id作为标识。程序在运行过程中要产生27个数据表：
  1. account_object:  
      保存账户信息，结构如下：  
      ```cpp
      class account_object : public chainbase::object<account_object_type,account_objct> {
        OBJECT_CTOR(account_object,(code)(abi))
        id_type              id;
        account_name         name;                  //账户名称base32编码
        uint8_t              vm_type      = 0;      // vm_type
        uint8_t              vm_version   = 0;      // vm_version
        bool                 privileged   = false;  // 是否优先

        time_point           last_code_update;      //上次参与权限验证的时间
        digest_type          code_version;
        block_timestamp_type creation_date;         //创建时间

        shared_string  code;
        shared_string  abi;

        void set_abi( const eosio::chain::abi_def& a ) {
          abi.resize( fc::raw::pack_size( a ) );
          fc::datastream<char*> ds( abi.data(), abi.size() );
          fc::raw::pack( ds, a );
        }

        eosio::chain::abi_def get_abi()const {
          eosio::chain::abi_def a;
          EOS_ASSERT( abi.size() != 0, abi_not_found_exception, "No ABI set on account ${n}", ("n",name) );

          fc::datastream<const char*> ds( abi.data(), abi.size() );
          fc::raw::unpack( ds, a );
          return a;
        }
      };
      ```
      其中宏OBJECT_CTOR(account_object,(code)(abi))展开如下：
      ```
      account_object() = delete; 
      public: 
      template<typename Constructor, typename Allocator> 
      account_object(Constructor&& c, chainbase::allocator<Allocator> a) : id(0) ,code(a) ,abi(a) { c(*this); }
      ```
      该结构保存了账户的信息，对应的多索引容器为：
      ```
      struct by_name;
      using account_index = chainbase::shared_multi_index_container<
          account_object,
          indexed_by<
            ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
            ordered_unique<tag<by_name>, member<account_object, account_name, &account_object::name>>
          >
      >;
      ```
      创建一个账户的函数调用在libraries/chain/eos_contract.cpp void apply_eosio_newaccount(apply_context& context)函数中.
  2. account_sequence_object  
    这个结构用来存储和账户相关的序列数据，具体结构如下：
      ```
      class account_sequence_object : public chainbase::object<account_sequence_object_type, account_sequence_object>
      {
          OBJECT_CTOR(account_sequence_object);

          id_type      id;
          account_name name;
          uint64_t     recv_sequence = 0;
          uint64_t     auth_sequence = 0;
          uint64_t     code_sequence = 0;
          uint64_t     abi_sequence  = 0;
      };
      ```
      对应的多索引容器如下：
      ```
      struct by_name;
      using account_sequence_index = chainbase::shared_multi_index_container<
          account_sequence_object,
          indexed_by<
            ordered_unique<tag<by_id>, member<account_sequence_object, account_sequence_object::id_type, &account_sequence_object::id>>,
            ordered_unique<tag<by_name>, member<account_sequence_object, account_name, &account_sequence_object::name>>
          >
      >;
      ```
  3. permission_object  
   用来存储授权相关信息，具体结构如下：
      ```
      class permission_object : public chainbase::object<permission_object_type, permission_object> {
      OBJECT_CTOR(permission_object, (auth) )

        id_type                           id;
        permission_usage_object::id_type  usage_id;
        id_type                           parent; ///< parent permission
        account_name                      owner; ///< the account this permission belongs to
        permission_name                   name; ///< human-readable name for the permission
        time_point                        last_updated; ///< the last time this authority was updated
        shared_authority                  auth; ///< authority required to execute this permission


        /**
        * @brief Checks if this permission is equivalent or greater than other
        * @tparam Index The permission_index
        * @return true if this permission is equivalent or greater than other, false otherwise
        *
        * Permissions are organized hierarchically such that a parent permission is strictly more powerful than its
        * children/grandchildren. This method checks whether this permission is of greater or equal power (capable of
        * satisfying) permission @ref other.
        */
        template <typename Index>
        bool satisfies(const permission_object& other, const Index& permission_index) const {
          // If the owners are not the same, this permission cannot satisfy other
          if( owner != other.owner )
              return false;

          // If this permission matches other, or is the immediate parent of other, then this permission satisfies other
          if( id == other.id || id == other.parent )
              return true;

          // Walk up other's parent tree, seeing if we find this permission. If so, this permission satisfies other
          const permission_object* parent = &*permission_index.template get<by_id>().find(other.parent);
          while( parent ) {
              if( id == parent->parent )
                return true;
              if( parent->parent._id == 0 )
                return false;
              parent = &*permission_index.template get<by_id>().find(parent->parent);
          }
          // This permission is not a parent of other, and so does not satisfy other
          return false;
        }
      };
      ```
      对应的多索引容器为：
        ```
        struct by_parent;
        struct by_owner;
        struct by_name;
        using permission_index = chainbase::shared_multi_index_container<
            permission_object,
            indexed_by<
              ordered_unique<tag<by_id>, member<permission_object, permission_object::id_type, &permission_object::id>>,
              ordered_unique<tag<by_parent>,
                  composite_key<permission_object,
                    member<permission_object, permission_object::id_type, &permission_object::parent>,
                    member<permission_object, permission_object::id_type, &permission_object::id>
                  >
              >,
              ordered_unique<tag<by_owner>,
                  composite_key<permission_object,
                    member<permission_object, account_name, &permission_object::owner>,
                    member<permission_object, permission_name, &permission_object::name>
                  >
              >,
              ordered_unique<tag<by_name>,
                  composite_key<permission_object,
                    member<permission_object, permission_name, &permission_object::name>,
                    member<permission_object, permission_object::id_type, &permission_object::id>
                  >
              >
            >
        >;
        ```

  4. permission_usage_object  
   保存了授权的使用信息，具体结构如下：
   ```
   class permission_usage_object : public chainbase::object<permission_usage_object_type, permission_usage_object> {
      OBJECT_CTOR(permission_usage_object)

      id_type           id;
      time_point        last_used;   ///< when this permission was last used
   };
   ```
   对应的多索引容器为：
   ```
    struct by_account_permission;
    using permission_usage_index = chainbase::shared_multi_index_container<
        permission_usage_object,
        indexed_by<
          ordered_unique<tag<by_id>, member<permission_usage_object, permission_usage_object::id_type, &permission_usage_object::id>>
        >
    >;
   ```
  5. permission_link_object  
   这个类记录了contract 和 action之间的permission_object的链接，以记录这些contract在执行的过程中所需要的权限
   ```
    class permission_link_object : public chainbase::object<permission_link_object_type, permission_link_object> {
        OBJECT_CTOR(permission_link_object)

        id_type        id;
        /// The account which is defining its permission requirements
        account_name    account;
        /// The contract which account requires @ref required_permission to invoke
        account_name    code; /// TODO: rename to scope
        /// The message type which account requires @ref required_permission to invoke
        /// May be empty; if so, it sets a default @ref required_permission for all messages to @ref code
        action_name       message_type;
        /// The permission level which @ref account requires for the specified message types
        permission_name required_permission;
    };
   ```
   对应的索引如下：
   ```
   struct by_action_name;
   struct by_permission_name;
   using permission_link_index = chainbase::shared_multi_index_container<
      permission_link_object,
      indexed_by<
         ordered_unique<tag<by_id>,
            BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_link_object::id_type, id)
         >,
         ordered_unique<tag<by_action_name>,
            composite_key<permission_link_object,
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, code),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, action_name, message_type)
            >
         >,
         ordered_unique<tag<by_permission_name>,
            composite_key<permission_link_object,
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_name, required_permission),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, code),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, action_name, message_type)
            >
         >
      >
   >;
   ```
  6. key_value_object  
    结构如下：
      ```
        struct key_value_object : public chainbase::object<key_value_object_type, key_value_object> {
          OBJECT_CTOR(key_value_object, (value))

          typedef uint64_t key_type;
          static const int number_of_keys = 1;

          id_type               id;
          table_id              t_id;
          uint64_t              primary_key; //主键
          account_name          payer = 0;
          shared_string         value;      //值
        };
      ```
      对应的索引：
      ```
          using key_value_index = chainbase::shared_multi_index_container<
          key_value_object,
          indexed_by<
            ordered_unique<tag<by_id>, member<key_value_object, key_value_object::id_type, &key_value_object::id>>,
            ordered_unique<tag<by_scope_primary>,
                composite_key< key_value_object,
                  member<key_value_object, table_id, &key_value_object::t_id>,
                  member<key_value_object, uint64_t, &key_value_object::primary_key>
                >,
                composite_key_compare< std::less<table_id>, std::less<uint64_t> >
            >
          >
      >;
      ```
  7. index64_object  
    是基于多索引容器建立的一个二级索引，定义如下：
    ```
    typedef secondary_index<uint64_t,index64_object_type>::index_object   index64_object;
    typedef secondary_index<uint64_t,index64_object_type>::index_index    index64_index;
    ```
  8. index128_object
   同上
  9.  index256_object  
    同上
  10. index_double_object  
    同上
  11. index_long_double_object  
   同上
  12. global_property_object
   存储了初始设定的值，用来调用块参数：
   ```
   class global_property_object : public chainbase::object<global_property_object_type, global_property_object>
   {
      OBJECT_CTOR(global_property_object, (proposed_schedule))

      id_type                           id;
      optional<block_num_type>          proposed_schedule_block_num;
      shared_producer_schedule_type     proposed_schedule;
      chain_config                      configuration;
   };
   ```
   对应的索引：
   ```
   using dynamic_global_property_multi_index = chainbase::shared_multi_index_container<
      dynamic_global_property_object,
      indexed_by<
         ordered_unique<tag<by_id>,
            BOOST_MULTI_INDEX_MEMBER(dynamic_global_property_object, dynamic_global_property_object::id_type, id)
         >
      >
   >;
   ```
  13. dynamic_global_property_object  
   记录了区块链正常操作期间所计算的值，这些值反映了区块链的当前的全局的值：
   ```
   class dynamic_global_property_object : public chainbase::object<dynamic_global_property_object_type, dynamic_global_property_object>
   {
        OBJECT_CTOR(dynamic_global_property_object)

        id_type    id;
        uint64_t   global_action_sequence = 0;
   };

   ```
   对应的索引为：
   ```
   using global_property_multi_index = chainbase::shared_multi_index_container<
      global_property_object,
      indexed_by<
         ordered_unique<tag<by_id>,
            BOOST_MULTI_INDEX_MEMBER(global_property_object, global_property_object::id_type, id)
         >
      >
   >;
   ```
  14. block_summary_object  
    block的一个简明信息，用于transaction的TaPos验证。结构如下：
    ```
     class block_summary_object : public chainbase::object<block_summary_object_type, block_summary_object>
    {
          OBJECT_CTOR(block_summary_object)

          id_type        id;
          block_id_type  block_id;
    };
    ```
    对应的索引为：
    ```
    struct by_block_id;
    using block_summary_multi_index = chainbase::shared_multi_index_container<
        block_summary_object,
        indexed_by<
          ordered_unique<tag<by_id>, BOOST_MULTI_INDEX_MEMBER(block_summary_object, block_summary_object::id_type, id)>
    //      ordered_unique<tag<by_block_id>, BOOST_MULTI_INDEX_MEMBER(block_summary_object, block_id_type, block_id)>
        >
    >;
    ```
    在controller::finalize_block函数中，会产生一个该结构的记录：
    ```
      set_action_merkle();
      set_trx_merkle();

      auto p = pending->_pending_block_state;
      p->id = p->header.id();

      create_block_summary(p->id); //创建一个block_summary

    ```
  15. transaction_object  
   记录了transaction的过期时间，在该过期时间内，如果该transaction还没得倒确认，则会删除：
      ```
      class transaction_object : public chainbase::object<transaction_object_type, transaction_object>
      {
            OBJECT_CTOR(transaction_object)

            id_type             id;
            time_point_sec      expiration;
            transaction_id_type trx_id;
      };
      ```
  对应的索引为：

  ```
   struct by_expiration;
   struct by_trx_id;
   using transaction_multi_index = chainbase::shared_multi_index_container<
      transaction_object,
      indexed_by<
         ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_object::id_type, id)>,
         ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER(transaction_object, transaction_id_type, trx_id)>,
         ordered_unique< tag<by_expiration>,
            composite_key< transaction_object,
               BOOST_MULTI_INDEX_MEMBER( transaction_object, time_point_sec, expiration ),
               BOOST_MULTI_INDEX_MEMBER( transaction_object, transaction_object::id_type, id)
            >
         >
      >
   >;
  ```
  在transaction执行的时候，会对收到的transaction做一个初始化工作，transaction_context::init_for_input_trx会调用该函数产生一个transaction_object记录：
  ```
    published = control.pending_block_time();
      is_input = true;
      if (!control.skip_trx_checks()) {
         control.validate_expiration(trx);
         control.validate_tapos(trx);
         control.validate_referenced_accounts(trx);
      }
      init( initial_net_usage);
      if (!skip_recording)
         record_transaction( id, trx.expiration ); /// checks for dupes
  ```
  16. generated_transaction_object  
   结构如下：
   ```
   class generated_transaction_object : public chainbase::object<generated_transaction_object_type, generated_transaction_object>
   {
         OBJECT_CTOR(generated_transaction_object, (packed_trx) )

         id_type                       id;
         transaction_id_type           trx_id;
         account_name                  sender;
         uint128_t                     sender_id = 0; /// ID given this transaction by the sender
         account_name                  payer;
         time_point                    delay_until; /// this generated transaction will not be applied until the specified time
         time_point                    expiration; /// this generated transaction will not be applied after this time
         time_point                    published;
         shared_string                 packed_trx;

         uint32_t set( const transaction& trx ) {
            auto trxsize = fc::raw::pack_size( trx );
            packed_trx.resize( trxsize );
            fc::datastream<char*> ds( packed_trx.data(), trxsize );
            fc::raw::pack( ds, trx );
            return trxsize;
         }
   };

   ```
   对应的索引：
   ```
   struct by_trx_id;
   struct by_expiration;
   struct by_delay;
   struct by_status;
   struct by_sender_id;

   using generated_transaction_multi_index = chainbase::shared_multi_index_container<
      generated_transaction_object,
      indexed_by<
         ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(generated_transaction_object, generated_transaction_object::id_type, id)>,
         ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, transaction_id_type, trx_id)>,
         ordered_unique< tag<by_expiration>,
            composite_key< generated_transaction_object,
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, expiration),
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
            >
         >,
         ordered_unique< tag<by_delay>,
            composite_key< generated_transaction_object,
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, delay_until),
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
            >
         >,
         ordered_unique< tag<by_sender_id>,
            composite_key< generated_transaction_object,
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, account_name, sender),
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, uint128_t, sender_id)
            >
         >
      >
   >;
   ```
  17. producer_object  
   结构如下：
   ```
    class producer_object : public chainbase::object<producer_object_type, producer_object> {
      OBJECT_CTOR(producer_object)

      id_type            id;
      account_name       owner;
      uint64_t           last_aslot = 0;
      public_key_type    signing_key;
      int64_t            total_missed = 0;
      uint32_t           last_confirmed_block_num = 0;


        /// The blockchain configuration values this producer recommends
        chain_config       configuration;
    };
   ```
   对应的索引：
   ```
   struct by_key;
    struct by_owner;
    using producer_multi_index = chainbase::shared_multi_index_container<
      producer_object,
      indexed_by<
          ordered_unique<tag<by_id>, member<producer_object, producer_object::id_type, &producer_object::id>>,
          ordered_unique<tag<by_owner>, member<producer_object, account_name, &producer_object::owner>>,
          ordered_unique<tag<by_key>,
            composite_key<producer_object,
                member<producer_object, public_key_type, &producer_object::signing_key>,
                member<producer_object, producer_object::id_type, &producer_object::id>
            >
          >
      >
    >;
   ```
  18. account_control_history_object
  19. public_key_history_object
  20. table_id_object
   结构如下：
   ```
    class table_id_object : public chainbase::object<table_id_object_type, table_id_object> {
        OBJECT_CTOR(table_id_object)

        id_type        id;
        account_name   code;
        scope_name     scope;
        table_name     table;
        account_name   payer;
        uint32_t       count = 0; /// the number of elements in the table
    };
   ```
   对应的索引：
   ```
   struct by_code_scope_table;

   using table_id_multi_index = chainbase::shared_multi_index_container<
      table_id_object,
      indexed_by<
         ordered_unique<tag<by_id>,
            member<table_id_object, table_id_object::id_type, &table_id_object::id>
         >,
         ordered_unique<tag<by_code_scope_table>,
            composite_key< table_id_object,
               member<table_id_object, account_name, &table_id_object::code>,
               member<table_id_object, scope_name,   &table_id_object::scope>,
               member<table_id_object, table_name,   &table_id_object::table>
            >
         >
      >
   >;
   ```
  21. resource_limits_object  
   结构如下：
   ```
    struct resource_limits_object : public chainbase::object<resource_limits_object_type, resource_limits_object> {

      OBJECT_CTOR(resource_limits_object)

      id_type id;
      account_name owner;
      bool pending = false;

      int64_t net_weight = -1;
      int64_t cpu_weight = -1;
      int64_t ram_bytes = -1;

   };
   ```
   对应的索引:
   ```
   struct by_owner;
   struct by_dirty;

   using resource_limits_index = chainbase::shared_multi_index_container<
      resource_limits_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_limits_object, resource_limits_object::id_type, &resource_limits_object::id>>,
         ordered_unique<tag<by_owner>,
            composite_key<resource_limits_object,
               BOOST_MULTI_INDEX_MEMBER(resource_limits_object, bool, pending),
               BOOST_MULTI_INDEX_MEMBER(resource_limits_object, account_name, owner)
            >
         >
      >
   >;
   ```
  22. resource_usage_object  
   结构如下：
   ```
   struct resource_usage_object : public chainbase::object<resource_usage_object_type, resource_usage_object> {
      OBJECT_CTOR(resource_usage_object)

      id_type id;
      account_name owner;

      usage_accumulator        net_usage;
      usage_accumulator        cpu_usage;

      uint64_t                 ram_usage = 0;
   };

   ```
   对应的索引：
   ```
   using resource_usage_index = chainbase::shared_multi_index_container<
      resource_usage_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_usage_object, resource_usage_object::id_type, &resource_usage_object::id>>,
         ordered_unique<tag<by_owner>, member<resource_usage_object, account_name, &resource_usage_object::owner> >
      >
   >;
   ```
  23. resource_limits_config_object  
   结构如下：
   ```
   class resource_limits_config_object : public chainbase::object<resource_limits_config_object_type, resource_limits_config_object> {
      OBJECT_CTOR(resource_limits_config_object);
      id_type id;

      static_assert( config::block_interval_ms > 0, "config::block_interval_ms must be positive" );
      static_assert( config::block_cpu_usage_average_window_ms >= config::block_interval_ms,
                     "config::block_cpu_usage_average_window_ms cannot be less than config::block_interval_ms" );
      static_assert( config::block_size_average_window_ms >= config::block_interval_ms,
                     "config::block_size_average_window_ms cannot be less than config::block_interval_ms" );


      elastic_limit_parameters cpu_limit_parameters = {EOS_PERCENT(config::default_max_block_cpu_usage, config::default_target_block_cpu_usage_pct), config::default_max_block_cpu_usage, config::block_cpu_usage_average_window_ms / config::block_interval_ms, 1000, {99, 100}, {1000, 999}};
      elastic_limit_parameters net_limit_parameters = {EOS_PERCENT(config::default_max_block_net_usage, config::default_target_block_net_usage_pct), config::default_max_block_net_usage, config::block_size_average_window_ms / config::block_interval_ms, 1000, {99, 100}, {1000, 999}};

      uint32_t account_cpu_usage_average_window = config::account_cpu_usage_average_window_ms / config::block_interval_ms;
      uint32_t account_net_usage_average_window = config::account_net_usage_average_window_ms / config::block_interval_ms;
   };
   ```
   对应的索引：
   ```
   using resource_limits_config_index = chainbase::shared_multi_index_container<
      resource_limits_config_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_limits_config_object, resource_limits_config_object::id_type, &resource_limits_config_object::id>>
      >
   >;
   ```
  24. resource_limits_state_object  
   ```
   class resource_limits_state_object : public chainbase::object<resource_limits_state_object_type, resource_limits_state_object> {
      OBJECT_CTOR(resource_limits_state_object);
      id_type id;

      /**
       * Track the average netusage for blocks
       */
      usage_accumulator average_block_net_usage;

      /**
       * Track the average cpu usage for blocks
       */
      usage_accumulator average_block_cpu_usage;

      void update_virtual_net_limit( const resource_limits_config_object& cfg );
      void update_virtual_cpu_limit( const resource_limits_config_object& cfg );

      uint64_t pending_net_usage = 0ULL;
      uint64_t pending_cpu_usage = 0ULL;

      uint64_t total_net_weight = 0ULL;
      uint64_t total_cpu_weight = 0ULL;
      uint64_t total_ram_bytes = 0ULL;

      /**
       * The virtual number of bytes that would be consumed over blocksize_average_window_ms
       * if all blocks were at their maximum virtual size. This is virtual because the
       * real maximum block is less, this virtual number is only used for rate limiting users.
       *
       * It's lowest possible value is max_block_size * blocksize_average_window_ms / block_interval
       * It's highest possible value is 1000 times its lowest possible value
       *
       * This means that the most an account can consume during idle periods is 1000x the bandwidth
       * it is gauranteed under congestion.
       *
       * Increases when average_block_size < target_block_size, decreases when
       * average_block_size > target_block_size, with a cap at 1000x max_block_size
       * and a floor at max_block_size;
       **/
      uint64_t virtual_net_limit = 0ULL;

      /**
       *  Increases when average_bloc
       */
      uint64_t virtual_cpu_limit = 0ULL;

   };
   ```
   对应的索引：
   ```
   using resource_limits_state_index = chainbase::shared_multi_index_container<
      resource_limits_state_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_limits_state_object, resource_limits_state_object::id_type, &resource_limits_state_object::id>>
      >
   >;
   ```
  25. account_history_object
  26. action_history_object
  27. reversible_block_object  
   记录还没变成不可逆的区块，结构如下：
   ```
   class reversible_block_object : public chainbase::object<reversible_block_object_type, reversible_block_object> {
      OBJECT_CTOR(reversible_block_object,(packedblock) )

      id_type        id;
      uint32_t       blocknum = 0;
      shared_string  packedblock;

      void set_block( const signed_block_ptr& b ) {
         packedblock.resize( fc::raw::pack_size( *b ) );
         fc::datastream<char*> ds( packedblock.data(), packedblock.size() );
         fc::raw::pack( ds, *b );
      }

      signed_block_ptr get_block()const {
         fc::datastream<const char*> ds( packedblock.data(), packedblock.size() );
         auto result = std::make_shared<signed_block>();
         fc::raw::unpack( ds, *result );
         return result;
      }
   };

   ```
   对应的索引为：
   ```
   struct by_num;
   using reversible_block_index = chainbase::shared_multi_index_container<
      reversible_block_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<reversible_block_object, reversible_block_object::id_type, &reversible_block_object::id>>,
         ordered_unique<tag<by_num>, member<reversible_block_object, uint32_t, &reversible_block_object::blocknum>>
      >
   >;
   ```

   以上数据表的初始化工作在controller_impl::add_indices()函数中：
   ```
      reversible_blocks.add_index<reversible_block_index>();

      db.add_index<account_index>();
      db.add_index<account_sequence_index>();

      db.add_index<table_id_multi_index>();
      db.add_index<key_value_index>();
      db.add_index<index64_index>();
      db.add_index<index128_index>();
      db.add_index<index256_index>();
      db.add_index<index_double_index>();
      db.add_index<index_long_double_index>();

      db.add_index<global_property_multi_index>();
      db.add_index<dynamic_global_property_multi_index>();
      db.add_index<block_summary_multi_index>();
      db.add_index<transaction_multi_index>();
      db.add_index<generated_transaction_multi_index>();

      authorization.add_indices();
      resource_limits.add_indices();
   ```
   在authorization_manager::add_indices():
   ```
      _db.add_index<permission_index>();
      _db.add_index<permission_usage_index>();
      _db.add_index<permission_link_index>();
   ```
   在resource_limits_manager::add_indices():
   ```
    _db.add_index<resource_limits_index>();
    _db.add_index<resource_usage_index>();
    _db.add_index<resource_limits_state_index>();
    _db.add_index<resource_limits_config_index>();
   ```
   在transaction执行过程中涉及到的数据及流程如下：
   调用controller_impl::push_transaction,在该函数中会生成一个transaction_context类型的变量trx_context, 然后对transaction进行初始化操作：
   ```
    transaction_context trx_context(self, trx->trx, trx->id);
         if ((bool)subjective_cpu_leeway && pending->_block_status == controller::block_status::incomplete) {
            trx_context.leeway = *subjective_cpu_leeway;
         }
         trx_context.deadline = deadline;
         trx_context.explicit_billed_cpu_time = explicit_billed_cpu_time;
         trx_context.billed_cpu_time_us = billed_cpu_time_us;
         trace = trx_context.trace;
         try {
            if( trx->implicit ) {
               trx_context.init_for_implicit_trx();
               trx_context.can_subjectively_fail = false;
            } else {
               bool skip_recording = replay_head_time && (time_point(trx->trx.expiration) <= *replay_head_time);
               trx_context.init_for_input_trx( trx->packed_trx.get_unprunable_size(),
                                               trx->packed_trx.get_prunable_size(),
                                               trx->trx.signatures.size(),
                                               skip_recording);
            }

            if( trx_context.can_subjectively_fail && pending->_block_status == controller::block_status::incomplete ) {
               check_actor_list( trx_context.bill_to_accounts ); // Assumes bill_to_accounts is the set of actors authorizing the transaction
            }


            trx_context.delay = fc::seconds(trx->trx.delay_sec);

   ```
   然后对transaction进行授权检查：
   ```
   if( !self.skip_auth_check() && !trx->implicit ) {
               authorization.check_authorization(
                       trx->trx.actions,
                       trx->recover_keys( chain_id ),
                       {},
                       trx_context.delay,
                       [](){}
                       /*std::bind(&transaction_context::add_cpu_usage_and_check_time, &trx_context,
                                 std::placeholders::_1)*/,
                       false
               );
            }
    trx_context.exec();
    trx_context.finalize(); /
   ```
  在此需要用到上面的global_property_object数据表，然后调用transaction_context::exec()对action进行调用：
  ```
    EOS_ASSERT( is_initialized, transaction_exception, "must first initialize" );

      if( apply_context_free ) {
         for( const auto& act : trx.context_free_actions ) {
            trace->action_traces.emplace_back();
            dispatch_action( trace->action_traces.back(), act, true );//action调用
         }
      }

      if( delay == fc::microseconds() ) {
         for( const auto& act : trx.actions ) {
            trace->action_traces.emplace_back();
            dispatch_action( trace->action_traces.back(), act ); //action调用
         }
      } else {
         schedule_transaction();
      }
  ```
  在transaction_context::dispatch_action中会产生一个类型为apply_context的变量 acontext,调用apply_context::exec()进行真正的action的执行
  ```
      apply_context  acontext( control, *this, a, recurse_depth );
      acontext.context_free = context_free;
      acontext.receiver     = receiver;

      try {
         acontext.exec();
      } catch( ... ) {
         trace = move(acontext.trace);
         throw;
      }

      trace = move(acontext.trace);
  ```
  在apply_context::exec()中会调用apply_context::exec_one() 调用vm借口进入合约层，进行action和数据的解析并执行。
  vm会通过注册进入的借口来调用action执行，注册的借口为：
  ```
    REGISTER_INTRINSICS(transaction_api,
    (send_inline,               void(int, int)               )
    (send_context_free_inline,  void(int, int)               )
    (send_deferred,             void(int, int64_t, int, int, int32_t) )
    (cancel_deferred,           int(int)                     )
    );
  ```
  transaction_api接口定义如下：
  ```
  class transaction_api : public context_aware_api {
   public:
      using context_aware_api::context_aware_api;

      void send_inline( array_ptr<char> data, size_t data_len ) {
         //TODO: Why is this limit even needed? And why is it not consistently checked on actions in input or deferred transactions
         EOS_ASSERT( data_len < context.control.get_global_properties().configuration.max_inline_action_size, inline_action_too_big,
                    "inline action too big" );

         action act;
         fc::raw::unpack<action>(data, data_len, act);
         context.execute_inline(std::move(act));
      }

      void send_context_free_inline( array_ptr<char> data, size_t data_len ) {
         //TODO: Why is this limit even needed? And why is it not consistently checked on actions in input or deferred transactions
         EOS_ASSERT( data_len < context.control.get_global_properties().configuration.max_inline_action_size, inline_action_too_big,
                   "inline action too big" );

         action act;
         fc::raw::unpack<action>(data, data_len, act);
         context.execute_context_free_inline(std::move(act));
      }

      void send_deferred( const uint128_t& sender_id, account_name payer, array_ptr<char> data, size_t data_len, uint32_t replace_existing) {
         try {
            transaction trx;
            fc::raw::unpack<transaction>(data, data_len, trx);
            context.schedule_deferred_transaction(sender_id, payer, std::move(trx), replace_existing);
         } FC_RETHROW_EXCEPTIONS(warn, "data as hex: ${data}", ("data", fc::to_hex(data, data_len)))
      }

      bool cancel_deferred( const unsigned __int128& val ) {
         fc::uint128_t sender_id(val>>64, uint64_t(val) );
         return context.cancel_deferred_transaction( (unsigned __int128)sender_id );
      }
  };
  ```
  以上为系统数据表和database交互的模式。  
  
  ## 智能合约的持久化存储和database交互
   说到智能合约的持久化存储离不开Multi-Index,这个Multi-index是EOS实现的类boost::multi_index_container的功能，定义在： contracts/eosiolib/multi_index.hpp文件中，采用的是hana元编程，我们写的智能合约中的数据就是存储在这个multi_index中的。
  该类实现了数据的增删改查接口：emplace,erase,modify,get,find等接口，通过这些接口和database进行交互。  
  1. emplace中和database交互的关键代码：
      ```
        datastream<char*> ds( (char*)buffer, size );
            ds << obj;

            auto pk = obj.primary_key();

            //db_store_i64就是和database进行交互的接口
            i.__primary_itr = db_store_i64( _scope, TableName, payer, pk, buffer, size );

            if ( max_stack_buffer_size < size ) {
               free(buffer);
            }
      ```
  2. erase中和database交互的关键代码:
      ```
        eosio_assert( itr2 != _items_vector.rend(), "attempt to remove object that was not in multi_index" );

         _items_vector.erase(--(itr2.base()));
         
         //和database进行交互
         db_remove_i64( objitem.__primary_itr );
      ```
  其他接口和数据库交互请参看源码。   
multi_index的使用：  
```
  class book_manager : public eosio::contract {
  public:
    void create()
    void delete()
    void find()
  private:
    account_name   _contract_name;
    struct book {
      uint64_t            _id;
      std::string         _name;
      EOSLIB_SERIALIZE(book,(_id)(_name));
    }
    typedef eosio::multi_index<N(book),book> _table;

  }
```
大概类似于上面的代码，后面我会出一个详细智能合约开发的例子。EOSLIB_SERIALIZE宏用于序列化book接口，将其转为字节数组。后面我就可以基于_table对book进行管理了，增删改查也会与database进行交互，现在来看一下database提供的api接口，这些接口定义在 libraries/chain/wasm_interface.cpp中：
```
class database_api : public context_aware_api {
   public:
      using context_aware_api::context_aware_api;

      int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, array_ptr<const char> buffer, size_t buffer_size ) {
         return context.db_store_i64( scope, table, payer, id, buffer, buffer_size );
      }
      void db_update_i64( int itr, uint64_t payer, array_ptr<const char> buffer, size_t buffer_size ) {
         context.db_update_i64( itr, payer, buffer, buffer_size );
      }
      void db_remove_i64( int itr ) {
         context.db_remove_i64( itr );
      }
      int db_get_i64( int itr, array_ptr<char> buffer, size_t buffer_size ) {
         return context.db_get_i64( itr, buffer, buffer_size );
      }
      int db_next_i64( int itr, uint64_t& primary ) {
         return context.db_next_i64(itr, primary);
      }
      int db_previous_i64( int itr, uint64_t& primary ) {
         return context.db_previous_i64(itr, primary);
      }
      int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
         return context.db_find_i64( code, scope, table, id );
      }
      int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
         return context.db_lowerbound_i64( code, scope, table, id );
      }
      int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
         return context.db_upperbound_i64( code, scope, table, id );
      }
      int db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
         return context.db_end_i64( code, scope, table );
      }

      DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx64,  uint64_t)
      DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx128, uint128_t)
      DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(idx256, 2, uint128_t)
      DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY(idx_double, float64_t)
      DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY(idx_long_double, float128_t)
  } ;
```
由上可见database_api调用的是apply_context提供的接口，而appy_context中有database的引用，最终所有的操作都会反映到database中去