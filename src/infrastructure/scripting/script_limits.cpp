#include "script_limits.hpp"

static void executionLimitHook(lua_State* lua, lua_Debug*) {
    using namespace todod::scripting::limits;
    auto** storage = static_cast<LimitState**>(lua_getextraspace(lua));

    LimitState* state = *storage;
    state->budget.instructionsLeft -= HOOK_INTERVAL;

    const bool instructionLimitReached = state->budget.instructionsLeft <= 0;

    const bool timeoutReached = std::chrono::steady_clock::now() >= state->budget.deadline;

    if (instructionLimitReached) {
        state->status = LimitStatus::InstructionLimitExceeded;
    } else if (timeoutReached) {
        state->status = LimitStatus::TimeLimitExceeded;
    }

    if (instructionLimitReached || timeoutReached) {
        luaL_error(lua, "Lua execution limit exceeded");
    }
}

namespace todod::scripting::limits {

ScriptExecutionLimit::ScriptExecutionLimit(sol::state& lua, std::chrono::milliseconds timeout,
                                           std::int64_t instructionLimit)
    : lua_(lua.lua_state()),
      state_{.status = LimitStatus::NotExceeded,
             .budget = {std::chrono::steady_clock::now() + timeout, instructionLimit}} {
    auto** storage = static_cast<LimitState**>(lua_getextraspace(lua_));

    *storage = &state_;

    lua_sethook(lua_, executionLimitHook, LUA_MASKCOUNT, HOOK_INTERVAL);
}

ScriptExecutionLimit::~ScriptExecutionLimit() {
    lua_sethook(lua_, nullptr, 0, 0);

    auto** storage = static_cast<LimitState**>(lua_getextraspace(lua_));

    *storage = nullptr;
}

LimitStatus ScriptExecutionLimit::status() const noexcept {
    return state_.status;
}

} // namespace todod::scripting::limits
