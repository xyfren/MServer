#pragma once

#include <string>

namespace mserver::persistence::models {

struct Session {
    std::string token;
    int userId{};
    std::string expiresAt;
};

} // namespace mserver::persistence::models
