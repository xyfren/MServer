#pragma once

#include "persistence/dao/session_dao.hpp"
#include "persistence/dao/user_dao.hpp"
#include <optional>
#include <string>

namespace mserver::business {

class AuthService {
public:
    AuthService(persistence::dao::UserDao& userDao, persistence::dao::SessionDao& sessionDao);

    std::optional<std::string> login(const std::string& username, const std::string& password);
    std::optional<std::string> registerUser(const std::string& username, const std::string& password);
    void logout(const std::string& token);
    std::optional<int> verifyToken(const std::string& token);

private:
    persistence::dao::UserDao& userDao_;
    persistence::dao::SessionDao& sessionDao_;
};

} // namespace mserver::business
