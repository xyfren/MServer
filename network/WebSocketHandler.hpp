#pragma once

#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace mserver::presentation {

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

class WebSocketHandler {
public:
    void handle(tcp::socket socket, http::request<http::string_body> req) const;
};

} // namespace mserver::presentation
