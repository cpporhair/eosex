//
// Created by 杨文宇 on 2018/11/5.
//
#pragma once

#include <string>
#include <memory>
#include <exception>
#include <protocol/fork_database_service.pb.h>

class fork_db_request_message {
public:
    fork_db_request_message(fork_db_message::message_id id):_req{} {
        _req.set_msg_id(id);
    }
    void set_data(const char *buf,std::size_t len) {
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
    fork_db_message::rpc_request  _req;
};

typedef std::shared_ptr<fork_db_request_message> fork_db_request_message_ptr;

class fork_db_response_message {
public:
private:
    enum {
        max_message_size = 1024 * 1024 * 2
    };
    fork_db_message::rpc_response _resp;
    char                          _data[max_message_size];
};

typedef std::shared_ptr<fork_db_message::rpc_request> raw_fork_db_request_ptr;

raw_fork_db_request_ptr get_raw_block_get_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_get_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_get_by_num_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_get_by_num_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_set_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_set_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_add_by_signed_block_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_add_by_signed_block_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_add_by_block_state_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_add_by_block_state_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_remove_by_id_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_remove_by_id_request);
    return req;
}

raw_fork_db_request_ptr  get_raw_block_add_by_header_confirmation_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_add_by_header_confirmation_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_get_head_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_get_head_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_fetch_branch_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_fetch_branch_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_set_validity_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_set_validity_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_mark_in_current_chain_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_mark_in_current_chain_request);
    return req;
}

raw_fork_db_request_ptr get_raw_block_prune_request() {
    raw_fork_db_request_ptr req{ new fork_db_message::rpc_request };
    req->set_msg_id(fork_db_message::block_prune_request);
    return req;
}

fork_db_request_message_ptr get_block_get_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_get_request} };
}

fork_db_request_message_ptr get_block_get_by_num_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_get_by_num_request} };
}

fork_db_request_message_ptr get_block_set_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_set_request} };
}

fork_db_request_message_ptr get_block_add_by_signed_block_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_add_by_signed_block_request} };
}

fork_db_request_message_ptr get_block_add_by_block_state_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_add_by_block_state_request} };
}

fork_db_request_message_ptr get_block_remove_by_id_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_remove_by_id_request} };
}

fork_db_request_message_ptr  get_block_add_by_header_confirmation_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_add_by_header_confirmation_request} };
}

fork_db_request_message_ptr get_block_get_head_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_get_head_request} };
}

fork_db_request_message_ptr get_block_fetch_branch_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_fetch_branch_request} };
}

fork_db_request_message_ptr get_block_set_validity_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_set_validity_request} };
}

fork_db_request_message_ptr get_block_mark_in_current_chain_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_mark_in_current_chain_request} };
}

fork_db_request_message_ptr get_block_prune_request() {
    return fork_db_request_message_ptr { new fork_db_request_message{fork_db_message::block_prune_request} };
}
