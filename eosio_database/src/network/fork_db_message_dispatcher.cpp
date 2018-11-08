//
// Created by 杨文宇 on 2018/11/8.
//

#include <network/fork_db_message_dispatcher.hpp>
#include <network/fork_database_message_handlers.hpp>
#include <protocol/fork_database_service.pb.h>

using namespace fork_db_message;

fork_db_message_dispatcher::fork_db_message_dispatcher(queue<std::shared_ptr<message>> *q):_queue{q} {
    initialize();
}

void fork_db_message_dispatcher::start() {
    _thread = std::thread([this](){
        run();
    });
}

void fork_db_message_dispatcher::run() {

    for (;;) {
        auto msg = _queue->wait_and_pop();
        rpc_request req;
        req.ParseFromArray(msg->body(),msg->body_length());
        _handlers[req.msg_id()]->handle_message(req,msg);
    }
}

void fork_db_message_dispatcher::initialize() {
    _handlers[block_get_request]                       = new block_get_handler;
    _handlers[block_get_by_num_request]                = new block_get_by_num_handler;
    _handlers[block_set_request]                       = new block_set_handler;
    _handlers[block_add_by_signed_block_request]       = new block_add_by_signed_block_handler;
    _handlers[block_add_by_block_state_request]        = new block_add_by_block_state_handler;
    _handlers[block_remove_by_id_request]              = new block_remove_by_id_handler;
    _handlers[block_add_by_header_confirmation_request]= new block_add_by_header_confirmation_handler;
    _handlers[block_get_head_request]                  = new block_get_head_handler;
    _handlers[block_fetch_branch_request]              = new block_fetch_branch_handler;
    _handlers[block_set_validity_request]              = new block_set_validity_handler;
    _handlers[block_mark_in_current_chain_request]     = new block_mark_in_current_chain_handler;
    _handlers[block_prune_request]                     = new block_prune_handler;
}



