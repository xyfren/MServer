#include "business/auth_service.hpp"

#include "utils/crypto.hpp"

namespace mserver::business {

AuthService::AuthService(persistence::dao::UserDao& userDao, persistence::dao::SessionDao& sessionDao)
    : userDao_(userDao), sessionDao_(sessionDao) {}

std::optional<std::string> AuthService::login(const std::string& username, const std::string& password) {
    const auto user = userDao_.findByUsername(username);
    if (!user || user->passwordHash != utils::hashPassword(password)) {
        return std::nullopt;
    }

    const std::string token = utils::makeSessionToken();
    sessionDao_.create(token, user->id);
    return token;
}

std::optional<std::string> AuthService::registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty() || userDao_.findByUsername(username).has_value()) {
        return std::nullopt;
    }

    const int userId = userDao_.create(username, utils::hashPassword(password));
    const std::string token = utils::makeSessionToken();
    sessionDao_.create(token, userId);
    return token;
}

void AuthService::logout(const std::string& token) {
    sessionDao_.remove(token);
}

std::optional<int> AuthService::verifyToken(const std::string& token) {
    return sessionDao_.findUserIdByToken(token);
}

} // namespace mserver::business
