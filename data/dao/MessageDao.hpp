#pragma once

#include <data/Database.hpp>
#include <domain/models/Message.hpp>
#include <vector>

class MessageDao {
public:
    explicit MessageDao(Database& db);

    std::vector<Message> getRecent(int limit);

private:
    Database& db_;
};