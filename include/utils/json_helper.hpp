#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace mserver::utils {

nlohmann::json parseJsonOrEmpty(const std::string& body);

} // namespace mserver::utils
