//
// Created by 杨文宇 on 2018/11/6.
//
#include <map>
#include <string>
#include <memory>
#include <network/connection.hpp>

class connection_manager {
public:
    connection_manager() = default;
    ~connection_manager() = default;
    void add(const std::string &name,connection_ptr conn);
    void remove(const std::string &name);
private:
    std::map<std::string,connection_ptr> _connections;
};