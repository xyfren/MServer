#include "presentation/http_handler.hpp"

#include "middleware/cors_middleware.hpp"
#include "presentation/router.hpp"
#include "utils/json_helper.hpp"
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace mserver::presentation {

namespace {

http::response<http::string_body> makeJson(http::status status, const nlohmann::json& body) {
    http::response<http::string_body> res{status, 11};
    res.set(http::field::content_type, "application/json");
    res.body() = body.dump();
    res.prepare_payload();
    middleware::applyCors(res);
    return res;
}

std::string queryValue(const std::string& query, const std::string& key, const std::string& fallback) {
    const std::string prefix = key + "=";
    const auto pos = query.find(prefix);
    if (pos == std::string::npos) {
        return fallback;
    }

    const auto end = query.find('&', pos);
    return query.substr(pos + prefix.size(), end == std::string::npos ? std::string::npos : end - pos - prefix.size());
}

} // namespace

HttpHandler::HttpHandler(business::AuthService& authService,
                         business::UserService& userService,
                         business::MessageService& messageService,
                         middleware::AuthMiddleware& authMiddleware)
    : authService_(authService), userService_(userService), messageService_(messageService), authMiddleware_(authMiddleware) {}

http::response<http::string_body> HttpHandler::handle(const http::request<http::string_body>& req) const {
    const auto route = splitPathAndQuery(std::string(req.target()));

    if (req.method() == http::verb::options) {
        return makeJson(http::status::ok, nlohmann::json::object());
    }

    if (route.path == "/auth/login" && req.method() == http::verb::post) {
        const auto payload = utils::parseJsonOrEmpty(req.body());
        const auto token = authService_.login(payload.value("username", ""), payload.value("password", ""));
        if (!token) {
            return makeJson(http::status::unauthorized, {{"error", "invalid credentials"}});
        }
        return makeJson(http::status::ok, {{"token", *token}});
    }

    if (route.path == "/auth/register" && req.method() == http::verb::post) {
        const auto payload = utils::parseJsonOrEmpty(req.body());
        const auto token = authService_.registerUser(payload.value("username", ""), payload.value("password", ""));
        if (!token) {
            return makeJson(http::status::bad_request, {{"error", "unable to register"}});
        }
        return makeJson(http::status::ok, {{"token", *token}});
    }

    if (route.path == "/auth/logout" && req.method() == http::verb::post) {
        std::string auth = std::string(req[http::field::authorization]);
        if (auth.rfind("Bearer ", 0) == 0) {
            auth = auth.substr(7);
        }
        authService_.logout(auth);
        return makeJson(http::status::ok, {{"status", "logged_out"}});
    }

    if (route.path.rfind("/users/", 0) == 0 && req.method() == http::verb::get) {
        int userId = 0;
        try {
            userId = std::stoi(route.path.substr(std::string("/users/").size()));
        } catch (...) {
            return makeJson(http::status::bad_request, {{"error", "invalid user id"}});
        }
        const auto user = userService_.getById(userId);
        if (!user) {
            return makeJson(http::status::not_found, {{"error", "user not found"}});
        }
        return makeJson(http::status::ok, {{"id", user->id}, {"username", user->username}, {"bio", user->bio}});
    }

    if (route.path == "/users/profile" && req.method() == http::verb::put) {
        const auto userId = authMiddleware_.authenticate(req);
        if (!userId) {
            return makeJson(http::status::unauthorized, {{"error", "token required"}});
        }
        const auto payload = utils::parseJsonOrEmpty(req.body());
        if (!userService_.updateProfile(*userId, payload.value("bio", ""))) {
            return makeJson(http::status::not_found, {{"error", "user not found"}});
        }
        return makeJson(http::status::ok, {{"status", "updated"}});
    }

    if (route.path == "/messages" && req.method() == http::verb::get) {
        int limit = 50;
        try {
            limit = std::stoi(queryValue(route.query, "limit", "50"));
        } catch (...) {
            return makeJson(http::status::bad_request, {{"error", "invalid limit"}});
        }
        nlohmann::json messages = nlohmann::json::array();
        for (const auto& message : messageService_.getRecent(limit)) {
            messages.push_back({{"id", message.id}, {"user_id", message.userId}, {"body", message.body}, {"created_at", message.createdAt}});
        }
        return makeJson(http::status::ok, { {"messages", messages} });
    }

    if (route.path.rfind("/avatars/", 0) == 0 && req.method() == http::verb::get) {
        const std::string filename = route.path.substr(std::string("/avatars/").size());
        static const std::regex avatarPattern(R"(^[0-9]+\.jpg$)");
        if (!std::regex_match(filename, avatarPattern)) {
            return makeJson(http::status::bad_request, {{"error", "invalid avatar name"}});
        }
        std::ifstream in("storage/avatars/" + filename, std::ios::binary);
        if (!in) {
            return makeJson(http::status::not_found, {{"error", "avatar not found"}});
        }

        std::ostringstream data;
        data << in.rdbuf();
        http::response<http::string_body> res{http::status::ok, 11};
        res.set(http::field::content_type, "image/jpeg");
        res.body() = data.str();
        res.prepare_payload();
        middleware::applyCors(res);
        return res;
    }

    if (route.path == "/avatars/upload" && req.method() == http::verb::post) {
        const auto userId = authMiddleware_.authenticate(req);
        if (!userId) {
            return makeJson(http::status::unauthorized, {{"error", "token required"}});
        }

        std::filesystem::create_directories("storage/avatars");
        const std::string filename = "storage/avatars/" + std::to_string(*userId) + ".jpg";
        std::ofstream out(filename, std::ios::binary);
        out << req.body();
        // TODO: implement multipart/form-data parsing for richer upload validation.
        return makeJson(http::status::ok, {{"path", "/avatars/" + std::to_string(*userId) + ".jpg"}});
    }

    return makeJson(http::status::not_found, {{"error", "route not found"}});
}

} // namespace mserver::presentation
