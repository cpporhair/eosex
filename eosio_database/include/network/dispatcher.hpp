//
// Created by 杨文宇 on 2018/11/7.
//
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <thread>
#include <thread_safe/queue.hpp>

class handler;
class message;
class dispatcher {
public:
    dispatcher(queue<std::shared_ptr<message>> *q);


    void start();
private:
    void run();
    void initialize();
private:
    std::map<int32_t,std::shared_ptr<handler>>  _handlers;
    queue<std::shared_ptr<message>>*            _queue;
    std::thread                                 _thread;
};