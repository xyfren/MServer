#pragma once

#include <string>

namespace mserver::presentation {

struct RouteInfo {
    std::string path;
    std::string query;
};

RouteInfo splitPathAndQuery(const std::string& target);

} // namespace mserver::presentation
