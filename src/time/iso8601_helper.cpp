#include "iso8601_helper.hpp"

#include <format>
#include <sstream>

namespace todod::helpers {

std::string timePointToIso8601(std::chrono::system_clock::time_point tp) {
    return std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(tp));
}

std::chrono::system_clock::time_point iso8601ToTimePoint(const std::string& str) {
    std::istringstream is{str};
    std::chrono::system_clock::time_point tp;
    is >> std::chrono::parse("%Y-%m-%dT%H:%M:%SZ", tp);

    return tp;
}

bool isValidIso8601(const std::string& str) {
    std::istringstream is{str};
    std::chrono::sys_time<std::chrono::milliseconds> res;
    is >> std::chrono::parse("%Y-%m-%dT%H:%M:%SZ", res);

    if (is.fail() || is.peek() != std::char_traits<char>::eof()) {
        return false;
    }

    return true;
}

} // namespace todod::helpers
