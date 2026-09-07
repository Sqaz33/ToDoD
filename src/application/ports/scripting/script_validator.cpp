#include "script_validator.hpp"

#include "infrastructure/scripting/script_safe_execution.hpp"

namespace todod::scripting::validation {

std::optional<error::ScriptError> validateScript(const domain::HandlerScriptDefinition& script) {
    return execution::execute(script.source(), "todo",
                              std::tuple{execution::named("id", std::int64_t{1}),
                                         execution::named("title", std::string{"test"}),
                                         execution::named("description", std::string{"test"}),
                                         execution::named("priority", std::uint32_t{1}),
                                         execution::named("completed", false)},
                              std::tuple{execution::named("complete", [](std::int64_t) {}),
                                         execution::named("set_priority", [](std::int64_t, int) {}),
                                         execution::named("log", [](const std::string&) {})},
                              error::ScriptPhase::TestRun);
}

} // namespace todod::scripting::validation
