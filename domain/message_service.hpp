#pragma once

#include "persistence/dao/message_dao.hpp"

namespace mserver::business {

class MessageService {
public:
    explicit MessageService(persistence::dao::MessageDao& messageDao);

    std::vector<persistence::models::Message> getRecent(int limit);

private:
    persistence::dao::MessageDao& messageDao_;
};

} // namespace mserver::business
