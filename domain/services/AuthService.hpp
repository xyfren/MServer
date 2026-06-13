#pragma once

#include <domain/repository/ISessionRepository.hpp>
#include <domain/repository/IUserRepository.hpp>
#include <optional>
#include <string>

class AuthService {
    IUserRepository& userRepo_;
    ISessionRepository& sessionStore_;

    std::string GenerateSessionId() {
        return "sess_" + std::to_string(std::rand()); // В реальности используйте криптостойкий UUID
    }

    bool VerifyPassword(const std::string& input, const std::string& hash) {
        return input == hash; // В реальности здесь будет вызов bcrypt/argon2
    }

public:
    AuthService(IUserRepository& user_repo, ISessionRepository& sessionStore)
        : userRepo_(user_repo), sessionStore_(sessionStore) {}

    net::awaitable<std::string> Login(const std::string& login, const std::string& password) {
        auto user_opt = co_await userRepo_.getUserByLogin(login);
        if (!user_opt.has_value()) {
            throw std::invalid_argument("User not found");
        }

        if (!VerifyPassword(password, user_opt->passwordHash)) {
            throw std::invalid_argument("Invalid password");
        }

        // 3. Создаем сессию
        std::string session_id = GenerateSessionId();
        co_await sessionStore_.saveSession(session_id, user_opt->id);

        // 4. Возвращаем токен сессии
        co_return session_id;
    }
};