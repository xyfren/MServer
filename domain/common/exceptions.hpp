#pragma once

#include <stdexcept>
#include <string>

namespace mserver::common {

class AppException : public std::runtime_error {
public:
    explicit AppException(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace mserver::common
