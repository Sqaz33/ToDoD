#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include "domain/ids.hpp"
#include "domain/script.hpp"
#include "sol.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace todod::scripting::api {

struct SetTodoPriorityCommand {
    domain::TodoId id;
    int priority;
};

struct CompleteTodoCommand {
    domain::TodoId id;
};

using ScriptCommand = std::variant<SetTodoPriorityCommand, CompleteTodoCommand>;

struct ScriptContext {
    std::vector<ScriptCommand> commands;
    std::vector<std::string> logs;
};

} // namespace todod::scripting::api
