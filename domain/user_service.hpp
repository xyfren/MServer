#pragma once

#include "persistence/dao/user_dao.hpp"
#include "persistence/models/user.hpp"
#include <optional>

namespace mserver::business {

class UserService {
public:
    explicit UserService(persistence::dao::UserDao& userDao);

    std::optional<persistence::models::User> getById(int userId);
    bool updateProfile(int userId, const std::string& bio);

private:
    persistence::dao::UserDao& userDao_;
};

} // namespace mserver::business
