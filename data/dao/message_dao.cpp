#include "persistence/dao/message_dao.hpp"

namespace mserver::persistence::dao {

MessageDao::MessageDao(Database& db) : db_(db) {}

std::vector<models::Message> MessageDao::getRecent(int limit) {
    if (limit <= 0) {
        limit = 50;
    }

    SQLite::Statement q(db_.handle(), "SELECT id, user_id, body, created_at FROM messages ORDER BY id DESC LIMIT ?");
    q.bind(1, limit);

    std::vector<models::Message> out;
    while (q.executeStep()) {
        out.push_back(models::Message{
            q.getColumn(0).getInt(),
            q.getColumn(1).getInt(),
            q.getColumn(2).getString(),
            q.getColumn(3).getString()});
    }
    return out;
}

} // namespace mserver::persistence::dao
