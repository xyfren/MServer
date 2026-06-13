#pragma once

#include <data/Database.hpp>
#include <domain/models/User.hpp>
#include <optional>

class UserDao {
public:
    explicit UserDao(Database& db);

    std::optional<User> findById(int userId);
    std::optional<User> findByUsername(const std::string& username);
    int create(const std::string& username, const std::string& passwordHash);
    bool updateProfile(int userId, const std::string& bio);
    void upgradeLegacyPassword(int userId, const std::string& passwordHash);

private:
    Database& db_;
};