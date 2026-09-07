#pragma once

#include "sol.hpp"

#include <chrono>
#include <cstdint>

namespace todod::scripting::limits {

constexpr int HOOK_INTERVAL = 10'000;

enum class LimitStatus {
    NotExceeded,
    InstructionLimitExceeded,
    TimeLimitExceeded,
};

struct LimitState {
    LimitStatus status;
    struct ScriptBudget {
        std::chrono::steady_clock::time_point deadline;
        std::int64_t instructionsLeft;
    } budget;
};

class ScriptExecutionLimit {
  public:
    ScriptExecutionLimit(sol::state& lua, std::chrono::milliseconds timeout,
                         std::int64_t instructionLimit);

    ~ScriptExecutionLimit();

    ScriptExecutionLimit(const ScriptExecutionLimit&) = delete;
    ScriptExecutionLimit& operator=(const ScriptExecutionLimit&) = delete;
    ScriptExecutionLimit(ScriptExecutionLimit&&) = delete;
    ScriptExecutionLimit& operator=(ScriptExecutionLimit&&) = delete;

  public:
    LimitStatus status() const noexcept;

  private:
    lua_State* lua_;
    LimitState state_;
};

} // namespace todod::scripting::limits
