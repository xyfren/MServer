#pragma once

#include "persistence/database.hpp"
#include "persistence/models/message.hpp"
#include <vector>

namespace mserver::persistence::dao {

class MessageDao {
public:
    explicit MessageDao(Database& db);

    std::vector<models::Message> getRecent(int limit);

private:
    Database& db_;
};

} // namespace mserver::persistence::dao
