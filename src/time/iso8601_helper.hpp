#pragma once

#include <chrono>
#include <string>

namespace todod::helpers {

std::string timePointToIso8601(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point iso8601ToTimePoint(const std::string& str);
bool isValidIso8601(const std::string& str);
} // namespace todod::helpers
