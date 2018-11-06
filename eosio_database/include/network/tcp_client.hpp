//
// Created by 杨文宇 on 2018/10/26.
//
#pragma once

#include <string>

class tcp_client {
public:
    tcp_client(std::string &remote,uint32_t port);
    virtual ~tcp_client();

    std::size_t connect();
    void close();

    std::size_t send(const char *data,std::size_t len);
    std::size_t receive(const char *data,std::size_t len);
    void async_send();
    void async_receive();
};
