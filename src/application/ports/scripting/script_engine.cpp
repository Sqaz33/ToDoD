#include "script_engine.hpp"

#include "infrastructure/scripting/script_safe_execution.hpp"

namespace todod::scripting::engine {

ExecutionResult ScriptEngine::execute(
    const domain::HandlerScript& script,
    const domain::TodoTask& todo) {
    api::ScriptContext context;

    auto&& complete = [&context](std::int64_t id) {
        context.commands.push_back(api::CompleteTodoCommand{{id}});
    };
    auto&& setPriority = [&context](std::int64_t id, int priority) {
        context.commands.push_back(api::SetTodoPriorityCommand{{id}, priority});
    };
    auto&& log = [&context](const std::string& message) {
        context.logs.push_back(message);
    };

    auto error = execution::execute(
        script.def.source(),
        "todo",
        std::tuple{
            execution::named("id", todo.id.id),
            execution::named("title", todo.def.title()),
            execution::named("description", todo.def.description()),
            execution::named("priority", todo.def.priority()),
            execution::named("completed", todo.def.completed())},
        std::tuple{
            execution::named("complete", complete),
            execution::named("set_priority", setPriority),
            execution::named("log", log)},
        error::ScriptPhase::Execution);

    return {.context = std::move(context), .error = std::move(error)};
}

} // namespace todod::scripting::engine
