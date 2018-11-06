//
// Created by 杨文宇 on 2018/11/6.
//

#include <functional>
#include <network/tcp_server.hpp>
#include <network/connection_manager.hpp>

tcp_server::tcp_server(uint16_t port):
    _io_context_pool{std::thread::hardware_concurrency()},
    _acceptor{_io_context_pool.get_io_context(),tcp::endpoint{tcp::v4(),port}} {

    connection_ptr conn{new connection{_io_context_pool.get_io_context()}};
    _acceptor.async_accept(conn->get_socket(),
            std::bind(&tcp_server::start_accept,this,conn,boost::asio::placeholders::error));
}

void tcp_server::start_accept(connection_ptr conn, const boost::system::error_code &err) {
    if(!err) {
        connection_manager::get().add(conn->remote_name(),conn);
        conn.reset(new connection{_io_context_pool.get_io_context()});
        _acceptor.async_accept(conn->get_socket(),
                std::bind(&tcp_server::start_accept,this,conn,boost::asio::placeholders::error));
    }
}

void tcp_server::run() {
    _io_context_pool.run();
}

void tcp_server::stop() {
    _io_context_pool.stop();
}

