//
// Created by 杨文宇 on 2018/11/6.
//
#include <boost/bind.hpp>
#include <network/connection.hpp>
#include <network/message.hpp>

connection::connection(io_context &io):_socket{io} {}

void connection::start() {
    _message.reset(new message{shared_from_this()});
    read_message_length();
}

void connection::read_message_length() {
    auto self{shared_from_this()};
    boost::asio::async_read(_socket,boost::asio::buffer(_message->data(),sizeof(int32_t)),
            [this,self](boost::system::error_code ec,std::size_t /*bytes_transferred*/) {
                if(!ec) {
                    _message->read_body_length();
                    read_message_body();
                } else {

                }
            }
    );
}

void connection::read_message_body() {
    auto self(shared_from_this());
    boost::asio::async_read(_socket,boost::asio::buffer(_message->body(),_message->body_length()),
            [this,self](boost::system::error_code ec,std::size_t /*bytes_transferred*/) {
                if(!ec) {
                    _message.reset(new message{shared_from_this()});
                    read_message_length();
                } else {

                }
            }
    );
}

tcp::socket& connection::get_socket() {
    return _socket;
}


std::size_t connection::sync_write(const char *buf, std::size_t len) {
    return boost::asio::write(_socket,boost::asio::buffer(buf,len));
}

std::size_t connection::sync_write(const std::string &data) {
    return sync_write(data.data(),data.size());
}


std::size_t connection::sync_write(const char *buf, std::size_t len, boost::system::error_code &ec) {
    return boost::asio::write(_socket,boost::asio::buffer(buf,len),ec);
}

std::size_t connection::sync_write(const std::string &data, boost::system::error_code &ec) {
    return sync_write(data.data(),data.size(),ec);
}
