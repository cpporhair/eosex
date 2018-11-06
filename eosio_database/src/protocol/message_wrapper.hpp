//
// Created by 杨文宇 on 2018/11/4.
//
#pragma once

#include <string>
#include <memory>
#include <exception>
#include <protocol/message.pb.h>


class request_message {
public:
    request_message(action_message::message_id id,action_message::object_type type):_req{} {
        _req.set_type(type);
        _req.set_msg_id(id);
    }
    ~request_message(){}

    void set_data(const char *buf,const std::size_t len) {
        _req.set_data(buf,len);
    }
    void set_data(const char *buf) {
        _req.set_data(buf);
    }
    void set_data(const std::string &data) {
        _req.set_data(data);
    }

    std::size_t get_data(char *buf,std::size_t len) {
        if(sizeof(int32_t) + _req.ByteSize() > max_message_size)
            throw std::runtime_error("message too large!");

        if(sizeof(int32_t) + _req.ByteSize() > len)
            throw std::runtime_error("buf len is not enough");

        int32_t size = _req.ByteSize();
        std::memcpy(buf,&size,sizeof(int32_t));
        _req.SerializeToArray(buf + sizeof(int32_t),len - sizeof(int32_t));

        return sizeof(int32_t) + _req.ByteSize();
    }

private:
    enum {
        max_message_size = 1024 * 1024 * 2
    };
    action_message::rpc_request _req;
};
typedef std::shared_ptr<request_message> request_message_shared_ptr;

class response_message {
public:
    response_message(action_message::message_id id,action_message::object_type type):_resp{} {
        _resp.set_msg_id(id);
        _resp.set_type(type);
    }
    ~response_message(){}

    void set_data(const char *buf,const std::size_t len) {
        _resp.set_data(buf,len);
    }
    void set_data(const char *buf) {
        _resp.set_data(buf);
    }
    void set_data(const std::string &data) {
        _resp.set_data(data);
    }

    std::size_t get_data(char *buf,std::size_t len) {
        if(sizeof(int32_t) + _resp.ByteSize() > max_message_size)
            throw std::runtime_error("message too large!");

        if(sizeof(int32_t) + _resp.ByteSize() > len)
            throw std::runtime_error("buf len is not enough");

        int32_t size = _resp.ByteSize();
        std::memcpy(buf,&size,sizeof(int32_t));
        _resp.SerializeToArray(buf + sizeof(int32_t),len - sizeof(int32_t));

        return sizeof(int32_t) + _resp.ByteSize();
    }

private:
    enum {
        max_message_size = 1024 * 1024 * 2
    };
    action_message::rpc_response _resp;
};
typedef std::shared_ptr<response_message> response_message_shared_ptr;

typedef std::shared_ptr<action_message::rpc_request> raw_request_shared_ptr;
typedef std::shared_ptr<action_message::rpc_response> raw_response_shared_ptr;

raw_request_shared_ptr get_raw_account_create_msg();
raw_request_shared_ptr get_raw_account_update_msg();
raw_request_shared_ptr get_raw_account_query_msg();
raw_request_shared_ptr get_raw_account_remove_msg();

raw_request_shared_ptr get_raw_account_sequence_create_msg();
raw_request_shared_ptr get_raw_account_sequence_query_msg();
raw_request_shared_ptr get_raw_account_sequence_update_msg();
raw_request_shared_ptr get_raw_account_sequence_remove_msg();

raw_request_shared_ptr get_raw_permission_create_msg();
raw_request_shared_ptr get_raw_permission_query_msg();
raw_request_shared_ptr get_raw_permission_update_msg();
raw_request_shared_ptr get_raw_permission_remove_msg();

raw_request_shared_ptr get_raw_permission_usage_create_msg();
raw_request_shared_ptr get_raw_permission_usage_query_msg();
raw_request_shared_ptr get_raw_permission_usage_update_msg();
raw_request_shared_ptr get_raw_permission_usage_remove_msg();

raw_request_shared_ptr get_raw_permission_link_create_msg();
raw_request_shared_ptr get_raw_permission_link_query_msg();
raw_request_shared_ptr get_raw_permission_link_update_msg();
raw_request_shared_ptr get_raw_permission_link_remove_msg();

raw_request_shared_ptr get_raw_key_value_create_msg();
raw_request_shared_ptr get_raw_key_value_update_msg();
raw_request_shared_ptr get_raw_key_value_query_msg();
raw_request_shared_ptr get_raw_key_value_remove_msg();

raw_request_shared_ptr get_raw_index64_create_msg();
raw_request_shared_ptr get_raw_index64_query_msg();
raw_request_shared_ptr get_raw_index64_update_msg();
raw_request_shared_ptr get_raw_index64_remove_msg();

raw_request_shared_ptr get_raw_index128_create_msg();
raw_request_shared_ptr get_raw_index128_query_msg();
raw_request_shared_ptr get_raw_index128_update_msg();
raw_request_shared_ptr get_raw_index128_remove_msg();

raw_request_shared_ptr get_raw_index256_create_msg();
raw_request_shared_ptr get_raw_index256_query_msg();
raw_request_shared_ptr get_raw_index256_update_msg();
raw_request_shared_ptr get_raw_index256_remove_msg();

raw_request_shared_ptr get_raw_index_double_create_msg();
raw_request_shared_ptr get_raw_index_double_query_msg();
raw_request_shared_ptr get_raw_index_double_update_msg();
raw_request_shared_ptr get_raw_index_double_remove_msg();

raw_request_shared_ptr get_raw_index_long_double_create_msg();
raw_request_shared_ptr get_raw_index_long_double_query_msg();
raw_request_shared_ptr get_raw_index_long_double_update_msg();
raw_request_shared_ptr get_raw_index_long_double_remove_msg();

raw_request_shared_ptr get_raw_global_property_create_msg();
raw_request_shared_ptr get_raw_global_property_query_msg();
raw_request_shared_ptr get_raw_global_property_update_msg();
raw_request_shared_ptr get_raw_global_property_remove_msg();

raw_request_shared_ptr get_raw_dynamic_property_create_msg();
raw_request_shared_ptr get_raw_dynamic_property_query_msg();
raw_request_shared_ptr get_raw_dynamic_property_update_msg();
raw_request_shared_ptr get_raw_dynamic_property_remove_msg();

raw_request_shared_ptr get_raw_block_summary_create_msg();
raw_request_shared_ptr get_raw_block_summary_query_msg();
raw_request_shared_ptr get_raw_block_summary_update_msg();
raw_request_shared_ptr get_raw_block_summary_remove_msg();

raw_request_shared_ptr get_raw_transaction_create_msg();
raw_request_shared_ptr get_raw_transaction_query_msg();
raw_request_shared_ptr get_raw_transaction_update_msg();
raw_request_shared_ptr get_raw_transaction_remove_msg();

raw_request_shared_ptr get_raw_generated_transaction_create_msg();
raw_request_shared_ptr get_raw_generated_transaction_query_msg();
raw_request_shared_ptr get_raw_generated_transaction_update_msg();
raw_request_shared_ptr get_raw_generated_transaction_remove_msg();

raw_request_shared_ptr get_raw_producer_create_msg();
raw_request_shared_ptr get_raw_producer_query_msg();
raw_request_shared_ptr get_raw_producer_update_msg();
raw_request_shared_ptr get_raw_producer_remove_msg();

raw_request_shared_ptr get_raw_account_control_history_create_msg();
raw_request_shared_ptr get_raw_account_control_history_query_msg();
raw_request_shared_ptr get_raw_account_control_history_update_msg();
raw_request_shared_ptr get_raw_account_control_history_remove_msg();

raw_request_shared_ptr get_raw_public_key_history_create_msg();
raw_request_shared_ptr get_raw_public_key_history_query_msg();
raw_request_shared_ptr get_raw_public_key_history_update_msg();
raw_request_shared_ptr get_raw_public_key_history_remove_msg();

raw_request_shared_ptr get_raw_table_id_create_msg();
raw_request_shared_ptr get_raw_table_id_query_msg();
raw_request_shared_ptr get_raw_table_id_update_msg();
raw_request_shared_ptr get_raw_table_id_remove_msg();

raw_request_shared_ptr get_raw_resource_limits_create_msg();
raw_request_shared_ptr get_raw_resource_limits_query_msg();
raw_request_shared_ptr get_raw_resource_limits_update_msg();
raw_request_shared_ptr get_raw_resource_limits_remove_msg();

raw_request_shared_ptr get_raw_resource_usage_create_msg();
raw_request_shared_ptr get_raw_resource_usage_query_msg();
raw_request_shared_ptr get_raw_resource_usage_update_msg();
raw_request_shared_ptr get_raw_resource_usage_remove_msg();

raw_request_shared_ptr get_raw_resource_limits_state_create_msg();
raw_request_shared_ptr get_raw_resource_limits_state_query_msg();
raw_request_shared_ptr get_raw_resource_limits_state_update_msg();
raw_request_shared_ptr get_raw_resource_limits_state_remove_msg();

raw_request_shared_ptr get_raw_resource_limits_config_create_msg();
raw_request_shared_ptr get_raw_resource_limits_config_query_msg();
raw_request_shared_ptr get_raw_resource_limits_config_update_msg();
raw_request_shared_ptr get_raw_resource_limits_config_remove_msg();

raw_request_shared_ptr get_raw_account_history_create_msg();
raw_request_shared_ptr get_raw_account_history_query_msg();
raw_request_shared_ptr get_raw_account_history_update_msg();
raw_request_shared_ptr get_raw_account_history_remove_msg();

raw_request_shared_ptr get_raw_action_history_create_msg();
raw_request_shared_ptr get_raw_action_history_query_msg();
raw_request_shared_ptr get_raw_action_history_update_msg();
raw_request_shared_ptr get_raw_action_history_remove_msg();

raw_request_shared_ptr get_raw_reversible_block_create_msg();
raw_request_shared_ptr get_raw_reversible_block_query_msg();
raw_request_shared_ptr get_raw_reversible_block_update_msg();
raw_request_shared_ptr get_raw_reversible_block_remove_msg();

request_message_shared_ptr get_account_create_msg();
request_message_shared_ptr get_account_update_msg();
request_message_shared_ptr get_account_query_msg();
request_message_shared_ptr get_account_remove_msg();

request_message_shared_ptr get_account_sequence_create_msg();
request_message_shared_ptr get_account_sequence_query_msg();
request_message_shared_ptr get_account_sequence_update_msg();
request_message_shared_ptr get_account_sequence_remove_msg();

request_message_shared_ptr get_permission_create_msg();
request_message_shared_ptr get_permission_query_msg();
request_message_shared_ptr get_permission_update_msg();
request_message_shared_ptr get_permission_remove_msg();

request_message_shared_ptr get_permission_usage_create_msg();
request_message_shared_ptr get_permission_usage_query_msg();
request_message_shared_ptr get_permission_usage_update_msg();
request_message_shared_ptr get_permission_usage_remove_msg();

request_message_shared_ptr get_permission_link_create_msg();
request_message_shared_ptr get_permission_link_query_msg();
request_message_shared_ptr get_permission_link_update_msg();
request_message_shared_ptr get_permission_link_remove_msg();

request_message_shared_ptr get_key_value_create_msg();
request_message_shared_ptr get_key_value_update_msg();
request_message_shared_ptr get_key_value_query_msg();
request_message_shared_ptr get_key_value_remove_msg();

request_message_shared_ptr get_index64_create_msg();
request_message_shared_ptr get_index64_query_msg();
request_message_shared_ptr get_index64_update_msg();
request_message_shared_ptr get_index64_remove_msg();

request_message_shared_ptr get_index128_create_msg();
request_message_shared_ptr get_index128_query_msg();
request_message_shared_ptr get_index128_update_msg();
request_message_shared_ptr get_index128_remove_msg();

request_message_shared_ptr get_index256_create_msg();
request_message_shared_ptr get_index256_query_msg();
request_message_shared_ptr get_index256_update_msg();
request_message_shared_ptr get_index256_remove_msg();

request_message_shared_ptr get_index_double_create_msg();
request_message_shared_ptr get_index_double_query_msg();
request_message_shared_ptr get_index_double_update_msg();
request_message_shared_ptr get_index_double_remove_msg();

request_message_shared_ptr get_index_long_double_create_msg();
request_message_shared_ptr get_index_long_double_query_msg();
request_message_shared_ptr get_index_long_double_update_msg();
request_message_shared_ptr get_index_long_double_remove_msg();

request_message_shared_ptr get_global_property_create_msg();
request_message_shared_ptr get_global_property_query_msg();
request_message_shared_ptr get_global_property_update_msg();
request_message_shared_ptr get_global_property_remove_msg();

request_message_shared_ptr get_dynamic_property_create_msg();
request_message_shared_ptr get_dynamic_property_query_msg();
request_message_shared_ptr get_dynamic_property_update_msg();
request_message_shared_ptr get_dynamic_property_remove_msg();

request_message_shared_ptr get_block_summary_create_msg();
request_message_shared_ptr get_block_summary_query_msg();
request_message_shared_ptr get_block_summary_update_msg();
request_message_shared_ptr get_block_summary_remove_msg();

request_message_shared_ptr get_transaction_create_msg();
request_message_shared_ptr get_transaction_query_msg();
request_message_shared_ptr get_transaction_update_msg();
request_message_shared_ptr get_transaction_remove_msg();

request_message_shared_ptr get_generated_transaction_create_msg();
request_message_shared_ptr get_generated_transaction_query_msg();
request_message_shared_ptr get_generated_transaction_update_msg();
request_message_shared_ptr get_generated_transaction_remove_msg();

request_message_shared_ptr get_producer_create_msg();
request_message_shared_ptr get_producer_query_msg();
request_message_shared_ptr get_producer_update_msg();
request_message_shared_ptr get_producer_remove_msg();

request_message_shared_ptr get_account_control_history_create_msg();
request_message_shared_ptr get_account_control_history_query_msg();
request_message_shared_ptr get_account_control_history_update_msg();
request_message_shared_ptr get_account_control_history_remove_msg();

request_message_shared_ptr get_public_key_history_create_msg();
request_message_shared_ptr get_public_key_history_query_msg();
request_message_shared_ptr get_public_key_history_update_msg();
request_message_shared_ptr get_public_key_history_remove_msg();

request_message_shared_ptr get_table_id_create_msg();
request_message_shared_ptr get_table_id_query_msg();
request_message_shared_ptr get_table_id_update_msg();
request_message_shared_ptr get_table_id_remove_msg();

request_message_shared_ptr get_resource_limits_create_msg();
request_message_shared_ptr get_resource_limits_query_msg();
request_message_shared_ptr get_resource_limits_update_msg();
request_message_shared_ptr get_resource_limits_remove_msg();

request_message_shared_ptr get_resource_usage_create_msg();
request_message_shared_ptr get_resource_usage_query_msg();
request_message_shared_ptr get_resource_usage_update_msg();
request_message_shared_ptr get_resource_usage_remove_msg();

request_message_shared_ptr get_resource_limits_state_create_msg();
request_message_shared_ptr get_resource_limits_state_query_msg();
request_message_shared_ptr get_resource_limits_state_update_msg();
request_message_shared_ptr get_resource_limits_state_remove_msg();

request_message_shared_ptr get_resource_limits_config_create_msg();
request_message_shared_ptr get_resource_limits_config_query_msg();
request_message_shared_ptr get_resource_limits_config_update_msg();
request_message_shared_ptr get_resource_limits_config_remove_msg();

request_message_shared_ptr get_account_history_create_msg();
request_message_shared_ptr get_account_history_query_msg();
request_message_shared_ptr get_account_history_update_msg();
request_message_shared_ptr get_account_history_remove_msg();

request_message_shared_ptr get_action_history_create_msg();
request_message_shared_ptr get_action_history_query_msg();
request_message_shared_ptr get_action_history_update_msg();
request_message_shared_ptr get_action_history_remove_msg();

request_message_shared_ptr get_reversible_block_create_msg();
request_message_shared_ptr get_reversible_block_query_msg();
request_message_shared_ptr get_reversible_block_update_msg();
request_message_shared_ptr get_reversible_block_remove_msg();