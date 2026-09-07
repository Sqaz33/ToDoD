#pragma once

#include <cstdint>
#include <optional>
#include <regex>
#include <string>
#include <tuple>
#include <utility>

#define SOL_ALL_SAFETIES_ON 1
#include "application/ports/scripting/script_error.hpp"
#include "script_limits.hpp"
#include "sol.hpp"

namespace todod::scripting::execution {

inline std::optional<std::uint32_t> extractLine(const std::string& text) {
    static const std::regex pattern{R"(\]:([0-9]+):)"};
    std::smatch matches;
    if (!std::regex_search(text, matches, pattern))
        return std::nullopt;
    try {
        return static_cast<std::uint32_t>(std::stoul(matches[1].str()));
    } catch (...) {
        return std::nullopt;
    }
}

template <class T> auto named(std::string name, T value) {
    return std::pair<std::string, T>{std::move(name), std::move(value)};
}

template <class... TableTy, class... CommandTy>
std::optional<error::ScriptError>
execute(const std::string& source, const std::string& tableName, std::tuple<TableTy...> table,
        std::tuple<CommandTy...> commands,
        error::ScriptPhase executionPhase = error::ScriptPhase::Execution) {
    using namespace error;
    using namespace limits;

    sol::state lua;
    auto&& loaded = lua.load(source, "user_script", sol::load_mode::text);

    if (!loaded.valid()) {
        const auto code = [&] {
            switch (loaded.status()) {
            case sol::load_status::syntax:
                return ScriptErrorCode::SyntaxError;
            case sol::load_status::memory:
                return ScriptErrorCode::MemoryAllocationFailed;
            default:
                return ScriptErrorCode::InternalError;
            }
        }();
        sol::error luaError = loaded;
        std::string diagnostic = luaError.what();
        return ScriptError{
            .phase = ScriptPhase::Compilation,
            .code = code,
            .diagnostic = diagnostic,
            .line = extractLine(diagnostic),
        };
    }

    sol::environment environment(lua, sol::create);
    auto&& luaTable = lua.create_table();
    std::apply([&](auto&&... fields) { ((luaTable[fields.first] = fields.second), ...); }, table);
    environment[tableName] = luaTable;

    std::apply([&](auto&&... command) { ((environment[command.first] = command.second), ...); },
               commands);

    constexpr auto timeout = std::chrono::milliseconds{100};
    constexpr std::int64_t instructionLimit = 1'000'000;
    ScriptExecutionLimit limit(lua, timeout, instructionLimit);

    sol::protected_function function = loaded;
    sol::set_environment(environment, function);
    auto&& result = function();

    if (limit.status() != LimitStatus::NotExceeded) {
        const auto code = limit.status() == LimitStatus::InstructionLimitExceeded
                              ? ScriptErrorCode::InstructionLimitExceeded
                              : ScriptErrorCode::TimeLimitExceeded;
        std::string diagnostic = "Lua execution limit exceeded";
        if (!result.valid()) {
            sol::error luaError = result;
            diagnostic = luaError.what();
        }
        return ScriptError{
            .phase = executionPhase,
            .code = code,
            .diagnostic = diagnostic,
            .line = extractLine(diagnostic),
        };
    }

    if (!result.valid()) {
        sol::error luaError = result;
        std::string diagnostic = luaError.what();
        return ScriptError{
            .phase = executionPhase,
            .code = ScriptErrorCode::RuntimeError,
            .diagnostic = diagnostic,
            .line = extractLine(diagnostic),
        };
    }

    return std::nullopt;
}

} // namespace todod::scripting::execution
