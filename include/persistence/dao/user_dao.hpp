#pragma once

#include "persistence/database.hpp"
#include "persistence/models/user.hpp"
#include <optional>

namespace mserver::persistence::dao {

class UserDao {
public:
    explicit UserDao(Database& db);

    std::optional<models::User> findById(int userId);
    std::optional<models::User> findByUsername(const std::string& username);
    int create(const std::string& username, const std::string& passwordHash);
    bool updateProfile(int userId, const std::string& bio);

private:
    Database& db_;
};

} // namespace mserver::persistence::dao
