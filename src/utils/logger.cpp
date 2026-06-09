#include "utils/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace mserver::utils {

namespace {
const char* levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
    }
    return "INFO";
}
} // namespace

void Logger::log(LogLevel level, const std::string& message) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t ts = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &ts);
#else
    localtime_r(&ts, &tm);
#endif
    std::cout << "[" << levelToString(level) << "] "
              << std::put_time(&tm, "%F %T") << " "
              << message << std::endl;
}

} // namespace mserver::utils
