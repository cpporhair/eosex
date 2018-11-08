//
// Created by 杨文宇 on 2018/11/7.
//
#include <network/dispatcher.hpp>
#include <network/message.hpp>
#include <network/message_handlers.hpp>
#include <protocol/message.pb.h>

using namespace action_message;

static rpc_request generate_request(message_ptr msg) {
    rpc_request req;
    req.ParseFromArray(msg->body(),msg->body_length());
    return req;
}

dispatcher::dispatcher(queue<std::shared_ptr<message>> *q):_queue{q} {
    initialize();
}

void dispatcher::start() {
    _thread = std::thread([this](){
        run();
    });
}

void dispatcher::run() {
    for (;;) {

        auto msg = _queue->wait_and_pop();
        rpc_request req = generate_request(msg);
        _handlers[req.msg_id()]->handle(req,msg);

    }
}

void dispatcher::initialize() {
    _handlers[db_store_i64_request]               = std::shared_ptr<handler>{ new db_store_i64_handler};
    _handlers[db_update_i64_request]              = std::shared_ptr<handler>{ new db_update_i64_handler};
    _handlers[db_remove_i64_request]              = std::shared_ptr<handler>{ new db_remove_i64_handler};
    _handlers[db_get_i64_request]                 = std::shared_ptr<handler>{ new db_get_i64_handler};
    _handlers[db_next_i64_request]                = std::shared_ptr<handler>{ new db_next_i64_handler};
    _handlers[db_previous_i64_request]            = std::shared_ptr<handler>{ new db_previous_i64_handler};
    _handlers[db_find_i64_request]                = std::shared_ptr<handler>{ new db_find_i64_handler};
    _handlers[db_lowerbound_i64_request]          = std::shared_ptr<handler>{ new db_lowerbound_i64_handler};
    _handlers[db_upperbound_i64_request]          = std::shared_ptr<handler>{ new db_upperbound_i64_handler};
    _handlers[db_end_i64_request]                 = std::shared_ptr<handler>{ new db_end_i64_handler};

    _handlers[db_idx64_store_request]             = std::shared_ptr<handler>{ new db_idx64_store_handler};
    _handlers[db_idx64_update_request]            = std::shared_ptr<handler>{ new db_idx64_update_handler};
    _handlers[db_idx64_remove_request]            = std::shared_ptr<handler>{ new db_idx64_remove_handler};
    _handlers[db_idx64_find_secondary_request]    = std::shared_ptr<handler>{ new db_idx64_find_secondary_handler};
    _handlers[db_idx64_find_primary_request]      = std::shared_ptr<handler>{ new db_idx64_find_primary_handler};
    _handlers[db_idx64_lowerbound_request]        = std::shared_ptr<handler>{ new db_idx64_lowerbound_handler};
    _handlers[db_idx64_upperbound_request]        = std::shared_ptr<handler>{ new db_idx64_upperbound_handler};
    _handlers[db_idx64_end_request]               = std::shared_ptr<handler>{ new db_idx64_end_handler};
    _handlers[db_idx64_next_request]              = std::shared_ptr<handler>{ new db_idx64_next_handler};
    _handlers[db_idx64_previous_request]          = std::shared_ptr<handler>{ new db_idx64_previous_handler};

    _handlers[db_idx128_store_request]             = std::shared_ptr<handler>{ new db_idx128_store_handler};
    _handlers[db_idx128_update_request]            = std::shared_ptr<handler>{ new db_idx128_update_handler};
    _handlers[db_idx128_remove_request]            = std::shared_ptr<handler>{ new db_idx128_remove_handler};
    _handlers[db_idx128_find_secondary_request]    = std::shared_ptr<handler>{ new db_idx128_find_secondary_handler};
    _handlers[db_idx128_find_primary_request]      = std::shared_ptr<handler>{ new db_idx128_find_primary_handler};
    _handlers[db_idx128_lowerbound_request]        = std::shared_ptr<handler>{ new db_idx128_lowerbound_handler};
    _handlers[db_idx128_upperbound_request]        = std::shared_ptr<handler>{ new db_idx128_upperbound_handler};
    _handlers[db_idx128_end_request]               = std::shared_ptr<handler>{ new db_idx128_end_handler};
    _handlers[db_idx128_next_request]              = std::shared_ptr<handler>{ new db_idx128_next_handler};
    _handlers[db_idx128_previous_request]          = std::shared_ptr<handler>{ new db_idx128_previous_handler};

    _handlers[db_idx256_store_request]             = std::shared_ptr<handler>{ new db_idx256_store_handler};
    _handlers[db_idx256_update_request]            = std::shared_ptr<handler>{ new db_idx256_update_handler};
    _handlers[db_idx256_remove_request]            = std::shared_ptr<handler>{ new db_idx256_remove_handler};
    _handlers[db_idx256_find_secondary_request]    = std::shared_ptr<handler>{ new db_idx256_find_secondary_handler};
    _handlers[db_idx256_find_primary_request]      = std::shared_ptr<handler>{ new db_idx256_find_primary_handler};
    _handlers[db_idx256_lowerbound_request]        = std::shared_ptr<handler>{ new db_idx256_lowerbound_handler};
    _handlers[db_idx256_upperbound_request]        = std::shared_ptr<handler>{ new db_idx256_upperbound_handler};
    _handlers[db_idx256_end_request]               = std::shared_ptr<handler>{ new db_idx256_end_handler};
    _handlers[db_idx256_next_request]              = std::shared_ptr<handler>{ new db_idx256_next_handler};
    _handlers[db_idx256_previous_request]          = std::shared_ptr<handler>{ new db_idx256_previous_handler};

    _handlers[db_idx_double_store_request]                  = std::shared_ptr<handler>{ new db_idx_double_store_handler};
    _handlers[db_idx_double_update_request]                 = std::shared_ptr<handler>{ new db_idx_double_update_handler};
    _handlers[db_idx_double_remove_request]                 = std::shared_ptr<handler>{ new db_idx_double_remove_handler};
    _handlers[db_idx_double_find_secondary_request]         = std::shared_ptr<handler>{ new db_idx_double_find_secondary_handler};
    _handlers[db_idx_double_find_primary_request]           = std::shared_ptr<handler>{ new db_idx_double_find_primary_handler};
    _handlers[db_idx_double_lowerbound_request]             = std::shared_ptr<handler>{ new db_idx_double_lowerbound_handler};
    _handlers[db_idx_double_upperbound_request]             = std::shared_ptr<handler>{ new db_idx_double_upperbound_handler};
    _handlers[db_idx_double_end_request]                    = std::shared_ptr<handler>{ new db_idx_double_end_handler};
    _handlers[db_idx_double_next_request]                   = std::shared_ptr<handler>{ new db_idx_double_next_handler};
    _handlers[db_idx_double_previous_request]               = std::shared_ptr<handler>{ new db_idx_double_previous_handler};
    _handlers[db_idx_long_double_store_request]             = std::shared_ptr<handler>{ new db_idx_long_double_store_handler};
    _handlers[db_idx_long_double_update_request]            = std::shared_ptr<handler>{ new db_idx_long_double_update_handler};
    _handlers[db_idx_long_double_remove_request]            = std::shared_ptr<handler>{ new db_idx_long_double_remove_handler};
    _handlers[db_idx_long_double_find_secondary_request]    = std::shared_ptr<handler>{ new db_idx_long_double_find_secondary_handler};
    _handlers[db_idx_long_double_find_primary_request]      = std::shared_ptr<handler>{ new db_idx_long_double_find_primary_handler};
    _handlers[db_idx_long_double_lowerbound_request]        = std::shared_ptr<handler>{ new db_idx_long_double_lowerbound_handler};
    _handlers[db_idx_long_double_upperbound_request]        = std::shared_ptr<handler>{ new db_idx_long_double_upperbound_handler};
    _handlers[db_idx_long_double_end_request]               = std::shared_ptr<handler>{ new db_idx_long_double_end_handler};
    _handlers[db_idx_long_double_next_request]              = std::shared_ptr<handler>{ new db_idx_long_double_next_handler};
    _handlers[db_idx_long_double_previous_request]          = std::shared_ptr<handler>{ new db_idx_long_double_previous_handler};

    _handlers[check_transaction_authorization_request]      = std::shared_ptr<handler>{ new check_transaction_authorization_handler};
    _handlers[check_permission_authorization_request]       = std::shared_ptr<handler>{ new check_permission_authorization_handler};
    _handlers[get_permission_last_used_request]             = std::shared_ptr<handler>{ new get_permission_last_used_handler};
    _handlers[get_account_creation_time_request]            = std::shared_ptr<handler>{ new get_account_creation_time_handler};
}