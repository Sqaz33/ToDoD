#pragma once

#include <optional>

#include "domain/script.hpp"
#include "domain/todo.hpp"
#include "script_api.hpp"
#include "script_error.hpp"

namespace todod::scripting::engine {

struct ExecutionResult {
    api::ScriptContext context;
    std::optional<error::ScriptError> error;
};

class ScriptEngine {
public:
    ExecutionResult execute(
        const domain::HandlerScript& script,
        const domain::TodoTask& todo);
};

} // namespace todod::scripting::engine
