#include "business/message_service.hpp"

namespace mserver::business {

MessageService::MessageService(persistence::dao::MessageDao& messageDao) : messageDao_(messageDao) {}

std::vector<persistence::models::Message> MessageService::getRecent(int limit) {
    return messageDao_.getRecent(limit);
}

} // namespace mserver::business
