#pragma once

#include "business/auth_service.hpp"
#include "business/Message_service.hpp"
#include "business/user_service.hpp"
#include "middleware/auth_middleware.hpp"
#include <boost/beast/http.hpp>

namespace mserver::presentation {

namespace http = boost::beast::http;

class HttpHandler {
public:
    HttpHandler(business::AuthService& authService,
                business::UserService& userService,
                business::MessageService& messageService,
                middleware::AuthMiddleware& authMiddleware);

    http::response<http::string_body> handle(const http::request<http::string_body>& req) const;

private:
    business::AuthService& authService_;
    business::UserService& userService_;
    business::MessageService& messageService_;
    middleware::AuthMiddleware& authMiddleware_;
};

} // namespace mserver::presentation
