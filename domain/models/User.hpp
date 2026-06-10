#include <string>
#include <stdint.h>

struct User {
    uint64_t id;
    std::string login;
    std::string password_hash;
};