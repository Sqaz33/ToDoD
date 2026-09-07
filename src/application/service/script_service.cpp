#include "script_service.hpp"

#include "command_validator.hpp"
#include "overload/overloaded.hpp"

namespace todod::service {

HandlerScriptService::HandlerScriptService(
    repository::TodoRepository& todoRepo, 
    repository::ScriptRepository& scriptRepo
    db::DataBase& db,
    scripting::engine::ScriptEngine& scriptEngine) : 
    todoRepo_(todoRepo)
    , scriptRepo_(scriptRepo)
    , db_(db)
    , scriptEngine_(scriptEngine)
{}

std::vector<HandlerExecutionResul> HandlerScriptService::runHandlers(
    const domain::TodoTask& todo, 
    domain::TodoEvent event)
{
    auto&& handlers = scriptRepo_.findByEvent(event, access);
    if (handlers) {
        std::vector<HandlerExecutionResul> handlersRes;
        for (auto&& handler : handlers.value()) {
            auto&& engineRes = scriptEngine_.execute(handler, todo);
            HandlerExecutionResult execRes {
                .id = handler.id,
                .scriptError = std::move(engineRes.mbError)
                .logs = std::move(engineRes.context.logs)
            };
            if (execRes.mbError) {
                handlersRes.push_back(std::move(execRes));
                continue; // пропускаем обработчик
            }
            db_.transaction([&](db::DBAccess& access, bool* commit) {
                *commit = true;
                std::size_t idx = 0;
                for (auto&& com : engineRes.context.commands) {
                    auto&& comRes = runCommand(com, access, idx++);
                    execRes.commandResult.push_back(std::move(comRes));
                    auto&& commandResultBack = execRes.commandResult.back();
                    if (commandResultBack.mbCommandError || commandResultBack.mbStorageError) {
                        *commit = false;  // отбрасываем изменения хендлера
                        break;
                    } 
                }
            });
            handlersRes.push_back(std::move(execRes));
        }
        return handlersRes;
    } else {
        return std::unexpected(handlers.error());
    }   
}

CommandResult HandlerScriptService::runCommand_(
    scripting::api::ScriptCommand command,
    db::DBAccess& access,
    std::site_t idx)
{
    CommandResult res {
        .index = idx,
        .command = std::move(command)
    };

    auto validationError = validateCommand(command);
    if (validationError) {
        res.mbCommandError = validationError.value();
        return res;
    }
    std::visit(helpers::overloaded{
        [&](SetTodoPriorityCommand& setPriority) {
            auto&& setPriorityRes = todoRepo.setPriority(
                setPriority.id.id, 
                setPriority.priority, 
                access);
            if (setPriorityRes && !setPriorityRes.value()) {
                res.mbCommandError = RunCommandError {
                    .code = RunCommandErrorCode::TodoNotFound
                };
            } else {
                res.mbStorageError = setPriorityRes.error();
            }
        },
        [&](CompleteTodoCommand& complete) {
            auto&& completeRes = todoRepo.setCompleteStatus(
                complete.id.id, 
                true,
                access);
            if (completeRes && !completeRes.value()) {
                res.mbCommandError = RunCommandError {
                    .code = RunCommandErrorCode::TodoNotFound
                };
            } else {
                res.mbStorageError = completeRes.error();
            }
        }
    });
    return res;
} 

} // namespace todod::service
