#include "persistence/dao/UserDao.hpp"

namespace mserver::persistence::dao {

UserDao::UserDao(Database& db) : db_(db) {}

std::optional<models::User> UserDao::findById(int userId) {
    SQLite::Statement q(db_.handle(),
                        "SELECT id, username, IFNULL(password_hash, ''), IFNULL(password, ''), IFNULL(bio, '') "
                        "FROM users WHERE id = ?");
    q.bind(1, userId);
    if (!q.executeStep()) {
        return std::nullopt;
    }

    return models::User{
        q.getColumn(0).getInt(),
        q.getColumn(1).getString(),
        q.getColumn(2).getString(),
        q.getColumn(3).getString(),
        q.getColumn(4).getString()};
}

std::optional<models::User> UserDao::findByUsername(const std::string& username) {
    SQLite::Statement q(db_.handle(),
                        "SELECT id, username, IFNULL(password_hash, ''), IFNULL(password, ''), IFNULL(bio, '') "
                        "FROM users WHERE username = ?");
    q.bind(1, username);
    if (!q.executeStep()) {
        return std::nullopt;
    }

    return models::User{
        q.getColumn(0).getInt(),
        q.getColumn(1).getString(),
        q.getColumn(2).getString(),
        q.getColumn(3).getString(),
        q.getColumn(4).getString()};
}

int UserDao::create(const std::string& username, const std::string& passwordHash) {
    SQLite::Statement q(db_.handle(), "INSERT INTO users(username, password_hash, password, bio) VALUES(?, ?, '', '')");
    q.bind(1, username);
    q.bind(2, passwordHash);
    q.exec();
    return static_cast<int>(db_.handle().getLastInsertRowid());
}

bool UserDao::updateProfile(int userId, const std::string& bio) {
    SQLite::Statement q(db_.handle(), "UPDATE users SET bio = ? WHERE id = ?");
    q.bind(1, bio);
    q.bind(2, userId);
    q.exec();
    return db_.handle().getChanges() > 0;
}

void UserDao::upgradeLegacyPassword(int userId, const std::string& passwordHash) {
    SQLite::Statement q(db_.handle(), "UPDATE users SET password_hash = ?, password = '' WHERE id = ?");
    q.bind(1, passwordHash);
    q.bind(2, userId);
    q.exec();
}

} // namespace mserver::persistence::dao
