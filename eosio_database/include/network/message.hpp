//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <cstdint>
#include <memory>
#include <network/connection.hpp>

class message {
public:
    message(connection_ptr conn):_conn{conn}{}
    enum {
        max_message_length = 1024 * 1024 * 2
    };

    char* data() {
        return _data;
    }

    char* body() {
        return _data + sizeof(int32_t);
    }

    void body_length(const std::size_t len) {
        _body_length = len;
    }

    void read_body_length() {
        int32_t tmp;
        std::memcpy(&tmp,_data,sizeof(int32_t));
        body_length(tmp);
    }

    const std::size_t body_length() const {
        return _body_length;
    }

    connection_ptr connection() const {
        return _conn;
    }

private:
    char _data[max_message_length + sizeof(int32_t)];
    std::size_t _body_length {0};
    connection_ptr _conn;
};

typedef std::shared_ptr<message> message_ptr;
