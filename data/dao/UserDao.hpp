#pragma once

#include "persistence/Database.hpp"
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
    void upgradeLegacyPassword(int userId, const std::string& passwordHash);

private:
    Database& db_;
};

} // namespace mserver::persistence::dao
