#include "persistence/database.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace mserver::persistence {

Database::Database(const std::string& dbPath)
    : db_(std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)) {}

SQLite::Database& Database::handle() {
    return *db_;
}

void Database::runSchema(const std::string& schemaPath) {
    std::ifstream in(schemaPath);
    if (!in) {
        throw std::runtime_error("Cannot open schema file: " + schemaPath);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    db_->exec(ss.str());

    // Compatibility migration for existing databases created by previous versions.
    bool hasPassword = false;
    bool hasPasswordHash = false;
    bool hasBio = false;

    SQLite::Statement userInfo(*db_, "PRAGMA table_info(users)");
    while (userInfo.executeStep()) {
        const std::string name = userInfo.getColumn(1).getString();
        if (name == "password") {
            hasPassword = true;
        } else if (name == "password_hash") {
            hasPasswordHash = true;
        } else if (name == "bio") {
            hasBio = true;
        }
    }

    if (!hasPasswordHash) {
        db_->exec("ALTER TABLE users ADD COLUMN password_hash TEXT DEFAULT ''");
    }
    if (!hasBio) {
        db_->exec("ALTER TABLE users ADD COLUMN bio TEXT DEFAULT ''");
    }
    if (hasPassword) {
        db_->exec("UPDATE users SET password_hash = password WHERE IFNULL(password_hash, '') = ''");
    }
}

} // namespace mserver::persistence
