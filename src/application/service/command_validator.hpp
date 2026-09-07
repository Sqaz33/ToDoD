#pragma once

#include "ports/scripting/script_api.hpp"

namespace todod::service {

enum class CommandValidationErrorCode {
    InvalidTodoId,
    NegativePriority,
};

struct CommandValidationError {
    CommandValidationErrorCode code;
}
 
using CommandValidataionResult = std::optional<CommandValidationError>;

CommandValidationResult validateCommand(scripting::api::ScriptCommand command);


} // namespace todod::service
