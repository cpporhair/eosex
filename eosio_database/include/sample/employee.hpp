//
// Created by 杨文宇 on 2018/10/18.
//

#pragma once

#include <string>
#include <boost/filesystem.hpp>

class employee {
public:
    employee(int id,const std::string &name):_id{id},_name{name}{}
    bool operator < (const employee &e) const {
        return _id < e._id;
    }

    const std::string& name() const {
        return _name;
    }

private:
    int         _id;
    std::string _name;
};

template <typename T>
class database {
public:

    void create();
    void modify();
    void remove();

private:
};
