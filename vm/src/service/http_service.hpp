//
// Created by null on 18-8-15.
//

#ifndef BOXED_HTTP_SERVICE_H
#define BOXED_HTTP_SERVICE_H

#include <boost/variant.hpp>
#include "src/utils/connection.hpp"
#include "http-parser/http_parser.h"
#include "../utils/http.hpp"
#include "../utils/unique_ptr_wrapper.hpp"

using namespace boost::hana::literals;

namespace http_service{
    template <http::response_item_type T,auto ...S>
    char* build_res_item(char* buf,http::response_item<T,S...>&& data){
        return buf+sprintf(buf,"%s%s\r\n",data._k,data._v);
    }
    template <typename T0>
    constexpr
    auto build_response_line(char* buf,T0&& t0){
        char* rtn=build_res_item(buf,std::move(t0));
        memset(rtn,0,1);
        return rtn+1;
    };
    template <typename T0,typename ...T1>
    constexpr
    auto build_response_line(char* buf,T0&& t0,T1&& ...t1){
        return build_response_line(build_res_item(buf,std::move(t0)),t1...);
    };

    template <typename... T>
    constexpr
    static auto order_response(char* buf,T&& ...t){
        auto x=boost::hana::make_tuple(t...);
        auto y=boost::hana::transform(http::response_item_order_maker<T...>::sorted_items,[&x](auto&& i){
            return x[boost::hana::at_c<0>(i)];
        });
        return boost::hana::unpack(y,[buf](auto&& ...y0){
            return build_response_line(buf,y0...);
        });
    }

    struct http_header {
        std::string name;
        std::string value;
    };
    struct http_request{
    private:
        static int cb_on_message_begin(http_parser* p){
            static_cast<http_request*>(p->data)->method=p->method;
            return 0;
        }
        static int cb_on_url(http_parser* p, const char *at, size_t length){
            static_cast<http_request*>(p->data)->url=std::string_view(at,length);
            return 0;
        }
        static int cb_on_status(http_parser* p, const char *at, size_t length){
            return 0;
        }
        static int cb_on_header_field(http_parser* p, const char *at, size_t length){
            static_cast<http_request*>(p->data)->pending_header_field=std::string_view(at,length);
            return 0;
        }
        static int cb_on_header_value(http_parser* p, const char *at, size_t length){
            if(strncmp(static_cast<http_request*>(p->data)->pending_header_field.data(),"Connection",10)==0){
                static_cast<http_request*>(p->data)->header_connection=std::string_view(at,length);
            }
            return 0;
        }
        static int cb_on_headers_complete(http_parser* p){
            return 0;
        }
        static int cb_on_body(http_parser* p, const char *at, size_t length){
            return 0;
        }
        static int cb_on_message_complete(http_parser* p){
            return 0;
        }
        static int cb_on_chunk_header(http_parser* p){
            return 0;
        }
        static int cb_on_chunk_complete(http_parser* p){
            return 0;
        }
        void init_cb(){
            settings.on_message_begin=cb_on_message_begin;
            settings.on_url=cb_on_url;
            settings.on_status=cb_on_status;
            settings.on_header_field=cb_on_header_field;
            settings.on_header_value=cb_on_header_value;
            settings.on_headers_complete=cb_on_headers_complete;
            settings.on_body=cb_on_body;
            settings.on_message_complete=cb_on_message_complete;
            settings.on_chunk_header=cb_on_chunk_header;
            settings.on_chunk_complete=cb_on_chunk_complete;
        }
        char* base_line;
        size_t base_line_len;
        size_t base_line_used;
        std::string_view pending_header_field;
        void do_parser(){
            http_parser_init(&parser,HTTP_REQUEST);
            parser.data=this;
            http_parser_execute(&parser,&settings,base_line,base_line_used);
        }
    public:
        http_parser_settings settings;
        http_parser parser;
        uint8_t method;
        std::string_view url;
        std::string_view body;
        std::string_view header_connection;
        //std::vector<http_header&> headers;
        unsigned int flags;
        unsigned short http_major, http_minor;
        const http_request& reset(const char* request_line,size_t len){
            if(base_line_len<(len+1)){
                delete base_line;
                base_line_len=len+1;
                base_line=new char[base_line_len];
            }
            memcpy(base_line,request_line,len);
            base_line_used=len;
            do_parser();
            return *this;
        }
        http_request():base_line_len(10240),base_line_used(0){
            base_line=new char[base_line_len];
            memset(base_line,0,base_line_len);
            init_cb();
        }
        ~http_request(){
            delete[] base_line;
        }
    };

    struct http_service_connection_context : public connection{
    public:
        http_request req;
        char* response_buf;
        http_service_connection_context(seastar::connected_socket&& socket, seastar::socket_address addr)
                :connection(std::move(socket),addr){
            response_buf=new char[10240];
            std::cout<<"build s1"<<std::endl;
        }
        virtual ~http_service_connection_context(){
            delete response_buf;
            std::cout<<"delete s2"<<std::endl;
        }
    };

    constexpr
    auto build_requset=[](http_parser&& parser
            , http_parser_settings&& settings
            , seastar::temporary_buffer<char>&& data
            , http_request&& req){
        http_parser_init(&parser,HTTP_REQUEST);
        parser.data=&req;
        http_parser_execute(&parser,&settings,data.begin(),data.size());
        return seastar::make_ready_future<http_request&&>(std::forward<http_request&&>(req));
    };

    constexpr
    const char* sz="12345";

    template <typename REQ_HANDLER>
    constexpr
    auto connection_proc(REQ_HANDLER&& _handler,seastar::connected_socket& fd, seastar::socket_address& addr){
        return seastar::do_with(
                UNIQUE_PTR_WRAPPER(new http_service_connection_context(std::move(fd),addr)),
                [&_handler](UNIQUE_PTR_WRAPPER<http_service_connection_context>& ctx){
                    return seastar::do_until(
                            [&ctx]()->bool{
                                return ctx.p->_in.eof();
                            },
                            [&ctx,&_handler](){
                                return ctx.p->_in.read()
                                        .then([&ctx,&_handler](seastar::temporary_buffer<char>&& data){

                                            if(data.empty())
                                                return seastar::make_ready_future();

                                            order_response(
                                                    ctx.p->response_buf,
                                                    http::response_item<http::response_item_type::content>(_handler(ctx.p->req.reset(data.begin(),data.size()))),
                                                    http::response_item<http::response_item_type::content_type>("text/html"),
                                                    http::response_item<http::response_item_type::version>("1.1"),
                                                    http::response_item<http::response_item_type::status,http::status_type::ok>(),
                                                    http::response_item<http::response_item_type::connection>("Keep-Alive")
                                            );

                                            return ctx.p->_out.write(ctx.p->response_buf)
                                                    .then([&ctx](){
                                                        return ctx.p->_out.flush().then([](){});
                                                    }).then(([&ctx](){
                                                        return ctx.p->_out.close();
                                                    }));

                                        });
                            }
                    );
                }
        ).finally([](){std::cout<<"connection closed"<<std::endl;});
    };


}

#endif //BOXED_HTTP_SERVICE_H
