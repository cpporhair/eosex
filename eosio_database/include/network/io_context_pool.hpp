//
// Created by 杨文宇 on 2018/11/6.
//

#include <thread>
#include <memory>
#include <vector>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>


class io_context_pool : private boost::noncopyable {
public:
    explicit io_context_pool(std::size_t pool_size):_next_io_context{0} {
        if(pool_size ==0)
            throw std::runtime_error("pool size is 0");

        for(std::size_t i = 0;i < pool_size;++i) {
            io_context_ptr cxt{new boost::asio::io_context};
            work_ptr       worker{new boost::asio::io_context::work{*cxt}};
            _io_contexts.push_back(cxt);
            _workers.push_back(worker);
        }

    }

    void run() {
        std::vector<std::shared_ptr<std::thread>> threads;
        for(std::size_t i{0};i < _io_contexts.size();++i) {
            std::shared_ptr<std::thread> thread {
                    new std::thread {
                        boost::bind(&boost::asio::io_context::run,_io_contexts[i])
                    }
            };
            threads.push_back(thread);
        }

        for(std::size_t i{0};i < threads.size();++i)
            threads[i]->join();
    }

    void stop() {
        for(std::size_t i{0};i < _io_contexts.size();++i)
            _io_contexts[i]->stop();
    }

    boost::asio::io_context& get_io_context() {
        boost::asio::io_context &cxt = *_io_contexts[_next_io_context];
        ++_next_io_context;
        if(_next_io_context == _io_contexts.size())
            _next_io_context = 0;

        return cxt;
    }

private:
    typedef std::shared_ptr<boost::asio::io_context> io_context_ptr;
    typedef std::shared_ptr<boost::asio::io_context::work> work_ptr;

    std::vector<io_context_ptr> _io_contexts;
    std::vector<work_ptr>       _workers;
    std::size_t                 _next_io_context;
};