//
// Created by 杨文宇 on 2018/11/7.
//
#pragma once

#include <protocol/message.pb.h>
#include <network/message.hpp>
#include <protocol/utility.hpp>

using namespace action_message;
class handler {
public:
    virtual ~handler(){
        std::cout << "aa" << std::endl;
    }
    void handle(rpc_request &req,message_ptr msg) {
        rpc_response resp;
        handle_message(req,resp);
        send_response(resp,msg->connection());
    }
    virtual void handle_message(rpc_request &req,rpc_response &resp) = 0;

protected:
    void send_response(rpc_response resp,connection_ptr conn );
};

class db_store_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_update_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_remove_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_get_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_next_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_previous_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_find_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_lowerbound_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_upperbound_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_end_i64_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_store_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_update_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_remove_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_find_secondary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_find_primary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_lowerbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_upperbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_end_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_next_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx64_previous_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_store_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_update_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_remove_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_find_secondary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_find_primary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_lowerbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_upperbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_end_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_next_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx128_previous_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_store_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_update_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_remove_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_find_secondary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_find_primary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_lowerbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_upperbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_end_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_next_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx256_previous_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_store_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_update_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_remove_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_find_secondary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_find_primary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_lowerbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_upperbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_end_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_next_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_double_previous_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_store_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_update_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_remove_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_find_secondary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_find_primary_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_lowerbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_upperbound_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_end_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_next_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class db_idx_long_double_previous_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class check_transaction_authorization_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class check_permission_authorization_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class get_permission_last_used_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class get_account_creation_time_handler : public handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};