//
// Created by anyone on 18-8-27.
//

#ifndef BOXED_HTTP_HPP
#define BOXED_HTTP_HPP

#include <boost/hana.hpp>
using namespace boost::hana::literals;
namespace http{
    enum class status_type {
        ok = 200, //!< ok
        created = 201, //!< created
        accepted = 202, //!< accepted
        no_content = 204, //!< no_content
        multiple_choices = 300, //!< multiple_choices
        moved_permanently = 301, //!< moved_permanently
        moved_temporarily = 302, //!< moved_temporarily
        not_modified = 304, //!< not_modified
        bad_request = 400, //!< bad_request
        unauthorized = 401, //!< unauthorized
        forbidden = 403, //!< forbidden
        not_found = 404, //!< not_found
        internal_server_error = 500, //!< internal_server_error
        not_implemented = 501, //!< not_implemented
        bad_gateway = 502, //!< bad_gateway
        service_unavailable = 503  //!< service_unavailable
    };

    template <status_type T>
    constexpr
    const char* status_string= nullptr;

    template <> constexpr const char* status_string<status_type::ok> = " 200 OK";
    template <> constexpr const char* status_string<status_type::created> = " 201 Created";
    template <> constexpr const char* status_string<status_type::accepted> = " 202 Accepted";
    template <> constexpr const char* status_string<status_type::no_content> = " 204 No Content";
    template <> constexpr const char* status_string<status_type::multiple_choices> = " 300 Multiple Choices";
    template <> constexpr const char* status_string<status_type::moved_permanently> = " 301 Moved Permanently";
    template <> constexpr const char* status_string<status_type::moved_temporarily> = " 302 Moved Temporarily";
    template <> constexpr const char* status_string<status_type::not_modified> = " 304 Not Modified";
    template <> constexpr const char* status_string<status_type::bad_request> = " 400 Bad Request";
    template <> constexpr const char* status_string<status_type::unauthorized> = " 401 Unauthorized";
    template <> constexpr const char* status_string<status_type::forbidden> = " 403 Forbidden";
    template <> constexpr const char* status_string<status_type::not_found> = " 404 Not Found";
    template <> constexpr const char* status_string<status_type::internal_server_error> = " 500 Internal Server Error";
    template <> constexpr const char* status_string<status_type::not_implemented> = " 501 Not Implemented";
    template <> constexpr const char* status_string<status_type::bad_gateway> = " 502 Bad Gateway";
    template <> constexpr const char* status_string<status_type::service_unavailable> = " 503 Service Unavailable";

    const char* get_status_string(const status_type status) {
        switch (status) {
            case status_type::ok:
                return status_string<status_type::ok>;
            case status_type::created:
                return status_string<status_type::created>;
            case status_type::accepted:
                return status_string<status_type::accepted>;
            case status_type::no_content:
                return status_string<status_type::no_content>;
            case status_type::multiple_choices:
                return status_string<status_type::multiple_choices>;
            case status_type::moved_permanently:
                return status_string<status_type::moved_permanently>;
            case status_type::moved_temporarily:
                return status_string<status_type::moved_temporarily>;
            case status_type::not_modified:
                return status_string<status_type::not_modified>;
            case status_type::bad_request:
                return status_string<status_type::bad_request>;
            case status_type::unauthorized:
                return status_string<status_type::unauthorized>;
            case status_type::forbidden:
                return status_string<status_type::forbidden>;
            case status_type::not_found:
                return status_string<status_type::not_found>;
            case status_type::internal_server_error:
                return status_string<status_type::internal_server_error>;
            case status_type::not_implemented:
                return status_string<status_type::not_implemented>;
            case status_type::bad_gateway:
                return status_string<status_type::bad_gateway>;
            case status_type::service_unavailable:
                return status_string<status_type::service_unavailable>;
            default:
                return status_string<status_type::internal_server_error>;
        }
    }

    enum class response_item_type{
        version,
        status,
        connection,
        content_type,
        content
    };
    template <response_item_type T>
    constexpr
    const char* response_item_name= "\r\n";

    template<> constexpr const char* response_item_name<response_item_type::version> = "HTTP/";
    template<> constexpr const char* response_item_name<response_item_type::status> = "";
    template<> constexpr const char* response_item_name<response_item_type::connection> = "Connection: ";
    template<> constexpr const char* response_item_name<response_item_type::content_type> = "Content-Type: ";
    template<> constexpr const char* response_item_name<response_item_type::content> = "\r\n";

    template <response_item_type T>
    constexpr
    const auto response_item_order= 9999_c;

    template<> constexpr auto response_item_order<response_item_type::version> = 0_c;
    template<> constexpr auto response_item_order<response_item_type::status> = 1_c;
    template<> constexpr auto response_item_order<response_item_type::connection> = 2_c;
    template<> constexpr auto response_item_order<response_item_type::content_type> = 3_c;
    template<> constexpr auto response_item_order<response_item_type::content> = 4_c;

    template <response_item_type T,auto ...S>
    struct response_item{
        const constexpr static auto     _t=response_item_order<T>;
        const constexpr static char*    _k=response_item_name<T>;
        const char*                     _v;
        response_item():_v(""){};
        response_item(const char* v):_v(v){};
    };

    template <status_type S>
    struct response_item<response_item_type::status,S> : public response_item<response_item_type::status>{
        response_item():response_item<response_item_type::status>(status_string<S>){}
    };

    template <typename... T>
    struct response_item_order_maker{
        constexpr
        static auto less=[](auto&& i1,auto&& i2){
            return boost::hana::less(boost::hana::at_c<1>(i1)._t,boost::hana::at_c<1>(i2)._t);
        };
        constexpr
        static
        auto compute_order(){
            typedef decltype(boost::hana::zip_shortest(
                    boost::hana::make_basic_tuple(0_c,1_c,2_c,3_c,4_c,5_c,6_c,7_c,8_c,9_c,10_c,11_c,12_c,13_c,14_c,15_c),
                    boost::hana::tuple<T...>())) _inner_zipped;
            typedef decltype(boost::hana::sort(
                    _inner_zipped(),
                    response_item_order_maker<T...>::less)) response_item_sorted;
            return response_item_sorted();
        }
        typedef decltype(compute_order()) sorted_items_type;
        const static sorted_items_type sorted_items;
    };
    template <typename... T>
    typename response_item_order_maker<T...>::sorted_items_type const response_item_order_maker<T...>::sorted_items;
}
#endif //BOXED_HTTP_HPP
