#include "persistence/dao/session_dao.hpp"

namespace mserver::persistence::dao {

SessionDao::SessionDao(Database& db) : db_(db) {}

void SessionDao::create(const std::string& token, int userId) {
    SQLite::Statement q(db_.handle(),
        "INSERT INTO sessions(token, user_id, expires_at) VALUES(?, ?, datetime('now', '+1 day'))");
    q.bind(1, token);
    q.bind(2, userId);
    q.exec();
}

std::optional<int> SessionDao::findUserIdByToken(const std::string& token) {
    SQLite::Statement q(db_.handle(),
        "SELECT user_id FROM sessions WHERE token = ? AND datetime(expires_at) > datetime('now')");
    q.bind(1, token);
    if (!q.executeStep()) {
        return std::nullopt;
    }
    return q.getColumn(0).getInt();
}

void SessionDao::remove(const std::string& token) {
    SQLite::Statement q(db_.handle(), "DELETE FROM sessions WHERE token = ?");
    q.bind(1, token);
    q.exec();
}

} // namespace mserver::persistence::dao
