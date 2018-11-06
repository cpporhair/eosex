//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <network/io_context_pool.hpp>
#include <network/connection.hpp>

class tcp_server {
public:

    using tcp = boost::asio::ip::tcp;

    tcp_server(uint16_t port);
    tcp_server(const std::string &ip,int16_t port);
    void start_accept(connection_ptr conn,const boost::system::error_code &err);
    void run();
    void stop();
private:
    io_context_pool   _io_context_pool;
    tcp::acceptor     _acceptor;
};