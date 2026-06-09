#pragma once

#include <cstddef>

namespace mserver::common {

constexpr std::size_t kDefaultMessageLimit = 50;
constexpr int kSessionTtlSeconds = 60 * 60 * 24;
constexpr unsigned short kDefaultHttpPort = 12345;

} // namespace mserver::common
