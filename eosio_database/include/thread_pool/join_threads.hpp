//
// Created by 杨文宇 on 2018/11/9.
//
#pragma once

#include <thread>
#include <vector>

class join_threads {
public:
    explicit join_threads(std::vector<std::thread> &threads):_threads{threads}{}
    ~join_threads(){
        for(std::size_t i{0};i < _threads.size();++i) {
            if(_threads[i].joinable())
                _threads[i].join();
        }
    }
private:
    std::vector<std::thread> &_threads;
};
