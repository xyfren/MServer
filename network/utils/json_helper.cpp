#include "utils/json_helper.hpp"

namespace mserver::utils {

nlohmann::json parseJsonOrEmpty(const std::string& body) {
    if (body.empty()) {
        return nlohmann::json::object();
    }

    try {
        return nlohmann::json::parse(body);
    } catch (...) {
        return nlohmann::json::object();
    }
}

} // namespace mserver::utils
