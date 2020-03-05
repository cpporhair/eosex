//
// Created by 杨文宇 on 2018/11/20.
//
#include <database_controller/database_controller.hpp>
#include <eosio/chain/config.hpp>

using namespace eosio::chain;
using namespace eosio::chain::resource_limits;

database_controller::database_controller():
    _state_db(eosio::chain::config::default_state_dir_name,chainbase::database::read_write,eosio::chain::config::default_state_size),
    _reversible_db(eosio::chain::config::reversible_blocks_dir_name,chainbase::database::read_write,eosio::chain::config::default_reversible_cache_size),
    _block_db(eosio::chain::config::default_state_dir_name){}


database_controller::~database_controller() {
}

void database_controller::initialize_database() {

    _state_db.add_index<account_index>();
    _state_db.add_index<account_sequence_index>();

    _state_db.add_index<table_id_multi_index>();
    _state_db.add_index<key_value_index>();
    _state_db.add_index<index64_index>();
    _state_db.add_index<index128_index>();
    _state_db.add_index<index256_index>();
    _state_db.add_index<index_double_index>();
    _state_db.add_index<index_long_double_index>();

    _state_db.add_index<global_property_multi_index>();
    _state_db.add_index<dynamic_global_property_multi_index>();
    _state_db.add_index<block_summary_multi_index>();
    _state_db.add_index<transaction_multi_index>();
    _state_db.add_index<generated_transaction_multi_index>();

    _state_db.add_index<resource_limits_index>();
    _state_db.add_index<resource_usage_index>();
    _state_db.add_index<resource_limits_state_index>();
    _state_db.add_index<resource_limits_config_index>();
    _state_db.add_index<permission_index>();
    _state_db.add_index<permission_usage_index>();
    _state_db.add_index<permission_link_index>();

    _reversible_db.add_index<reversible_block_index>();
}
