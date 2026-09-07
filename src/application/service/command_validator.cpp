#include "command_validator.hpp"

#include "overload/overloaded.hpp"

namespace todod::service {

CommandValidationResult validateCommand(const scripting::api::ScriptCommand& command) {
    return std::visit(
        helpers::overloaded{
            [](const scripting::api::SetTodoPriorityCommand& value) -> CommandValidationResult {
                if (value.id.id <= 0)
                    return CommandValidationError{CommandValidationErrorCode::InvalidTodoId};
                if (value.priority < 0)
                    return CommandValidationError{CommandValidationErrorCode::NegativePriority};
                return std::nullopt;
            },
            [](const scripting::api::CompleteTodoCommand& value) -> CommandValidationResult {
                if (value.id.id <= 0)
                    return CommandValidationError{CommandValidationErrorCode::InvalidTodoId};
                return std::nullopt;
            }},
        command);
}

} // namespace todod::service
