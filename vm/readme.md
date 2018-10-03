# Table of Content

* [总览](##总览)
* [Web Assembly介绍](##Web Assembly介绍)
* [Web Assembly介绍](##EOS虚拟机架构介绍)
* [WABT虚拟机](##chainbase分析)
* [API注册](##chainbase分析)
* [初始化虚拟机](##chainbase分析)
* [注入opcode](##chainbase分析)
* [执行智能合约](##chainbase分析)
* [并发,可扩展的智能合约解释器](##chainbase分析)

##总览
  
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


