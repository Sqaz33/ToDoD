#include "json_serializer.hpp"

#include <variant>

#include "overload/overloaded.hpp"
#include "time/iso8601_helper.hpp"

namespace todod::http {

namespace {

std::string commandStatus(service::CommandStatus status) {
    switch (status) {
        case service::CommandStatus::Applied: return "applied";
        case service::CommandStatus::Failed: return "failed";
        case service::CommandStatus::RolledBack: return "rolled_back";
        case service::CommandStatus::NotExecuted: return "not_executed";
    }
    return "failed";
}

std::string handlerStatus(service::HandlerExecutionStatus status) {
    switch (status) {
        case service::HandlerExecutionStatus::Success: return "success";
        case service::HandlerExecutionStatus::ScriptError: return "script_error";
        case service::HandlerExecutionStatus::LimitExceeded: return "limit_exceeded";
        case service::HandlerExecutionStatus::CommandError: return "command_error";
    }
    return "script_error";
}

crow::json::wvalue commandToJson(const service::CommandResult& result) {
    crow::json::wvalue json;
    json["index"] = static_cast<std::uint64_t>(result.index);
    json["status"] = commandStatus(result.status);
    json["error"] = crow::json::wvalue{};

    std::visit(helpers::overloaded{
        [&](const scripting::api::SetTodoPriorityCommand& command) {
            json["type"] = "set_priority";
            json["todoId"] = command.id.id;
            json["arguments"]["priority"] = command.priority;
        },
        [&](const scripting::api::CompleteTodoCommand& command) {
            json["type"] = "complete";
            json["todoId"] = command.id.id;
            json["arguments"] = crow::json::wvalue::empty_object();
        }}, result.command);

    if (result.commandError) {
        std::visit(helpers::overloaded{
            [&](const service::RunCommandError&) {
                json["error"]["code"] = "TODO_NOT_FOUND";
                json["error"]["message"] = "Todo was not found";
            },
            [&](const service::CommandValidationError& error) {
                json["error"]["code"] = error.code == service::CommandValidationErrorCode::NegativePriority
                    ? "INVALID_PRIORITY" : "INVALID_TODO_ID";
                json["error"]["message"] = error.code == service::CommandValidationErrorCode::NegativePriority
                    ? "priority must be greater than or equal to 0"
                    : "todo id must be positive";
            }}, *result.commandError);
    } else if (result.storageError) {
        json["error"]["code"] = "DATABASE_ERROR";
        json["error"]["message"] = "Database command failed";
    }
    return json;
}

} // namespace

crow::json::wvalue todoToJson(const domain::TodoTask& todo) {
    crow::json::wvalue json;
    json["id"] = todo.id.id;
    json["title"] = todo.def.title();
    json["description"] = todo.def.description();
    json["priority"] = todo.def.priority();
    json["completedAt"] = helpers::timePointToIso8601(todo.def.completedAt());
    json["completed"] = todo.def.completed();
    return json;
}

crow::json::wvalue handlerToJson(const domain::HandlerScript& handler) {
    crow::json::wvalue json;
    json["id"] = handler.id.id;
    json["name"] = handler.def.name();
    json["source"] = handler.def.source();
    json["event"] = static_cast<int>(handler.def.event());
    json["enabled"] = handler.def.enabled();
    return json;
}

crow::json::wvalue handlersReportToJson(
    const std::vector<service::HandlerExecutionResult>& handlers) {
    crow::json::wvalue report;
    crow::json::wvalue::list executions;
    crow::json::wvalue::list failedIds;
    std::uint64_t succeeded = 0;

    for (const auto& handler : handlers) {
        crow::json::wvalue execution;
        execution["handlerId"] = handler.id.id;
        execution["handlerName"] = handler.name;
        execution["event"] = static_cast<int>(handler.event);
        execution["status"] = handlerStatus(handler.status);
        execution["durationMs"] = handler.durationMs;

        crow::json::wvalue::list logs;
        for (const auto& log : handler.logs) logs.emplace_back(log);
        execution["logs"] = std::move(logs);

        crow::json::wvalue::list commands;
        for (const auto& command : handler.commandResults) commands.push_back(commandToJson(command));
        execution["commands"] = std::move(commands);
        execution["error"] = crow::json::wvalue{};

        if (handler.status == service::HandlerExecutionStatus::Success) {
            ++succeeded;
        } else {
            failedIds.emplace_back(handler.id.id);
            if (handler.scriptError) {
                const bool limit = handler.status == service::HandlerExecutionStatus::LimitExceeded;
                execution["error"]["code"] = limit ? "EXECUTION_LIMIT_EXCEEDED" : "LUA_RUNTIME_ERROR";
                execution["error"]["message"] = handler.scriptError->diagnostic;
            } else if (handler.storageError) {
                execution["error"]["code"] = "DATABASE_ERROR";
                execution["error"]["message"] = "Handler transaction failed";
            } else {
                execution["error"]["code"] = "COMMAND_TRANSACTION_ROLLED_BACK";
                execution["error"]["message"] = "Handler commands were rolled back";
                for (const auto& command : handler.commandResults) {
                    if (command.status == service::CommandStatus::Failed) {
                        execution["error"]["failedCommandIndex"] = static_cast<std::uint64_t>(command.index);
                        break;
                    }
                }
            }
        }
        executions.push_back(std::move(execution));
    }

    const auto total = static_cast<std::uint64_t>(handlers.size());
    report["allSucceeded"] = succeeded == total;
    report["total"] = total;
    report["succeeded"] = succeeded;
    report["failed"] = total - succeeded;
    report["failedHandlerIds"] = std::move(failedIds);
    report["executions"] = std::move(executions);
    return report;
}

crow::response errorResponse(
    int status, std::string code, std::string message, std::string field) {
    crow::json::wvalue json;
    json["error"]["code"] = std::move(code);
    json["error"]["message"] = message;
    crow::json::wvalue::list details;
    if (!field.empty()) {
        crow::json::wvalue detail;
        detail["field"] = std::move(field);
        detail["message"] = message;
        details.push_back(std::move(detail));
    }
    json["error"]["details"] = std::move(details);
    crow::response response{status, json};
    response.set_header("Content-Type", "application/json");
    return response;
}

crow::response notImplementedResponse() {
    return errorResponse(501, "NOT_IMPLEMENTED", "Route is not implemented yet");
}

} // namespace todod::http
