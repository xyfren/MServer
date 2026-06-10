#pragma once

#include <common/common.hpp>

class ISessionRepository {
public:
    virtual ~ISessionRepository() = default;
    virtual net::awaitable<void> saveSession(const std::string& session_id, uint64_t user_id) = 0;
    virtual net::awaitable<bool> isSessionValid(const std::string& session_id) = 0;
};