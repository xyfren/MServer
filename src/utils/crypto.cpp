#include "utils/crypto.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <functional>

namespace mserver::utils {

std::string hashPassword(const std::string& password) {
    // FIXME: optimize and replace with stronger password hashing (e.g. Argon2/Bcrypt).
    return std::to_string(std::hash<std::string>{}(password));
}

std::string makeSessionToken() {
    return boost::uuids::to_string(boost::uuids::random_generator()());
}

} // namespace mserver::utils
