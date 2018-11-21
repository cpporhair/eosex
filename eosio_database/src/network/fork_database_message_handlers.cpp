//
// Created by 杨文宇 on 2018/11/8.
//
#include <iostream>
#include <network/fork_database_message_handlers.hpp>
#include <protocol/utility.hpp>

void fork_db_msg_handler::send_response(rpc_response &resp, connection_ptr conn) {
    std::unique_ptr<serializer> serial{new serializer};
    serial->serialize(resp);
    conn->sync_write(serial->data(),serial->length());
}

void block_get_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_get_handler" << std::endl;
}

void block_get_by_num_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_get_by_num_handler" << std::endl;
}

void block_set_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_set_handler" << std::endl;
}

void block_add_by_signed_block_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_add_by_signed_block_handler" << std::endl;
}

void block_add_by_block_state_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_add_by_block_state_handler" << std::endl;
}

void block_remove_by_id_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_remove_by_id_handler" << std::endl;
}

void block_add_by_header_confirmation_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_add_by_header_confirmation_handler" << std::endl;
}

void block_get_head_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_get_head_handler" << std::endl;
}

void block_fetch_branch_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_fetch_branch_handler" << std::endl;
}

void block_set_validity_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_get_validity_handler" << std::endl;
}

void block_mark_in_current_chain_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_mark_in_current_chain_handler" << std::endl;
}

void block_prune_handler::handle_message(rpc_request &req,rpc_response &resp) {
    std::cout << "block_prune_handler" << std::endl;
}