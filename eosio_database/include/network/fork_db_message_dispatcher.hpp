//
// Created by 杨文宇 on 2018/11/8.
//
#include <memory>
#include <thread>
#include <cstdint>
#include <map>
#include <thread_safe/queue.hpp>


class fork_db_msg_handler;
class message;
class fork_db_message_dispatcher {
public:
    fork_db_message_dispatcher(queue<std::shared_ptr<message>> *q);
    void start();
private:
    void run();
    void initialize();
private:
    std::map<int32_t,fork_db_msg_handler*>       _handlers;
    queue<std::shared_ptr<message>>* _queue;
    std::thread                      _thread;
};