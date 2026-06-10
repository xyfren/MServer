#pragma once

#include <domain/repository/ISessionRepository.hpp>
#include <domain/repository/IUserRepository.hpp>
#include <optional>
#include <string>

class AuthService {
    IUserRepository& user_repo_;
    ISessionRepository& session_store_;

    std::string GenerateSessionId() {
        return "sess_" + std::to_string(std::rand()); // В реальности используйте криптостойкий UUID
    }

    bool VerifyPassword(const std::string& input, const std::string& hash) {
        return input == hash; // В реальности здесь будет вызов bcrypt/argon2
    }

public:
    AuthService(IUserRepository& user_repo, ISessionRepository& session_store)
        : user_repo_(user_repo), session_store_(session_store) {}

    net::awaitable<std::string> Login(const std::string& login, const std::string& password) {
        // 1. Ищем пользователя
        auto user_opt = co_await user_repo_.getUserByLogin(login);
        if (!user_opt.has_value()) {
            throw std::invalid_argument("User not found");
        }

        // 2. Проверяем пароль
        if (!VerifyPassword(password, user_opt->password_hash)) {
            throw std::invalid_argument("Invalid password");
        }

        // 3. Создаем сессию
        std::string session_id = GenerateSessionId();
        co_await session_store_.saveSession(session_id, user_opt->id);

        // 4. Возвращаем токен сессии
        co_return session_id;
    }
};