//
// Created by 杨文宇 on 2018/11/19.
//
#pragma once

#include <chainbase/chainbase.hpp>
#include <eosio/chain/account_object.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/transaction_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/permission_link_object.hpp>
#include <eosio/chain/resource_limits_private.hpp>
#include <eosio/chain/reversible_block_object.hpp>
#include <eosio/chain/fork_database.hpp>

using namespace chainbase;
using namespace eosio::chain;
class database_controller {
public:

    database_controller(const database_controller&) = delete;
    database_controller& operator=(const database_controller&) = delete;
    ~database_controller();


    static database_controller& get() {
        static database_controller ctrl;
        return ctrl;
    }

    void initialize_database();
    database& get_state_db() {
        return _state_db;
    }
    database& get_reversible_db() {
        return _reversible_db;
    }
    fork_database& get_block_db() {
        return _block_db;
    }

private:
    database_controller();


private:
    database         _state_db;
    database         _reversible_db;
    fork_database    _block_db;
};
