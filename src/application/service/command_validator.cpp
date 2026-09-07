#include "command_validator.hpp"

#include "overload/overloaded.hpp"

namespace todod::service {

CommandValidationResult validateCommand(scripting::api::ScriptCommand command) {
    return std::visit(helpers::overloaded{
        [](SetTodoPriorityCommand& setPriority) -> CommandValidationResult {
            if (setPriority.id.id < 0) {
                return CommandValidationError {
                    .code = CommandValidationErrorCode::InvalidTodoId;
                }
            }
            if (setPriority.priority < 0) {
                return CommandValidationError {
                    .code = CommandValidationErrorCode::NegativePriority;
                }
            }
            return std::nullopt;
        },
        [](CompleteTodoCommand& complete) -> CommandValidationResult {
            if (complete.id.id < 0) {
                return CommandValidationError {
                    .code = CommandValidationErrorCode::InvalidTodoId;
                }
            }
            return std::nullopt;
        }
    });

} // namespace todod::service
ё