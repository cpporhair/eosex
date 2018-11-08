#include <iostream>
#include <network/tcp_server.hpp>
#include <network/dispatcher.hpp>
#include <network/fork_db_message_dispatcher.hpp>
#include <thread_safe/queue.hpp>

int main(int argc, char const *argv[]) {

    queue<std::shared_ptr<message>> chain_base_msg_queue,fork_db_msg_queue;
    dispatcher chain_base_dispatcher{&chain_base_msg_queue};
    chain_base_dispatcher.start();

    fork_db_message_dispatcher fork_dispatcher{&fork_db_msg_queue};
    fork_dispatcher.start();

    io_context_pool pool{std::thread::hardware_concurrency()};
    tcp_server chain_base_server{9988,pool,&chain_base_msg_queue};
    std::cout << "chain base service at " << 9988 << std::endl;
    tcp_server fork_db_server{9989,pool,&fork_db_msg_queue};
    std::cout << "fork db service at " << 9989 << std::endl;
    pool.run();
    pool.stop();

    return 0;
}
