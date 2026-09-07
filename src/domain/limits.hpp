#pragma once

#include <cstdint>

namespace todod::domain::limits {

inline constexpr std::size_t TodoTitleMaxBytes = 512;
inline constexpr std::size_t TodoDescriptionMaxBytes = 16 * 1024;

inline constexpr std::size_t HandlerNameMaxBytes = 256;
inline constexpr std::size_t HandlerSourceMaxBytes = 64 * 1024;

} // namespace todod::domain::limits
