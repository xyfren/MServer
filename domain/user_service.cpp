#include "business/user_service.hpp"

namespace mserver::business {

UserService::UserService(persistence::dao::UserDao& userDao) : userDao_(userDao) {}

std::optional<persistence::models::User> UserService::getById(int userId) {
    return userDao_.findById(userId);
}

bool UserService::updateProfile(int userId, const std::string& bio) {
    return userDao_.updateProfile(userId, bio);
}

} // namespace mserver::business
