#pragma once

#include <boost/beast/http.hpp>

namespace mserver::middleware {

namespace http = boost::beast::http;

template <typename Response>
void applyCors(Response& response) {
    response.set(http::field::access_control_allow_origin, "*");
    response.set(http::field::access_control_allow_methods, "GET,POST,PUT,OPTIONS");
    response.set(http::field::access_control_allow_headers, "Content-Type,Authorization");
}

} // namespace mserver::middleware
