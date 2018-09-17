//
// Created by anyone on 18-8-18.
//

#ifndef BOXED_CONNECTION_HPP
#define BOXED_CONNECTION_HPP

#include "core/iostream.hh"
#include "net/api.hh"
#include <boost/noncopyable.hpp>

struct connection : public boost::noncopyable {
    seastar::connected_socket _socket;
    seastar::socket_address _addr;
    seastar::input_stream<char> _in;
    seastar::output_stream<char> _out;
    connection(seastar::connected_socket&& socket, seastar::socket_address addr)
            : _socket(std::move(socket))
            , _addr(addr)
            , _in(_socket.input())
            , _out(_socket.output())
    {}
};

#endif //BOXED_CONNECTION_HPP
