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
class message;
typedef std::shared_ptr<message> message_ptr;
class connection : public std::enable_shared_from_this<connection> {
public:
    connection(io_context& io);
    tcp::socket& get_socket();
    void start();
    void async_write();
    std::size_t sync_write(const char *buf,std::size_t len);
    std::size_t sync_write(const std::string &data);
    std::size_t sync_write(const char *buf,std::size_t len,boost::system::error_code &ec);
    std::size_t sync_write(const std::string &data,boost::system::error_code &ec);
    void name(const std::string &name) { _name = name; }
    const std::string& name() const { return _name; }
    void manager(connection_manager *mgr) { _manager = mgr; }
    const std::string remote_name() const {
        return _socket.remote_endpoint().address().to_string();
    }

private:
    void read_message_length();
    void read_message_body();
private:
    tcp::socket        _socket;
    std::string        _name;
    connection_manager *_manager;
    message_ptr        _message;
};

typedef std::shared_ptr<connection> connection_ptr;