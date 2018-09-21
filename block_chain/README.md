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
  
  1.block_header,定义在：libraries/chain/include/eosio/chain/block_header.hpp第7行

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
  
  2.signed_block_header,定义在：libraries/chain/include/eosio/chain/block_header.hpp第45行

  ``` cpp
  struct signed_block_header : public block_header {
	  signature_type producer_signature; //生产者的签名
  }
  ```

  3.signed_block,定义在：libraries/chain/include/eosio/chain/block.hpp 57行

  ``` cpp
  struct signed_block : public signed_block_header {
	  vector<transacton_receipt> transactions; //区块包含的transactions执行后得到的回执
	  extrension_type            block_extensions;
  }
  ```

  4.transaction_receipt_header,定义在：libraries/chain/include/eosio/chain/block.hpp 12行

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

  5.transaction_receipt,定义在：libraries/chain/include/eosio/chain/block.hpp 33行

  ``` cpp
  struct transaction_receipt : public transaction_receipt_header {
    fc::static_variant<transaction_id_type,packed_transaction> trx; //已经执行过的transactions
  }
  ```

  6.transaction_header,定义在：libraries/chain/include/eosio/chain/transaction.hpp 30行

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

  7.transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 54行

  ``` cpp
  struct transaction : public transaction_header {
    vector<action>                 context_free_actions; //上下文无关的actions
    vector<action>                 actions;
    extension_type                 transaction_extensions;
  }
  ```

  8.signed_transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 78行

  ``` cpp
  struct signed_transaction : public transaction {
    vector<signature>         signatures;
    vector<bytes>             context_free_data; //和context_free_action一一对应
  }
  ```

  9.packed_transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 98行

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

  10.deferred_transaction,定义在：libraries/chain/include/eosio/chain/transaction.hpp 157行
  
  ``` cpp
  struct deferred_transaction : public signed_transaction {
    uint128_t                  sender_id;
    account_name               sender;
    account_name               payer;
    time_point_sec             execute_after;
  }
  ```

### producer_plugin

### controller

## chainbase分析

### database基本数据结构
