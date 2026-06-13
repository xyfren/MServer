#pragma once

#include <data/Database.hpp>
#include <optional>
#include <string>

class SessionDao {
public:
    explicit SessionDao(Database& db);

    void create(const std::string& token, int userId);
    std::optional<int> findUserIdByToken(const std::string& token);
    void remove(const std::string& token);

private:
    Database& db_;
};