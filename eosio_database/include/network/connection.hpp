//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <cstdlib>
#include <memory>
#include <utility>
#include <string>
#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;
using io_context = boost::asio::io_context;

class connection_manager;
class connection : public std::enable_shared_from_this<connection> {
public:
    connection(io_context& io);
    tcp::socket& get_socket();
    void start();
    void handle_read_msg_len();
    void handle_read_msg_body();
    void async_write(const char *buf,std::size_t len);
    void async_write(const char *buf);
    void async_write(const std::string &data);
    std::size_t sync_write(const char *buf);
    std::size_t sync_write(const char *buf,std::size_t len);
    std::size_t sync_write(const std::string &data);
    void name(const std::string &name);
    const std::string& name() const;
    void manager(connection_manager *mgr);
    const std::string remote_name() const {
        return _socket.remote_endpoint().address().to_string();
    }
private:
    tcp::socket        _socket;
    std::string        _name;
    connection_manager *_manager;
};

typedef std::shared_ptr<connection> connection_ptr;