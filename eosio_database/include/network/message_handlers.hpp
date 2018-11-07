//
// Created by 杨文宇 on 2018/11/7.
//
#pragma once

#include <network/message.hpp>

class handler {
public:
    virtual ~handler(){}
    virtual void handle_message(message_ptr msg) = 0;
};