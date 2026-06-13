#pragma once

#include <string>

struct Message {
    int id{};
    int userId{};
    std::string body;
    std::string createdAt;
};