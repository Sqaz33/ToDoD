#pragma once

#include <variant>

#include "application/ports/repository/todo_repository.hpp"
#include "application/ports/repository/script_repository.hpp"
#include "application/ports/scripting/script_engine.hpp"
#include "application/ports/scripting/script_validator.hpp"

namespace todod::service {

enum class RunCommandErrorCode {
    TodoNotFound
};

struct RunCommandError {
    RunCommandErrorCode code;
};

using CommandError = std::variant<RunCommandError, CommandValidationError>;

struct CommandResult {
    std::size_t index;
    scripting::api::ScriptCommand command;
    std::optional<CommandError> mbCommandError;
    std::optional<db::error::StorageError> mbStorageError;
}

struct HandlerExecutionResult {
    domain::HandlerScriptId id;
    std::optional<scripting::error::ScriptError> mbSriptError;
    std::vector<std::string> logs;
    std::vector<CommandResult> commandResults;
};

using RunHandlersResult = std::expected<
    std::vector<HandlerExecutionResult>, 
    db::error::StorageError
>;

class HandlerScriptService {
public:
    HandlerScriptService(
        repository::TodoRepository& todoRepo, 
        repository::ScriptRepository& scriptRepo,
        db::DataBase& db,
        scripting::engine::ScriptEngine& scriptEngine);

public:
    RunHandlersResult runHandlers(
        const domain::TodoTask& todo, 
        domain::TodoEvent event);

private:
    CommandResult runCommand_(
        scripting::api::ScriptCommand command,
        db::DBAccess& access,
        std::site_t idx
    );
    
private:
    repository::TodoRepository& todoRepo_;
    repository::ScriptRepository& scriptRepo_;
    db::DataBase& db_;
    scripting::engine::ScriptEngine& scriptEngine_;
};

} // namespace todod::service