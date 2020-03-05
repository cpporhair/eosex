//
// Created by 杨文宇 on 2018/11/8.
//

#include <iostream>
#include <string>
#include <memory>
#include <protocol/message.pb.h>
#include <protocol/fork_database_service.pb.h>
#include <protocol/message_factory.hpp>
#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;


class client {
public:
    client(boost::asio::io_context &cxt):_socket{cxt} {}

    std::size_t  connect(const std::string &ip,int16_t port) {
        boost::asio::ip::address addr;
        addr.from_string(ip);
        tcp::endpoint ep(addr,port);
        boost::system::error_code ec;
        _socket.connect(ep,ec);
        if(!ec)
            return 0;
        std::cout << ec.message() << std::endl;
        return -1;
    }

    std::size_t send(const char *buf,std::size_t len) {
        return boost::asio::write(_socket,boost::asio::buffer(buf,len));
    }



    void close() {
        _socket.close();
    }

private:
    tcp::socket _socket;
};

class message_wrapper {
public:
    void serialize( ::google::protobuf::Message &msg ){
        _length = sizeof(int32_t) + msg.ByteSize();
        int32_t len = msg.ByteSize();
        std::memcpy(_buf,&len,sizeof(int32_t));
        msg.SerializeToArray(_buf+sizeof(int32_t),len);
    }

    const char* data() const {
        return _buf;
    }

    const int32_t length() const {
        return _length;
    }

private:
    char    _buf[1024]{};
    int32_t _length;
};

action_message::rpc_request generate_request(action_message::message_id id) {
    action_message::rpc_request req;
    req.set_msg_id(id);
    return req;
}

void send_db_store_i64(client &c) {
    auto req = generate_request(action_message::db_store_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_update_i64(client &c) {
    auto req = generate_request(action_message::db_update_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_remove_i64(client &c) {
    auto req = generate_request(action_message::db_remove_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_get_i64(client &c) {
    auto req = generate_request(action_message::db_get_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_next_i64(client &c) {
    auto req = generate_request(action_message::db_next_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_previous_i64(client &c) {
    auto req = generate_request(action_message::db_previous_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_find_i64(client &c) {
    auto req = generate_request(action_message::db_find_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_lowerbound_i64(client &c) {
    auto req = generate_request(action_message::db_lowerbound_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_upperbound_i64(client &c) {
    auto req = generate_request(action_message::db_upperbound_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_db_end_i64(client &c) {
    auto req = generate_request(action_message::db_end_i64_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_store(client &c) {
    auto req = generate_request(action_message::db_idx64_store_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_update(client &c) {
    auto req = generate_request(action_message::db_idx64_update_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_remove(client &c) {
    auto req = generate_request(action_message::db_idx64_remove_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_find_secondary(client &c) {
    auto req = generate_request(action_message::db_idx64_find_secondary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_find_primary(client &c) {
    auto req = generate_request(action_message::db_idx64_find_primary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_lowerbound(client &c) {
    auto req = generate_request(action_message::db_idx64_lowerbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_upperbound(client &c) {
    auto req = generate_request(action_message::db_idx64_upperbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_end(client &c) {
    auto req = generate_request(action_message::db_idx64_end_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_next(client &c) {
    auto req = generate_request(action_message::db_idx64_next_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx64_previous(client &c) {
    auto req = generate_request(action_message::db_idx64_previous_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_store(client &c) {
    auto req = generate_request(action_message::db_idx128_store_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_update(client &c) {
    auto req = generate_request(action_message::db_idx128_update_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_remove(client &c) {
    auto req = generate_request(action_message::db_idx128_remove_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_find_secondary(client &c) {
    auto req = generate_request(action_message::db_idx128_find_secondary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_find_primary(client &c) {
    auto req = generate_request(action_message::db_idx128_find_primary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_lowerbound(client &c) {
    auto req = generate_request(action_message::db_idx128_lowerbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_upperbound(client &c) {
    auto req = generate_request(action_message::db_idx128_upperbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_end(client &c) {
    auto req = generate_request(action_message::db_idx128_end_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_next(client &c) {
    auto req = generate_request(action_message::db_idx128_next_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx128_previous(client &c) {
    auto req = generate_request(action_message::db_idx128_previous_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_store(client &c) {
    auto req = generate_request(action_message::db_idx256_store_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_update(client &c) {
    auto req = generate_request(action_message::db_idx256_update_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_remove(client &c) {
    auto req = generate_request(action_message::db_idx256_remove_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_find_secondary(client &c) {
    auto req = generate_request(action_message::db_idx256_find_secondary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_find_primary(client &c) {
    auto req = generate_request(action_message::db_idx256_find_primary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_lowerbound(client &c) {
    auto req = generate_request(action_message::db_idx256_lowerbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_upperbound(client &c) {
    auto req = generate_request(action_message::db_idx256_upperbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_end(client &c) {
    auto req = generate_request(action_message::db_idx256_end_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_next(client &c) {
    auto req = generate_request(action_message::db_idx256_next_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx256_previous(client &c) {
    auto req = generate_request(action_message::db_idx256_previous_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_store(client &c) {
    auto req = generate_request(action_message::db_idx_double_store_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_update(client &c) {
    auto req = generate_request(action_message::db_idx_double_update_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_remove(client &c) {
    auto req = generate_request(action_message::db_idx_double_remove_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_find_secondary(client &c) {
    auto req = generate_request(action_message::db_idx_double_find_secondary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_find_primary(client &c) {
    auto req = generate_request(action_message::db_idx_double_find_primary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_lowerbound(client &c) {
    auto req = generate_request(action_message::db_idx_double_lowerbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_upperbound(client &c) {
    auto req = generate_request(action_message::db_idx_double_upperbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_end(client &c) {
    auto req = generate_request(action_message::db_idx_double_end_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_next(client &c) {
    auto req = generate_request(action_message::db_idx_double_next_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_double_previous(client &c) {
    auto req = generate_request(action_message::db_idx_double_previous_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_store(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_store_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_update(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_update_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_remove(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_remove_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_find_secondary(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_find_secondary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_find_primary(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_find_primary_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_lowerbound(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_lowerbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_upperbound(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_upperbound_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_end(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_end_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_next(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_next_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_idx_long_double_previous(client &c) {
    auto req = generate_request(action_message::db_idx_long_double_previous_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_check_transaction_auth(client &c) {
    auto req = generate_request(action_message::check_transaction_authorization_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_check_permission_auth(client &c) {
    auto req = generate_request(action_message::check_permission_authorization_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_get_permission(client &c) {
    auto req = generate_request(action_message::get_permission_last_used_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_get_account_creation_time(client &c) {
    auto req = generate_request(action_message::get_account_creation_time_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void test_chain_base_service() {

    boost::asio::io_context cxt;
    client c{cxt};
    c.connect("127.0.0.1",9988);
    send_db_store_i64(c);
    send_db_update_i64(c);
    send_db_remove_i64(c);
    send_db_get_i64(c);
    send_db_next_i64(c);
    send_db_previous_i64(c);
    send_db_find_i64(c);
    send_db_lowerbound_i64(c);
    send_db_upperbound_i64(c);
    send_db_end_i64(c);

    send_idx64_store(c);
    send_idx64_update(c);
    send_idx64_remove(c);
    send_idx64_find_secondary(c);
    send_idx64_find_primary(c);
    send_idx64_lowerbound(c);
    send_idx64_upperbound(c);
    send_idx64_end(c);
    send_idx64_next(c);
    send_idx64_previous(c);

    send_idx128_store(c);
    send_idx128_update(c);
    send_idx128_remove(c);
    send_idx128_find_secondary(c);
    send_idx128_find_primary(c);
    send_idx128_lowerbound(c);
    send_idx128_upperbound(c);
    send_idx128_end(c);
    send_idx128_next(c);
    send_idx128_previous(c);

    send_idx256_store(c);
    send_idx256_update(c);
    send_idx256_remove(c);
    send_idx256_find_secondary(c);
    send_idx256_find_primary(c);
    send_idx256_lowerbound(c);
    send_idx256_upperbound(c);
    send_idx256_end(c);
    send_idx256_next(c);
    send_idx256_previous(c);

    send_idx_double_store(c);
    send_idx_double_update(c);
    send_idx_double_remove(c);
    send_idx_double_find_secondary(c);
    send_idx_double_find_primary(c);
    send_idx_double_lowerbound(c);
    send_idx_double_upperbound(c);
    send_idx_double_end(c);
    send_idx_double_next(c);
    send_idx_double_previous(c);

    send_idx_long_double_store(c);
    send_idx_long_double_update(c);
    send_idx_long_double_remove(c);
    send_idx_long_double_find_secondary(c);
    send_idx_long_double_find_primary(c);
    send_idx_long_double_lowerbound(c);
    send_idx_long_double_upperbound(c);
    send_idx_long_double_end(c);
    send_idx_long_double_next(c);
    send_idx_long_double_previous(c);

    send_check_transaction_auth(c);
    send_check_permission_auth(c);
    send_get_permission(c);
    send_get_account_creation_time(c);

    char ch = getchar();
    c.close();
}


fork_db_message::rpc_request generate_request(fork_db_message::message_id id) {
    fork_db_message::rpc_request req;
    req.set_msg_id(id);
    return req;
}

void send_block_get(client &c) {
    auto req = generate_request(fork_db_message::block_get_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_get_by_num(client &c) {
    auto req = generate_request(fork_db_message::block_get_by_num_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_set(client &c) {
    auto req = generate_request(fork_db_message::block_set_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_add_by_signed_block(client &c) {
    auto req = generate_request(fork_db_message::block_add_by_signed_block_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_add_by_block_state(client &c) {
    auto req = generate_request(fork_db_message::block_add_by_block_state_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_remove_by_id(client &c) {
    auto req = generate_request(fork_db_message::block_remove_by_id_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_add_by_header(client &c) {
    auto req = generate_request(fork_db_message::block_add_by_header_confirmation_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_get_head(client &c) {
    auto req = generate_request(fork_db_message::block_get_head_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_fetch_branch(client &c) {
    auto req = generate_request(fork_db_message::block_fetch_branch_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_set_validity(client &c) {
    auto req = generate_request(fork_db_message::block_set_validity_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_mark_in_chain(client &c) {
    auto req = generate_request(fork_db_message::block_mark_in_current_chain_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void send_block_prune(client &c) {
    auto req = generate_request(fork_db_message::block_prune_request);
    message_wrapper msg;
    msg.serialize(req);
    c.send(msg.data(),msg.length());
}

void test_fork_db_service() {

    boost::asio::io_context cxt;
    client c{cxt};
    c.connect("127.0.0.1",9989);

    send_block_get(c);
    send_block_get_by_num(c);
    send_block_set(c);
    send_block_add_by_signed_block(c);
    send_block_add_by_block_state(c);
    send_block_remove_by_id(c);
    send_block_add_by_header(c);
    send_block_get_head(c);
    send_block_fetch_branch(c);
    send_block_set_validity(c);
    send_block_mark_in_chain(c);
    send_block_prune(c);

    char ch = getchar();
    c.close();
}

int main(int argc,char** argv) {

    char _buf[1024*1024*5];
    test_chain_base_service();
    test_fork_db_service();

    std::cout << "Hello,World!" << std::endl;

}
