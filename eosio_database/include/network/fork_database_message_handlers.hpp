//
// Created by 杨文宇 on 2018/11/7.
//
#include <protocol/fork_database_service.pb.h>
#include <network/message.hpp>

using namespace fork_db_message;
class fork_db_msg_handler {
public:
    virtual ~fork_db_msg_handler(){}
    void handle(rpc_request &req,connection_ptr conn) {
        rpc_response resp;
        handle_message(req,resp);
        send_response(resp,conn);
    }
    virtual void handle_message(rpc_request &req,rpc_response &resp) = 0;

protected:
    void send_response(rpc_response &resp,connection_ptr conn);
};

class block_get_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_get_by_num_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_set_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_add_by_signed_block_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_add_by_block_state_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_remove_by_id_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_add_by_header_confirmation_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_get_head_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_fetch_branch_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_set_validity_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_mark_in_current_chain_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_prune_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_irreversible_broadcast_handler : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};

class block_irreversible_notify : public fork_db_msg_handler {
public:
    virtual void handle_message(rpc_request &req,rpc_response &resp);
};