//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <network/io_context_pool.hpp>
#include <network/connection.hpp>
#include <thread_safe/queue.hpp>


class connection_manager;
class tcp_server {
public:

    using tcp = boost::asio::ip::tcp;

    tcp_server(uint16_t port,io_context_pool &pool,queue<std::shared_ptr<message>>* q);
    void start_accept();
    void run();
    void stop();
private:
    io_context_pool   &_io_context_pool;
    tcp::acceptor     _acceptor;
    queue<std::shared_ptr<message>>* _queue;
    std::shared_ptr<connection_manager> _manger;
};