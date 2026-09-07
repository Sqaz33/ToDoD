#include "script_service.hpp"

#include <utility>

#include "overload/overloaded.hpp"

namespace todod::service {

HandlerScriptService::HandlerScriptService(
    repository::TodoRepository& todoRepository,
    repository::ScriptRepository& scriptRepository,
    db::DataBase& database,
    scripting::engine::ScriptEngine& scriptEngine)
    : todoRepository_(todoRepository),
      scriptRepository_(scriptRepository),
      database_(database),
      scriptEngine_(scriptEngine) {}

RunHandlersResult HandlerScriptService::runHandlers(
    const domain::TodoTask& todo,
    domain::TodoEvent event) {
    auto handlers = scriptRepository_.findByEvent(event);
    if (!handlers) return std::unexpected(handlers.error());

    std::vector<HandlerExecutionResult> results;
    results.reserve(handlers->size());

    for (const auto& handler : *handlers) {
        const auto started = std::chrono::steady_clock::now();
        auto engineResult = scriptEngine_.execute(handler, todo);

        HandlerExecutionResult result{
            .id = handler.id,
            .name = handler.def.name(),
            .event = handler.def.event(),
            .logs = std::move(engineResult.context.logs),
        };

        if (engineResult.error) {
            const auto code = engineResult.error->code;
            result.status =
                code == scripting::error::ScriptErrorCode::InstructionLimitExceeded ||
                code == scripting::error::ScriptErrorCode::TimeLimitExceeded
                ? HandlerExecutionStatus::LimitExceeded
                : HandlerExecutionStatus::ScriptError;
            result.scriptError = std::move(engineResult.error);
        } else {
            try {
                database_.transaction([&](db::DBAccess& access, bool* commit) {
                    std::size_t failedIndex = engineResult.context.commands.size();
                    for (std::size_t index = 0; index < engineResult.context.commands.size(); ++index) {
                        auto commandResult = runCommand_(engineResult.context.commands[index], access, index);
                        const bool failed = commandResult.commandError.has_value() ||
                                            commandResult.storageError.has_value();
                        result.commandResults.push_back(std::move(commandResult));
                        if (failed) {
                            *commit = false;
                            failedIndex = index;
                            result.status = HandlerExecutionStatus::CommandError;
                            break;
                        }
                    }

                    if (!*commit) {
                        for (auto& commandResult : result.commandResults) {
                            if (commandResult.status == CommandStatus::Applied) {
                                commandResult.status = CommandStatus::RolledBack;
                            }
                        }
                        for (std::size_t index = failedIndex + 1;
                             index < engineResult.context.commands.size(); ++index) {
                            result.commandResults.push_back(CommandResult{
                                .index = index,
                                .command = engineResult.context.commands[index],
                                .status = CommandStatus::NotExecuted,
                            });
                        }
                    }
                });
            } catch (const SQLite::Exception& exception) {
                result.status = HandlerExecutionStatus::CommandError;
                result.storageError = db::error::StorageError::create(
                    "execute handler transaction", exception);
                for (auto& commandResult : result.commandResults) {
                    if (commandResult.status == CommandStatus::Applied) {
                        commandResult.status = CommandStatus::RolledBack;
                    }
                }
            }
        }

        result.durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started).count();
        results.push_back(std::move(result));
    }

    return results;
}

CommandResult HandlerScriptService::runCommand_(
    const scripting::api::ScriptCommand& command,
    db::DBAccess& access,
    std::size_t index) {
    CommandResult result{.index = index, .command = command};

    if (auto error = validateCommand(command)) {
        result.status = CommandStatus::Failed;
        result.commandError = *error;
        return result;
    }

    std::visit(helpers::overloaded{
        [&](const scripting::api::SetTodoPriorityCommand& value) {
            auto updated = todoRepository_.setPriority(value.id, value.priority, access);
            if (!updated) {
                result.status = CommandStatus::Failed;
                result.storageError = updated.error();
            } else if (!*updated) {
                result.status = CommandStatus::Failed;
                result.commandError = RunCommandError{RunCommandErrorCode::TodoNotFound};
            } else {
                result.status = CommandStatus::Applied;
            }
        },
        [&](const scripting::api::CompleteTodoCommand& value) {
            auto updated = todoRepository_.setCompleteStatus(value.id, true, access);
            if (!updated) {
                result.status = CommandStatus::Failed;
                result.storageError = updated.error();
            } else if (!*updated) {
                result.status = CommandStatus::Failed;
                result.commandError = RunCommandError{RunCommandErrorCode::TodoNotFound};
            } else {
                result.status = CommandStatus::Applied;
            }
        }}, command);

    return result;
}

} // namespace todod::service
