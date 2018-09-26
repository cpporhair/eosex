# Table of Content

* [总览](##总览)
* [环境](##环境)
* [预备知识](##预备知识)
* [chain分析](##chain分析)
  * [基本数据结构](###blockchain基本数据结构)
  * [producer_plugin](###producer_plugin)
  * [controller](###controller)
* [chainbase分析](##chainbase分析)
  * [基本数据结构](###database基本数据结构)

## 总览
  
  本文主要致力于EOS区块结构、生产、打包、验证、存储等流程分析，当然其他部分源码也有涉及，但在此不做详细讨论。

## 环境
  
  Mac OS High Sierra 10.13.6  
  EOS源码版本1.2.1  
  CLion  
  CMake

## 预备知识

- 文档  
  
  [C++11新特性学习](C++新特性学习.md)

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
   * 调用controller::abort_block,controller::start_block者两个函数在后文详述。
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
   chain.finalize_block();
   chain.sign_block( [&]( const digest_type& d ) {
      auto debug_logger = maybe_make_debug_time_logger();
      return signature_provider_itr->second(d);
   } );

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

   至此producer_plugin中区块的生产流程已经介绍完毕，更详细的分析会在controller中体现出来。  


   区块同步流程：




### controller
    producer_plugin在区块生产的过程中扮演着调度的角色，而实际工作是放在controller中来完成的，下面将纤细分析controller在区块生成过程中所扮演的角色功能：  
    

## chainbase分析

### database基本数据结构
