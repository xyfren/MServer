#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>

namespace mserver::persistence {

class Database {
public:
    explicit Database(const std::string& dbPath);

    SQLite::Database& handle();
    void runSchema(const std::string& schemaPath);

private:
    std::unique_ptr<SQLite::Database> db_;
};

} // namespace mserver::persistence
