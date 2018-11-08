//
// Created by 杨文宇 on 2018/11/6.
//
#include <network/connection_manager.hpp>

void connection_manager::add(const std::string &name, connection_ptr conn) {
    if(_connections.find(name) == _connections.end()) {
        conn->name(name);
        conn->start();
        _connections.emplace(std::make_pair(name,conn));
    }
}

void connection_manager::remove(const std::string &name) {
    if(_connections.find(name) != _connections.end())
        _connections.erase(name);
}

