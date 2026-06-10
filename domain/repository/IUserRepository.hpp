#pragma once

#include <common/common.hpp>
#include <domain/models/User.hpp>

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual net::awaitable<std::optional<User>> getUserByLogin(const std::string& login) = 0;
};