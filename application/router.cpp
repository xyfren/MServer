#include "presentation/router.hpp"

namespace mserver::presentation {

RouteInfo splitPathAndQuery(const std::string& target) {
    const auto pos = target.find('?');
    if (pos == std::string::npos) {
        return RouteInfo{target, ""};
    }
    return RouteInfo{target.substr(0, pos), target.substr(pos + 1)};
}

} // namespace mserver::presentation
