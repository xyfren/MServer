#include "utils/crypto.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

namespace mserver::utils {

std::string hashPassword(const std::string& password) {
    // FIXME: optimize and replace with stronger password hashing (e.g. Argon2/Bcrypt).
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.data()), password.size(), hash);

    std::ostringstream out;
    for (unsigned char byte : hash) {
        out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return out.str();
}

std::string makeSessionToken() {
    return boost::uuids::to_string(boost::uuids::random_generator()());
}

} // namespace mserver::utils
