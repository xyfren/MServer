#pragma once

#include "business/auth_service.hpp"
#include <boost/beast/http.hpp>
#include <optional>

namespace mserver::middleware {

namespace http = boost::beast::http;

class AuthMiddleware {
public:
    explicit AuthMiddleware(business::AuthService& authService);

    std::optional<int> authenticate(const http::request<http::string_body>& req) const;

private:
    business::AuthService& authService_;
};

} // namespace mserver::middleware
