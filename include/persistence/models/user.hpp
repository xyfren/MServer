#pragma once

#include <string>

namespace mserver::persistence::models {

struct User {
    int id{};
    std::string username;
    std::string passwordHash;
    std::string bio;
};

} // namespace mserver::persistence::models
