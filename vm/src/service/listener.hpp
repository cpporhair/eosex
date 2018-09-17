//
// Created by anyone on 18-8-18.
//

#ifndef BOXED_LISTENER_HPP
#define BOXED_LISTENER_HPP

#include "core/reactor.hh"
#include "core/thread.hh"
#include "core/future-util.hh"

template<typename CONN_PROC>
constexpr
auto build_listen_proc(CONN_PROC&& _proc){
    return [&_proc](uint16_t _port){
        return seastar::do_with(seastar::engine().listen(seastar::make_ipv4_address({_port}),{true}),
                                [&_proc,_port](auto& listener){
                                    return seastar::keep_doing([&listener,&_proc,_port]{
                                        std::cout<<"listen at "<<_port<<std::endl;
                                        return listener.accept().then([&_proc](seastar::connected_socket fd, seastar::socket_address addr)mutable{
                                            seastar::async([&_proc,&fd,&addr](){
                                                _proc(fd,addr);
                                            });
                                        });
                                    });
                                }
        );
    };
};
#endif //BOXED_LISTENER_HPP
