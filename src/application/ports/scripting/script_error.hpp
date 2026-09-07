#pragma once

#include <optional>
#include <cstdint>
#include <string>

namespace todod::scripting::error {

enum class ScriptPhase {
    Compilation,
    TestRun,
    Execution,
};

enum class ScriptErrorCode {
    SyntaxError,
    RuntimeError,
    InstructionLimitExceeded,
    TimeLimitExceeded,
    MemoryAllocationFailed,
    CommandLimitExceeded,
    LogLimitExceeded,
    InternalError,
};

struct ScriptError {
    ScriptPhase phase;
    ScriptErrorCode code;
    std::string diagnostic;
    std::optional<std::uint32_t> line;
};

} // todod::scripting::error
