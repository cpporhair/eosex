//
// Created by 杨文宇 on 2018/11/6.
//

#include <network/connection.hpp>

connection::connection(io_context &io):_socket{io} {}

void connection::start() {

}

tcp::socket& connection::get_socket() {
    return _socket;
}
