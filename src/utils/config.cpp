#include "utils/config.hpp"

#include <fstream>

namespace mserver::utils {

AppConfig loadConfig(const std::string& path) {
    AppConfig cfg;
    std::ifstream in(path);
    if (!in) {
        return cfg;
    }

    std::string key;
    while (in >> key) {
        if (key == "http_port") {
            in >> cfg.httpPort;
        } else if (key == "db_path") {
            in >> cfg.dbPath;
        }
    }

    return cfg;
}

} // namespace mserver::utils
