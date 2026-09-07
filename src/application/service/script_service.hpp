#pragma once

#include "application/ports/repository/script_repository.hpp"
#include "application/ports/repository/todo_repository.hpp"
#include "application/ports/scripting/script_engine.hpp"
#include "command_validator.hpp"

#include <chrono>
#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace todod::service {

enum class RunCommandErrorCode { TodoNotFound };
struct RunCommandError {
    RunCommandErrorCode code;
};
using CommandError = std::variant<RunCommandError, CommandValidationError>;

enum class CommandStatus { Applied, Failed, RolledBack, NotExecuted };

struct CommandResult {
    std::size_t index;
    scripting::api::ScriptCommand command;
    CommandStatus status{CommandStatus::NotExecuted};
    std::optional<CommandError> commandError;
    std::optional<db::error::StorageError> storageError;
};

enum class HandlerExecutionStatus { Success, ScriptError, LimitExceeded, CommandError };

struct HandlerExecutionResult {
    domain::HandlerScriptId id;
    std::string name;
    domain::TodoEvent event;
    HandlerExecutionStatus status{HandlerExecutionStatus::Success};
    std::vector<std::string> logs;
    std::vector<CommandResult> commandResults;
    std::optional<scripting::error::ScriptError> scriptError;
    std::optional<db::error::StorageError> storageError;
    std::int64_t durationMs{0};
};

using RunHandlersResult =
    std::expected<std::vector<HandlerExecutionResult>, db::error::StorageError>;

class HandlerScriptService {
  public:
    HandlerScriptService(repository::TodoRepository& todoRepository,
                         repository::ScriptRepository& scriptRepository, db::DataBase& database,
                         scripting::engine::ScriptEngine& scriptEngine);

    RunHandlersResult runHandlers(const domain::TodoTask& todo, domain::TodoEvent event);

  private:
    CommandResult runCommand_(const scripting::api::ScriptCommand& command, db::DBAccess& access,
                              std::size_t index);

    repository::TodoRepository& todoRepository_;
    repository::ScriptRepository& scriptRepository_;
    db::DataBase& database_;
    scripting::engine::ScriptEngine& scriptEngine_;
};

} // namespace todod::service
