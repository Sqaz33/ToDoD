#include "timestamp_helpers.hpp"

namespace todod::helpers {

std::int64_t toTimestamp(const std::chrono::system_clock::time_point& time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

std::chrono::system_clock::time_point fromTimestamp(std::int64_t timestamp) {
    return std::chrono::system_clock::time_point{std::chrono::milliseconds{timestamp}};
}

} // namespace todod::helpers
