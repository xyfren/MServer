#pragma once

#include <string>

namespace mserver::persistence::models {

struct Message {
    int id{};
    int userId{};
    std::string body;
    std::string createdAt;
};

} // namespace mserver::persistence::models
