#pragma once

#include <string>

namespace mserver::utils {

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
    static void log(LogLevel level, const std::string& message);
};

} // namespace mserver::utils
