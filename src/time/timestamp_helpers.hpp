#pragma once

#include <chrono>
#include <cstdint>

namespace todod::helpers {

std::int64_t toTimestamp(const std::chrono::system_clock::time_point& time);

std::chrono::system_clock::time_point fromTimestamp(std::int64_t timestamp);

} // namespace todod::helpers
