//
// Created by 杨文宇 on 2018/11/6.
//
#include <map>
#include <string>
#include <network/connection.hpp>

class connection_manager {
public:

    static connection_manager& get() {
        static connection_manager mgr;
        return mgr;
    }

    ~connection_manager() = default;
    void add(const std::string &name,connection_ptr conn);
    void remove(const std::string &name);
private:
    connection_manager() = default;
    std::map<std::string,connection_ptr> _connections;
};
