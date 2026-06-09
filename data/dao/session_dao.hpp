#pragma once

#include "persistence/database.hpp"
#include <optional>
#include <string>

namespace mserver::persistence::dao {

class SessionDao {
public:
    explicit SessionDao(Database& db);

    void create(const std::string& token, int userId);
    std::optional<int> findUserIdByToken(const std::string& token);
    void remove(const std::string& token);

private:
    Database& db_;
};

} // namespace mserver::persistence::dao
