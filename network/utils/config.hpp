#pragma once

#include <string>

namespace mserver::utils {

struct AppConfig {
    unsigned short httpPort{12345};
    std::string dbPath{"db/test.db"};
};

AppConfig loadConfig(const std::string& path);

} // namespace mserver::utils
