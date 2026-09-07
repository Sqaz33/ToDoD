#pragma once

#include <optional>

#include "domain/script.hpp"
#include "script_error.hpp"

namespace todod::scripting::validation {

std::optional<error::ScriptError> validateScript(
    const domain::HandlerScriptDefinition& script);

} // namespace todod::scripting::validation
