//
// Created by 杨文宇 on 2018/11/6.
//
#include <utility>
#include <boost/bind.hpp>
#include <network/tcp_server.hpp>
#include <network/connection_manager.hpp>
#include <iostream>

tcp_server::tcp_server(uint16_t port,io_context_pool &pool,queue<std::shared_ptr<message>>* q):
    _io_context_pool{pool},
    _acceptor{_io_context_pool.get_io_context(),
    tcp::endpoint{tcp::v6(),port}},
    _queue{q},
    _manger{new connection_manager} {

    start_accept();

}

void tcp_server::start_accept() {


    _acceptor.async_accept(
            [this](boost::system::error_code ec,tcp::socket socket) {
                if( !ec ) {
                    auto conn = std::make_shared<connection>(std::move(socket),_queue);
                    _manger->add(conn->remote_name(),conn);
                    conn->manager(_manger);
                    start_accept();
                }
            });
}

void tcp_server::run() {
    _io_context_pool.run();
}

void tcp_server::stop() {
    _io_context_pool.stop();
}

