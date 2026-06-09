#pragma once

#include <string>

namespace mserver::utils {

std::string hashPassword(const std::string& password);
std::string makeSessionToken();

} // namespace mserver::utils
