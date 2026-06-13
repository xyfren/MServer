#pragma once

#include <string>

struct Session {
    std::string token;
    int userId{};
    std::string expiresAt;
};