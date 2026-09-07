#pragma once

#include "application/ports/scripting/script_api.hpp"

#include <optional>

namespace todod::service {

enum class CommandValidationErrorCode {
    InvalidTodoId,
    NegativePriority,
};

struct CommandValidationError {
    CommandValidationErrorCode code;
};

using CommandValidationResult = std::optional<CommandValidationError>;

CommandValidationResult validateCommand(const scripting::api::ScriptCommand& command);

} // namespace todod::service
