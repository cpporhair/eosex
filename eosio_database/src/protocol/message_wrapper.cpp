//
// Created by 杨文宇 on 2018/11/4.
//
#include <protocol/message_wrapper.hpp>

inline static raw_request_shared_ptr create(action_message::message_id id,action_message::object_type t) {
    raw_request_shared_ptr req{new action_message::rpc_request};
    req->set_msg_id(id);
    req->set_type(t);
    return req;
}

static raw_request_shared_ptr create_request_msg(action_message::message_id id) {
    switch (id){
        case action_message::message_id::account_create_request:
            return create(id,action_message::object_type::account_object_type);
        //case action_message::message_id::account_create_response:
        case action_message::message_id::account_query_request:
            return create(id,action_message::object_type::account_object_type);
       // case action_message::message_id::account_query_response:
        case action_message::message_id::account_update_request:
            return create(id,action_message::object_type::account_object_type);
       // case action_message::message_id::account_update_response:
        case action_message::message_id::account_remove_request:
            return create(id,action_message::object_type::account_object_type);
       // case action_message::message_id::account_remove_response:
        case action_message::message_id::account_sequence_create_request:
            return create(id,action_message::object_type::account_sequence_object_type);
        //case action_message::message_id::account_sequence_create_response:
        case action_message::message_id::account_sequence_query_request:
            return create(id,action_message::object_type::account_sequence_object_type);
        //case action_message::message_id::account_sequence_query_response:
        case action_message::message_id::account_sequence_update_request:
            return create(id,action_message::object_type::account_sequence_object_type);
        //case action_message::message_id::account_sequence_update_response:
        case action_message::message_id::account_sequence_remove_request:
            return create(id,action_message::object_type::account_sequence_object_type);
        //case action_message::message_id::account_sequence_remove_response:
        case action_message::message_id::permission_create_request:
            return create(id,action_message::object_type::permission_object_type);
        //case action_message::message_id::permission_create_response:
        case action_message::message_id::permission_query_request:
            return create(id,action_message::object_type::permission_object_type);
        //case action_message::message_id::permission_query_response:
        case action_message::message_id::permission_update_request:
            return create(id,action_message::object_type::permission_object_type);
        //case action_message::message_id::permission_update_response:
        case action_message::message_id::permission_remove_request:
            return create(id,action_message::object_type::permission_object_type);
        //case action_message::message_id::permission_remove_response:
        case action_message::message_id::permission_usage_create_request:
            return create(id,action_message::object_type::permission_usage_object_type);
        //case action_message::message_id::permission_usage_create_response:
        case action_message::message_id::permission_usage_query_request:
            return create(id,action_message::object_type::permission_usage_object_type);
        //case action_message::message_id::permission_usage_query_response:
        case action_message::message_id::permission_usage_update_request:
            return create(id,action_message::object_type::permission_usage_object_type);
        //case action_message::message_id::permission_usage_update_response:
        case action_message::message_id::permission_usage_remove_request:
            return create(id,action_message::object_type::permission_usage_object_type);
        //case action_message::message_id::permission_usage_remove_response:
        case action_message::message_id::permission_link_create_request:
            return create(id,action_message::object_type::permission_link_object_type);
        //case action_message::message_id::permission_link_create_response:
        case action_message::message_id::permission_link_query_request:
            return create(id,action_message::object_type::permission_link_object_type);
        //case action_message::message_id::permission_link_query_response:
        case action_message::message_id::permission_link_update_request:
            return create(id,action_message::object_type::permission_link_object_type);
        //case action_message::message_id::permission_link_update_response:
        case action_message::message_id::permission_link_remove_request:
            return create(id,action_message::object_type::permission_link_object_type);
       // case action_message::message_id::permission_link_remove_response:
        case action_message::message_id::key_value_create_request:
            return create(id,action_message::object_type::key_value_object_type);
        //case action_message::message_id::key_value_create_response:
        case action_message::message_id::key_value_query_request:
            return create(id,action_message::object_type::key_value_object_type);
        //case action_message::message_id::key_value_query_response:
        case action_message::message_id::key_value_update_request:
            return create(id,action_message::object_type::key_value_object_type);
        //case action_message::message_id::key_value_update_response:
        case action_message::message_id::key_value_remove_request:
            return create(id,action_message::object_type::key_value_object_type);
        //case action_message::message_id::key_value_remove_response:
        case action_message::message_id::index64_create_request:
            return create(id,action_message::object_type::index64_object_type);
        //case action_message::message_id::index64_create_response:
        case action_message::message_id::index64_query_request:
            return create(id,action_message::object_type::index64_object_type);
        //case action_message::message_id::index64_query_response:
        case action_message::message_id::index64_update_request:
            return create(id,action_message::object_type::index64_object_type);
        //case action_message::message_id::index64_update_response:
        case action_message::message_id::index64_remove_request:
            return create(id,action_message::object_type::index64_object_type);
        //case action_message::message_id::index64_remove_response:
        case action_message::message_id::index128_create_request:
            return create(id,action_message::object_type::index128_object_type);
        //case action_message::message_id::index128_create_response:
        case action_message::message_id::index128_query_request:
            return create(id,action_message::object_type::index128_object_type);
        //case action_message::message_id::index128_query_response:
        case action_message::message_id::index128_update_request:
            return create(id,action_message::object_type::index128_object_type);
        //case action_message::message_id::index128_update_response:
        case action_message::message_id::index128_remove_request:
            return create(id,action_message::object_type::index128_object_type);
        //case action_message::message_id::index128_remove_response:
        case action_message::message_id::index256_create_request:
            return create(id,action_message::object_type::index256_object_type);
        //case action_message::message_id::index256_create_response:
        case action_message::message_id::index256_query_request:
            return create(id,action_message::object_type::index256_object_type);
        //case action_message::message_id::index256_query_response:
        case action_message::message_id::index256_update_request:
            return create(id,action_message::object_type::index256_object_type);
        //case action_message::message_id::index256_update_response:
        case action_message::message_id::index256_remove_request:
            return create(id,action_message::object_type::index256_object_type);
        //case action_message::message_id::index256_remove_response:
        case action_message::message_id::index_double_create_request:
            return create(id,action_message::object_type::index_double_object_type);
        //case action_message::message_id::index_double_create_response:
        case action_message::message_id::index_double_query_request:
            return create(id,action_message::object_type::index_double_object_type);
        //case action_message::message_id::index_double_query_response:
        case action_message::message_id::index_double_update_request:
            return create(id,action_message::object_type::index_double_object_type);
        //case action_message::message_id::index_double_update_response:
        case action_message::message_id::index_double_remove_request:
            return create(id,action_message::object_type::index_double_object_type);
        //case action_message::message_id::index_double_remove_response:
        case action_message::message_id::index_long_double_create_request:
            return create(id,action_message::object_type::index_long_double_object_type);
        //case action_message::message_id::index_long_double_create_response:
        case action_message::message_id::index_long_double_query_request:
            return create(id,action_message::object_type::index_long_double_object_type);
        //case action_message::message_id::index_long_double_query_response:
        case action_message::message_id::index_long_double_update_request:
            return create(id,action_message::object_type::index_long_double_object_type);
        //case action_message::message_id::index_long_double_update_response:
        case action_message::message_id::index_long_double_remove_request:
            return create(id,action_message::object_type::index_long_double_object_type);
        //case action_message::message_id::index_long_double_remove_response:
        case action_message::message_id::global_property_create_request:
            return create(id,action_message::object_type::global_property_object_type);
        //case action_message::message_id::global_property_create_response:
        case action_message::message_id::global_property_query_request:
            return create(id,action_message::object_type::global_property_object_type);
        //case action_message::message_id::global_property_query_response:
        case action_message::message_id::global_property_update_request:
            return create(id,action_message::object_type::global_property_object_type);
        //case action_message::message_id::global_property_update_response:
        case action_message::message_id::global_property_remove_request:
            return create(id,action_message::object_type::global_property_object_type);
        //case action_message::message_id::global_property_remove_response:
        case action_message::message_id::dynamic_global_property_create_request:
            return create(id,action_message::object_type::dynamic_global_property_object_type);
        //case action_message::message_id::dynamic_global_property_create_response:
        case action_message::message_id::dynamic_global_property_query_request:
            return create(id,action_message::object_type::dynamic_global_property_object_type);
        //case action_message::message_id::dynamic_global_property_query_response:
        case action_message::message_id::dynamic_global_property_update_request:
            return create(id,action_message::object_type::dynamic_global_property_object_type);
        //case action_message::message_id::dynamic_global_property_update_response:
        case action_message::message_id::dynamic_global_property_remove_request:
            return create(id,action_message::object_type::dynamic_global_property_object_type);
        //case action_message::message_id::dynamic_global_property_remove_response:
        case action_message::message_id::block_summary_create_request:
            return create(id,action_message::object_type::block_summary_object_type);
        //case action_message::message_id::block_summary_create_response:
        case action_message::message_id::block_summary_query_request:
            return create(id,action_message::object_type::block_summary_object_type);
        //case action_message::message_id::block_summary_query_response:
        case action_message::message_id::block_summary_update_request:
            return create(id,action_message::object_type::block_summary_object_type);
        //case action_message::message_id::block_summary_update_response:
        case action_message::message_id::block_summary_remove_request:
            return create(id,action_message::object_type::block_summary_object_type);
        //case action_message::message_id::block_summary_remove_response:
        case action_message::message_id::transaction_create_request:
            return create(id,action_message::object_type::transaction_object_type);
        //case action_message::message_id::transaction_create_response:
        case action_message::message_id::transaction_query_request:
            return create(id,action_message::object_type::transaction_object_type);
        //case action_message::message_id::transaction_query_response:
        case action_message::message_id::transaction_update_request:
            return create(id,action_message::object_type::transaction_object_type);
        //case action_message::message_id::transaction_update_response:
        case action_message::message_id::transaction_remove_request:
            return create(id,action_message::object_type::transaction_object_type);
        //case action_message::message_id::transaction_remove_response:
        case action_message::message_id::generated_transaction_create_request:
            return create(id,action_message::object_type::generated_transaction_object_type);
        //case action_message::message_id::generated_transaction_create_response:
        case action_message::message_id::generated_transaction_query_request:
            return create(id,action_message::object_type::generated_transaction_object_type);
        //case action_message::message_id::generated_transaction_query_response:
        case action_message::message_id::generated_transaction_update_request:
            return create(id,action_message::object_type::generated_transaction_object_type);
        //case action_message::message_id::generated_transaction_update_response:
        case action_message::message_id::generated_transaction_remove_request:
            return create(id,action_message::object_type::generated_transaction_object_type);
        //case action_message::message_id::generated_transaction_remove_response:
        case action_message::message_id::producer_create_request:
            return create(id,action_message::object_type::producer_object_type);
        //case action_message::message_id::producer_create_response:
        case action_message::message_id::producer_query_request:
            return create(id,action_message::object_type::producer_object_type);
        //case action_message::message_id::producer_query_response:
        case action_message::message_id::producer_update_request:
            return create(id,action_message::object_type::producer_object_type);
        //case action_message::message_id::producer_update_response:
        case action_message::message_id::producer_remove_request:
            return create(id,action_message::object_type::producer_object_type);
        //case action_message::message_id::producer_remove_response:
        case action_message::message_id::account_control_history_create_request:
            return create(id,action_message::object_type::account_control_history_object_type);
        //case action_message::message_id::account_control_history_create_response:
        case action_message::message_id::account_control_history_query_request:
            return create(id,action_message::object_type::account_control_history_object_type);
        //case action_message::message_id::account_control_history_query_response:
        case action_message::message_id::account_control_history_update_request:
            return create(id,action_message::object_type::account_control_history_object_type);
        //case action_message::message_id::account_control_history_update_response:
        case action_message::message_id::account_control_history_remove_request:
            return create(id,action_message::object_type::account_control_history_object_type);
        //case action_message::message_id::account_control_history_remove_response:
        case action_message::message_id::public_key_history_create_request:
            return create(id,action_message::object_type::public_key_history_object_type);
        //case action_message::message_id::public_key_history_create_response:
        case action_message::message_id::public_key_history_query_request:
            return create(id,action_message::object_type::public_key_history_object_type);
        //case action_message::message_id::public_key_history_query_response:
        case action_message::message_id::public_key_history_update_request:
            return create(id,action_message::object_type::public_key_history_object_type);
        //case action_message::message_id::public_key_history_update_response:
        case action_message::message_id::public_key_history_remove_request:
            return create(id,action_message::object_type::public_key_history_object_type);
        //case action_message::message_id::public_key_history_remove_response:
        case action_message::message_id::table_id_create_request:
            return create(id,action_message::object_type::table_id_object_type);
        //case action_message::message_id::table_id_create_response:
        case action_message::message_id::table_id_query_request:
            return create(id,action_message::object_type::table_id_object_type);
        //case action_message::message_id::table_id_query_response:
        case action_message::message_id::table_id_update_request:
            return create(id,action_message::object_type::table_id_object_type);
        //case action_message::message_id::table_id_update_response:
        case action_message::message_id::table_id_remove_request:
            return create(id,action_message::object_type::table_id_object_type);
        //case action_message::message_id::table_id_remove_response:
        case action_message::message_id::resource_limits_create_request:
            return create(id,action_message::object_type::resource_limits_object_type);
        //case action_message::message_id::resource_limits_create_response:
        case action_message::message_id::resource_limits_query_request:
            return create(id,action_message::object_type::resource_limits_object_type);
        //case action_message::message_id::resource_limits_query_response:
        case action_message::message_id::resource_limits_update_request:
            return create(id,action_message::object_type::resource_limits_object_type);
        //case action_message::message_id::resource_limits_update_response:
        case action_message::message_id::resource_limits_remove_request:
            return create(id,action_message::object_type::resource_limits_object_type);
        //case action_message::message_id::resource_limits_remove_response:
        case action_message::message_id::resource_usage_create_request:
            return create(id,action_message::object_type::resource_usage_object_type);
        //case action_message::message_id::resource_usage_create_response:
        case action_message::message_id::resource_usage_query_request:
            return create(id,action_message::object_type::resource_usage_object_type);
        //case action_message::message_id::resource_usage_query_response:
        case action_message::message_id::resource_usage_update_request:
            return create(id,action_message::object_type::resource_usage_object_type);
        //case action_message::message_id::resource_usage_update_response:
        case action_message::message_id::resource_usage_remove_request:
            return create(id,action_message::object_type::resource_usage_object_type);
        //case action_message::message_id::resource_usage_remove_response:
        case action_message::message_id::resource_limits_state_create_request:
            return create(id,action_message::object_type::resource_limits_state_object_type);
        //case action_message::message_id::resource_limits_state_create_response:
        case action_message::message_id::resource_limits_state_query_request:
            return create(id,action_message::object_type::resource_limits_state_object_type);
        //case action_message::message_id::resource_limits_state_query_response:
        case action_message::message_id::resource_limits_state_update_request:
            return create(id,action_message::object_type::resource_limits_state_object_type);
        //case action_message::message_id::resource_limits_state_update_response:
        case action_message::message_id::resource_limits_state_remove_request:
            return create(id,action_message::object_type::resource_limits_state_object_type);
        //case action_message::message_id::resource_limits_state_remove_response:
        case action_message::message_id::resource_limits_config_create_request:
            return create(id,action_message::object_type::resource_limits_config_object_type);
        //case action_message::message_id::resource_limits_config_create_response:
        case action_message::message_id::resource_limits_config_query_request:
            return create(id,action_message::object_type::resource_limits_config_object_type);
        //case action_message::message_id::resource_limits_config_query_response:
        case action_message::message_id::resource_limits_config_update_request:
            return create(id,action_message::object_type::resource_limits_config_object_type);
        //case action_message::message_id::resource_limits_config_update_response:
        case action_message::message_id::resource_limits_config_remove_request:
            return create(id,action_message::object_type::resource_limits_config_object_type);
        //case action_message::message_id::resource_limits_config_remove_response:
        case action_message::message_id::account_history_create_request:
            return create(id,action_message::object_type::account_history_object_type);
        //case action_message::message_id::account_history_create_response:
        case action_message::message_id::account_history_query_request:
            return create(id,action_message::object_type::account_history_object_type);
        //case action_message::message_id::account_history_query_response:
        case action_message::message_id::account_history_update_request:
            return create(id,action_message::object_type::account_history_object_type);
        //case action_message::message_id::account_history_update_response:
        case action_message::message_id::account_history_remove_request:
            return create(id,action_message::object_type::account_history_object_type);
        //case action_message::message_id::account_history_remove_response:
        case action_message::message_id::action_history_create_request:
            return create(id,action_message::object_type::action_history_object_type);
        //case action_message::message_id::action_history_create_response:
        case action_message::message_id::action_history_query_request:
            return create(id,action_message::object_type::action_history_object_type);
        //case action_message::message_id::action_history_query_response:
        case action_message::message_id::action_history_update_request:
            return create(id,action_message::object_type::action_history_object_type);
        //case action_message::message_id::action_history_update_response:
        case action_message::message_id::action_history_remove_request:
            return create(id,action_message::object_type::action_history_object_type);
        //case action_message::message_id::action_history_remove_response:
        case action_message::message_id::reversible_block_create_request:
            return create(id,action_message::object_type::reversible_block_object_type);
        //case action_message::message_id::reversible_block_create_response:
        case action_message::message_id::reversible_block_query_request:
            return create(id,action_message::object_type::reversible_block_object_type);
        //case action_message::message_id::reversible_block_query_response:
        case action_message::message_id::reversible_block_update_request:
            return create(id,action_message::object_type::reversible_block_object_type);
        //case action_message::message_id::reversible_block_update_response:
        case action_message::message_id::reversible_block_remove_request:
            return create(id,action_message::object_type::reversible_block_object_type);
        //case action_message::message_id::reversible_block_remove_response:
        case action_message::message_id::permission_level_check_request:
            return create(id,action_message::object_type::permission_object_type);
        //case action_message::message_id::permission_level_check_response:
        default:
            return nullptr;
    }
}



raw_request_shared_ptr get_raw_account_create_msg() {
    return create_request_msg(action_message::account_create_request);
}

raw_request_shared_ptr get_raw_account_update_msg(){
    return create_request_msg(action_message::account_update_request);
}
raw_request_shared_ptr get_raw_account_query_msg() {
    return create_request_msg(action_message::account_query_request);
}
raw_request_shared_ptr get_raw_account_remove_msg() {
    return create_request_msg(action_message::account_remove_request);
}

raw_request_shared_ptr get_raw_account_sequence_create_msg() {
    return create_request_msg(action_message::account_sequence_create_request);
}
raw_request_shared_ptr get_raw_account_sequence_query_msg() {
    return create_request_msg(action_message::account_sequence_query_request);
}
raw_request_shared_ptr get_raw_account_sequence_update_msg() {
    return create_request_msg(action_message::account_sequence_update_request);
}
raw_request_shared_ptr get_raw_account_sequence_remove_msg() {
    return create_request_msg(action_message::account_sequence_remove_request);
}

raw_request_shared_ptr get_raw_permission_create_msg() {
    return create_request_msg(action_message::permission_create_request);
}
raw_request_shared_ptr get_raw_permission_query_msg() {
    return create_request_msg(action_message::permission_query_request);
}
raw_request_shared_ptr get_raw_permission_update_msg() {
    return create_request_msg(action_message::permission_update_request);
}
raw_request_shared_ptr get_raw_permission_remove_msg() {
    return create_request_msg(action_message::permission_remove_request);
}

raw_request_shared_ptr get_raw_permission_usage_create_msg() {
    return create_request_msg(action_message::permission_usage_create_request);
}
raw_request_shared_ptr get_raw_permission_usage_query_msg() {
    return create_request_msg(action_message::permission_usage_query_request);
}
raw_request_shared_ptr get_raw_permission_usage_update_msg() {
    return create_request_msg(action_message::permission_usage_update_request);
}
raw_request_shared_ptr get_raw_permission_usage_remove_msg() {
    return create_request_msg(action_message::permission_usage_remove_request);
}

raw_request_shared_ptr get_raw_permission_link_create_msg() {
    return create_request_msg(action_message::permission_link_create_request);
}
raw_request_shared_ptr get_raw_permission_link_query_msg() {
    return create_request_msg(action_message::permission_link_query_request);
}
raw_request_shared_ptr get_raw_permission_link_update_msg() {
    return create_request_msg(action_message::permission_link_update_request);
}
raw_request_shared_ptr get_raw_permission_link_remove_msg() {
    return create_request_msg(action_message::permission_link_remove_request);
}

raw_request_shared_ptr get_raw_key_value_create_msg() {
    return create_request_msg(action_message::key_value_create_request);
}
raw_request_shared_ptr get_raw_key_value_update_msg() {
    return create_request_msg(action_message::key_value_update_request);
}
raw_request_shared_ptr get_raw_key_value_query_msg() {
    return create_request_msg(action_message::key_value_update_request);
}
raw_request_shared_ptr get_raw_key_value_remove_msg() {
    return create_request_msg(action_message::key_value_remove_request);
}

raw_request_shared_ptr get_raw_index64_create_msg() {
    return create_request_msg(action_message::index64_create_request);
}
raw_request_shared_ptr get_raw_index64_query_msg() {
    return create_request_msg(action_message::index64_query_request);
}
raw_request_shared_ptr get_raw_index64_update_msg() {
    return create_request_msg(action_message::index64_update_request);
}
raw_request_shared_ptr get_raw_index64_remove_msg() {
    return create_request_msg(action_message::index64_remove_request);
}

raw_request_shared_ptr get_raw_index128_create_msg() {
    return create_request_msg(action_message::index128_create_request);
}
raw_request_shared_ptr get_raw_index128_query_msg() {
    return create_request_msg(action_message::index128_query_request);
}
raw_request_shared_ptr get_raw_index128_update_msg() {
    return create_request_msg(action_message::index128_update_request);
}
raw_request_shared_ptr get_raw_index128_remove_msg() {
    return create_request_msg(action_message::index128_remove_request);
}

raw_request_shared_ptr get_raw_index256_create_msg() {
    return create_request_msg(action_message::index256_create_request);
}
raw_request_shared_ptr get_raw_index256_query_msg() {
    return create_request_msg(action_message::index256_query_request);
}
raw_request_shared_ptr get_raw_index256_update_msg() {
    return create_request_msg(action_message::index256_update_request);
}
raw_request_shared_ptr get_raw_index256_remove_msg() {
    return create_request_msg(action_message::index256_remove_request);
}

raw_request_shared_ptr get_raw_index_double_create_msg() {
    return create_request_msg(action_message::index_double_create_request);
}
raw_request_shared_ptr get_raw_index_double_query_msg() {
    return create_request_msg(action_message::index_double_query_request);
}
raw_request_shared_ptr get_raw_index_double_update_msg() {
    return create_request_msg(action_message::index_double_update_request);
}
raw_request_shared_ptr get_raw_index_double_remove_msg() {
    return create_request_msg(action_message::index_double_remove_request);
}

raw_request_shared_ptr get_raw_index_long_double_create_msg() {
    return create_request_msg(action_message::index_long_double_create_request);
}
raw_request_shared_ptr get_raw_index_long_double_query_msg() {
    return create_request_msg(action_message::index_long_double_query_request);
}
raw_request_shared_ptr get_raw_index_long_double_update_msg() {
    return create_request_msg(action_message::index_long_double_update_request);
}
raw_request_shared_ptr get_raw_index_long_double_remove_msg() {
    return create_request_msg(action_message::index_long_double_remove_request);
}

raw_request_shared_ptr get_raw_global_property_create_msg() {
    return create_request_msg(action_message::global_property_create_request);
}
raw_request_shared_ptr get_raw_global_property_query_msg() {
    return create_request_msg(action_message::global_property_query_request);
}
raw_request_shared_ptr get_raw_global_property_update_msg() {
    return create_request_msg(action_message::global_property_update_request);
}
raw_request_shared_ptr get_raw_global_property_remove_msg() {
    return create_request_msg(action_message::global_property_remove_request);
}

raw_request_shared_ptr get_raw_dynamic_property_create_msg() {
    return create_request_msg(action_message::dynamic_global_property_create_request);
}
raw_request_shared_ptr get_raw_dynamic_property_query_msg() {
    return create_request_msg(action_message::dynamic_global_property_query_request);
}
raw_request_shared_ptr get_raw_dynamic_property_update_msg() {
    return create_request_msg(action_message::dynamic_global_property_update_request);
}
raw_request_shared_ptr get_raw_dynamic_property_remove_msg() {
    return create_request_msg(action_message::dynamic_global_property_remove_request);
}

raw_request_shared_ptr get_raw_block_summary_create_msg() {
    return create_request_msg(action_message::block_summary_create_request);
}
raw_request_shared_ptr get_raw_block_summary_query_msg() {
    return create_request_msg(action_message::block_summary_query_request);
}
raw_request_shared_ptr get_raw_block_summary_update_msg() {
    return create_request_msg(action_message::block_summary_update_request);
}
raw_request_shared_ptr get_raw_block_summary_remove_msg() {
    return create_request_msg(action_message::block_summary_remove_request);
}

raw_request_shared_ptr get_raw_transaction_create_msg() {
    return create_request_msg(action_message::transaction_create_request);
}
raw_request_shared_ptr get_raw_transaction_query_msg() {
    return create_request_msg(action_message::transaction_query_request);
}
raw_request_shared_ptr get_raw_transaction_update_msg() {
    return create_request_msg(action_message::transaction_update_request);
}
raw_request_shared_ptr get_raw_transaction_remove_msg() {
    return create_request_msg(action_message::transaction_remove_request);
}

raw_request_shared_ptr get_raw_generated_transaction_create_msg() {
    return create_request_msg(action_message::generated_transaction_create_request);
}
raw_request_shared_ptr get_raw_generated_transaction_query_msg() {
    return create_request_msg(action_message::generated_transaction_query_request);
}
raw_request_shared_ptr get_raw_generated_transaction_update_msg() {
    return create_request_msg(action_message::generated_transaction_update_request);
}
raw_request_shared_ptr get_raw_generated_transaction_remove_msg() {
    return create_request_msg(action_message::generated_transaction_remove_request);
}

raw_request_shared_ptr get_raw_producer_create_msg() {
    return create_request_msg(action_message::producer_create_request);
}
raw_request_shared_ptr get_raw_producer_query_msg() {
    return create_request_msg(action_message::producer_query_request);
}
raw_request_shared_ptr get_raw_producer_update_msg() {
    return create_request_msg(action_message::producer_update_request);
}
raw_request_shared_ptr get_raw_producer_remove_msg() {
    return create_request_msg(action_message::producer_remove_request);
}

raw_request_shared_ptr get_raw_account_control_history_create_msg() {
    return create_request_msg(action_message::account_control_history_create_request);
}
raw_request_shared_ptr get_raw_account_control_history_query_msg() {
    return create_request_msg(action_message::account_control_history_query_request);
}
raw_request_shared_ptr get_raw_account_control_history_update_msg() {
    return create_request_msg(action_message::account_control_history_update_request);
}
raw_request_shared_ptr get_raw_account_control_history_remove_msg() {
    return create_request_msg(action_message::account_control_history_remove_request);
}

raw_request_shared_ptr get_raw_public_key_history_create_msg() {
    return create_request_msg(action_message::public_key_history_create_request);
}
raw_request_shared_ptr get_raw_public_key_history_query_msg() {
    return create_request_msg(action_message::public_key_history_query_request);
}
raw_request_shared_ptr get_raw_public_key_history_update_msg() {
    return create_request_msg(action_message::public_key_history_update_request);
}
raw_request_shared_ptr get_raw_public_key_history_remove_msg() {
    return create_request_msg(action_message::public_key_history_remove_request);
}

raw_request_shared_ptr get_raw_table_id_create_msg() {
    return create_request_msg(action_message::table_id_create_request);
}
raw_request_shared_ptr get_raw_table_id_query_msg() {
    return create_request_msg(action_message::table_id_query_request);
}
raw_request_shared_ptr get_raw_table_id_update_msg() {
    return create_request_msg(action_message::table_id_update_request);
}
raw_request_shared_ptr get_raw_table_id_remove_msg() {
    return create_request_msg(action_message::table_id_remove_request);
}

raw_request_shared_ptr get_raw_resource_limits_create_msg() {
    return create_request_msg(action_message::resource_limits_create_request);
}
raw_request_shared_ptr get_raw_resource_limits_query_msg() {
    return create_request_msg(action_message::resource_limits_query_request);
}
raw_request_shared_ptr get_raw_resource_limits_update_msg() {
    return create_request_msg(action_message::resource_limits_update_request);
}
raw_request_shared_ptr get_raw_resource_limits_remove_msg() {
    return create_request_msg(action_message::resource_limits_remove_request);
}

raw_request_shared_ptr get_raw_resource_usage_create_msg() {
    return create_request_msg(action_message::resource_usage_create_request);
}
raw_request_shared_ptr get_raw_resource_usage_query_msg() {
    return create_request_msg(action_message::resource_usage_query_request);
}
raw_request_shared_ptr get_raw_resource_usage_update_msg() {
    return create_request_msg(action_message::resource_usage_update_request);
}
raw_request_shared_ptr get_raw_resource_usage_remove_msg() {
    return create_request_msg(action_message::resource_usage_remove_request);
}

raw_request_shared_ptr get_raw_resource_limits_state_create_msg() {
    return create_request_msg(action_message::resource_limits_state_create_request);
}
raw_request_shared_ptr get_raw_resource_limits_state_query_msg() {
    return create_request_msg(action_message::resource_limits_state_query_request);
}
raw_request_shared_ptr get_raw_resource_limits_state_update_msg() {
    return create_request_msg(action_message::resource_limits_state_update_request);
}
raw_request_shared_ptr get_raw_resource_limits_state_remove_msg() {
    return create_request_msg(action_message::resource_limits_state_remove_request);
}


raw_request_shared_ptr get_raw_resource_limits_config_create_msg() {
    return create_request_msg(action_message::resource_limits_config_create_request);
}
raw_request_shared_ptr get_raw_resource_limits_config_query_msg() {
    return create_request_msg(action_message::resource_limits_config_query_request);
}
raw_request_shared_ptr get_raw_resource_limits_config_update_msg() {
    return create_request_msg(action_message::resource_limits_config_update_request);
}
raw_request_shared_ptr get_raw_resource_limits_config_remove_msg() {
    return create_request_msg(action_message::resource_limits_config_remove_request);
}

raw_request_shared_ptr get_raw_account_history_create_msg() {
    return create_request_msg(action_message::account_history_create_request);
}
raw_request_shared_ptr get_raw_account_history_query_msg() {
    return create_request_msg(action_message::account_history_query_request);
}
raw_request_shared_ptr get_raw_account_history_update_msg() {
    return create_request_msg(action_message::account_history_update_request);
}
raw_request_shared_ptr get_raw_account_history_remove_msg() {
    return create_request_msg(action_message::account_history_remove_request);
}

raw_request_shared_ptr get_raw_action_history_create_msg() {
    return create_request_msg(action_message::action_history_create_request);
}
raw_request_shared_ptr get_raw_action_history_query_msg() {
    return create_request_msg(action_message::action_history_query_request);
}
raw_request_shared_ptr get_raw_action_history_update_msg() {
    return create_request_msg(action_message::action_history_update_request);
}
raw_request_shared_ptr get_raw_action_history_remove_msg() {
    return create_request_msg(action_message::action_history_remove_request);
}

raw_request_shared_ptr get_raw_reversible_block_create_msg() {
    return create_request_msg(action_message::reversible_block_create_request);
}
raw_request_shared_ptr get_raw_reversible_block_query_msg() {
    return create_request_msg(action_message::reversible_block_query_request);
}
raw_request_shared_ptr get_raw_reversible_block_update_msg() {
    return create_request_msg(action_message::reversible_block_update_request);
}
raw_request_shared_ptr get_raw_reversible_block_remove_msg() {
    return create_request_msg(action_message::reversible_block_remove_request);
}

request_message_shared_ptr get_account_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_create_request,action_message::account_object_type}};
}
request_message_shared_ptr get_account_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_update_request,action_message::account_object_type}};
}
request_message_shared_ptr get_account_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_query_request,action_message::account_object_type}};
}
request_message_shared_ptr get_account_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_remove_request,action_message::account_object_type}};
}

request_message_shared_ptr get_account_sequence_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_sequence_create_request,action_message::account_sequence_object_type}};
}
request_message_shared_ptr get_account_sequence_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_sequence_query_request,action_message::account_sequence_object_type}};
}
request_message_shared_ptr get_account_sequence_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_sequence_update_request,action_message::account_sequence_object_type}};
}
request_message_shared_ptr get_account_sequence_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_sequence_remove_request,action_message::account_sequence_object_type}};
}

request_message_shared_ptr get_permission_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_create_request,action_message::permission_object_type}};
}
request_message_shared_ptr get_permission_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_query_request,action_message::permission_object_type}};
}
request_message_shared_ptr get_permission_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_update_request,action_message::permission_object_type}};
}
request_message_shared_ptr get_permission_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_remove_request,action_message::permission_object_type}};
}

request_message_shared_ptr get_permission_usage_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_usage_create_request,action_message::permission_usage_object_type}};
}
request_message_shared_ptr get_permission_usage_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_usage_query_request,action_message::permission_usage_object_type}};
}
request_message_shared_ptr get_permission_usage_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_usage_update_request,action_message::permission_usage_object_type}};
}
request_message_shared_ptr get_permission_usage_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_usage_remove_request,action_message::permission_usage_object_type}};
}

request_message_shared_ptr get_permission_link_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_link_create_request,action_message::permission_link_object_type}};
}
request_message_shared_ptr get_permission_link_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_link_query_request,action_message::permission_link_object_type}};
}
request_message_shared_ptr get_permission_link_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_link_update_request,action_message::permission_link_object_type}};
}
request_message_shared_ptr get_permission_link_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::permission_link_remove_request,action_message::permission_link_object_type}};
}

request_message_shared_ptr get_key_value_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::key_value_create_request,action_message::key_value_object_type}};
}
request_message_shared_ptr get_key_value_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::key_value_update_request,action_message::key_value_object_type}};
}
request_message_shared_ptr get_key_value_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::key_value_query_request,action_message::key_value_object_type}};
}
request_message_shared_ptr get_key_value_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::key_value_remove_request,action_message::key_value_object_type}};
}

request_message_shared_ptr get_index64_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::index64_create_request,action_message::index64_object_type}};
}
request_message_shared_ptr get_index64_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::index64_query_request,action_message::index64_object_type}};
}
request_message_shared_ptr get_index64_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::index64_update_request,action_message::index64_object_type}};
}
request_message_shared_ptr get_index64_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::index64_remove_request,action_message::index64_object_type}};
}

request_message_shared_ptr get_index128_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::index128_create_request,action_message::index128_object_type}};
}
request_message_shared_ptr get_index128_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::index128_query_request,action_message::index128_object_type}};
}
request_message_shared_ptr get_index128_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::index128_update_request,action_message::index128_object_type}};
}
request_message_shared_ptr get_index128_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::index128_remove_request,action_message::index128_object_type}};
}

request_message_shared_ptr get_index256_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::index256_create_request,action_message::index256_object_type}};
}
request_message_shared_ptr get_index256_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::index256_query_request,action_message::index256_object_type}};
}
request_message_shared_ptr get_index256_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::index256_update_request,action_message::index256_object_type}};
}
request_message_shared_ptr get_index256_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::index256_remove_request,action_message::index256_object_type}};
}

request_message_shared_ptr get_index_double_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_double_create_request,action_message::index_double_object_type}};
}
request_message_shared_ptr get_index_double_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_double_query_request,action_message::index_double_object_type}};
}
request_message_shared_ptr get_index_double_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_double_update_request,action_message::index_double_object_type}};
}
request_message_shared_ptr get_index_double_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_double_remove_request,action_message::index_double_object_type}};
}

request_message_shared_ptr get_index_long_double_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_long_double_create_request,action_message::index_long_double_object_type}};
}
request_message_shared_ptr get_index_long_double_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_long_double_query_request,action_message::index_long_double_object_type}};
}
request_message_shared_ptr get_index_long_double_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_long_double_update_request,action_message::index_long_double_object_type}};
}
request_message_shared_ptr get_index_long_double_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::index_long_double_remove_request,action_message::index_long_double_object_type}};
}

request_message_shared_ptr get_global_property_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::global_property_create_request,action_message::global_property_object_type}};
}
request_message_shared_ptr get_global_property_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::global_property_query_request,action_message::global_property_object_type}};
}
request_message_shared_ptr get_global_property_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::global_property_update_request,action_message::global_property_object_type}};
}
request_message_shared_ptr get_global_property_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::global_property_remove_request,action_message::global_property_object_type}};
}

request_message_shared_ptr get_dynamic_property_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::dynamic_global_property_create_request,action_message::dynamic_global_property_object_type}};
}
request_message_shared_ptr get_dynamic_property_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::dynamic_global_property_query_request,action_message::dynamic_global_property_object_type}};
}
request_message_shared_ptr get_dynamic_property_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::dynamic_global_property_update_request,action_message::dynamic_global_property_object_type}};
}
request_message_shared_ptr get_dynamic_property_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::dynamic_global_property_remove_request,action_message::dynamic_global_property_object_type}};
}

request_message_shared_ptr get_block_summary_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::block_summary_create_request,action_message::block_summary_object_type}};
}
request_message_shared_ptr get_block_summary_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::block_summary_query_request,action_message::block_summary_object_type}};
}
request_message_shared_ptr get_block_summary_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::block_summary_update_request,action_message::block_summary_object_type}};
}
request_message_shared_ptr get_block_summary_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::block_summary_remove_request,action_message::block_summary_object_type}};
}

request_message_shared_ptr get_transaction_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::transaction_create_request,action_message::transaction_object_type}};
}
request_message_shared_ptr get_transaction_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::transaction_query_request,action_message::transaction_object_type}};
}
request_message_shared_ptr get_transaction_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::transaction_update_request,action_message::transaction_object_type}};
}
request_message_shared_ptr get_transaction_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::transaction_remove_request,action_message::transaction_object_type}};
}

request_message_shared_ptr get_generated_transaction_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::generated_transaction_create_request,action_message::generated_transaction_object_type}};
}
request_message_shared_ptr get_generated_transaction_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::generated_transaction_query_request,action_message::generated_transaction_object_type}};
}
request_message_shared_ptr get_generated_transaction_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::generated_transaction_update_request,action_message::generated_transaction_object_type}};
}
request_message_shared_ptr get_generated_transaction_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::generated_transaction_remove_request,action_message::generated_transaction_object_type}};
}

request_message_shared_ptr get_producer_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::producer_create_request,action_message::producer_object_type}};
}
request_message_shared_ptr get_producer_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::producer_query_request,action_message::producer_object_type}};
}
request_message_shared_ptr get_producer_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::producer_update_request,action_message::producer_object_type}};
}
request_message_shared_ptr get_producer_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::producer_remove_request,action_message::producer_object_type}};
}

request_message_shared_ptr get_account_control_history_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_control_history_create_request,action_message::account_control_history_object_type}};
}
request_message_shared_ptr get_account_control_history_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_control_history_query_request,action_message::account_control_history_object_type}};
}
request_message_shared_ptr get_account_control_history_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_control_history_update_request,action_message::account_control_history_object_type}};
}
request_message_shared_ptr get_account_control_history_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_control_history_remove_request,action_message::account_control_history_object_type}};
}

request_message_shared_ptr get_public_key_history_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::public_key_history_create_request,action_message::public_key_history_object_type}};
}
request_message_shared_ptr get_public_key_history_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::public_key_history_query_request,action_message::public_key_history_object_type}};
}
request_message_shared_ptr get_public_key_history_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::public_key_history_update_request,action_message::public_key_history_object_type}};
}
request_message_shared_ptr get_public_key_history_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::public_key_history_remove_request,action_message::public_key_history_object_type}};
}

request_message_shared_ptr get_table_id_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::table_id_create_request,action_message::table_id_object_type}};
}
request_message_shared_ptr get_table_id_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::table_id_query_request,action_message::table_id_object_type}};
}
request_message_shared_ptr get_table_id_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::table_id_update_request,action_message::table_id_object_type}};
}
request_message_shared_ptr get_table_id_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::table_id_remove_request,action_message::table_id_object_type}};
}

request_message_shared_ptr get_resource_limits_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_create_request,action_message::resource_limits_object_type}};
}
request_message_shared_ptr get_resource_limits_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_query_request,action_message::resource_limits_object_type}};
}
request_message_shared_ptr get_resource_limits_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_update_request,action_message::resource_limits_object_type}};
}
request_message_shared_ptr get_resource_limits_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_remove_request,action_message::resource_limits_object_type}};
}

request_message_shared_ptr get_resource_usage_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_usage_create_request,action_message::resource_usage_object_type}};
}
request_message_shared_ptr get_resource_usage_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_usage_query_request,action_message::resource_usage_object_type}};
}
request_message_shared_ptr get_resource_usage_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_usage_update_request,action_message::resource_usage_object_type}};
}
request_message_shared_ptr get_resource_usage_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_usage_remove_request,action_message::resource_usage_object_type}};
}

request_message_shared_ptr get_resource_limits_state_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_state_create_request,action_message::resource_limits_state_object_type}};
}
request_message_shared_ptr get_resource_limits_state_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_state_query_request,action_message::resource_limits_state_object_type}};
}
request_message_shared_ptr get_resource_limits_state_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_state_update_request,action_message::resource_limits_state_object_type}};
}
request_message_shared_ptr get_resource_limits_state_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_state_remove_request,action_message::resource_limits_state_object_type}};
}

request_message_shared_ptr get_resource_limits_config_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_config_create_request,action_message::resource_limits_config_object_type}};
}
request_message_shared_ptr get_resource_limits_config_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_config_query_request,action_message::resource_limits_config_object_type}};
}
request_message_shared_ptr get_resource_limits_config_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_config_update_request,action_message::resource_limits_config_object_type}};
}
request_message_shared_ptr get_resource_limits_config_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::resource_limits_config_remove_request,action_message::resource_limits_config_object_type}};
}

request_message_shared_ptr get_account_history_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_history_create_request,action_message::account_history_object_type}};
}
request_message_shared_ptr get_account_history_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_history_query_request,action_message::account_history_object_type}};
}
request_message_shared_ptr get_account_history_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_history_update_request,action_message::account_history_object_type}};
}
request_message_shared_ptr get_account_history_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::account_history_remove_request,action_message::account_history_object_type}};
}

request_message_shared_ptr get_action_history_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::action_history_create_request,action_message::action_history_object_type}};
}
request_message_shared_ptr get_action_history_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::action_history_query_request,action_message::action_history_object_type}};
}
request_message_shared_ptr get_action_history_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::action_history_update_request,action_message::action_history_object_type}};
}
request_message_shared_ptr get_action_history_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::action_history_remove_request,action_message::action_history_object_type}};
}

request_message_shared_ptr get_reversible_block_create_msg() {
    return request_message_shared_ptr{new response_message{action_message::reversible_block_create_request,action_message::reversible_block_object_type}};
}
request_message_shared_ptr get_reversible_block_query_msg() {
    return request_message_shared_ptr{new response_message{action_message::reversible_block_query_request,action_message::reversible_block_object_type}};
}
request_message_shared_ptr get_reversible_block_update_msg() {
    return request_message_shared_ptr{new response_message{action_message::reversible_block_update_request,action_message::reversible_block_object_type}};
}
request_message_shared_ptr get_reversible_block_remove_msg() {
    return request_message_shared_ptr{new response_message{action_message::reversible_block_remove_request,action_message::reversible_block_object_type}};
}
