#include "middleware/auth_middleware.hpp"

namespace mserver::middleware {

AuthMiddleware::AuthMiddleware(business::AuthService& authService) : authService_(authService) {}

std::optional<int> AuthMiddleware::authenticate(const http::request<http::string_body>& req) const {
    const auto value = req[http::field::authorization];
    if (value.empty()) {
        return std::nullopt;
    }

    std::string token = std::string(value);
    const std::string prefix = "Bearer ";
    if (token.rfind(prefix, 0) == 0) {
        token = token.substr(prefix.size());
    }

    return authService_.verifyToken(token);
}

} // namespace mserver::middleware
