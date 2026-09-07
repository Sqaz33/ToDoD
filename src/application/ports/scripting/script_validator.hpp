#pragma once

#include "domain/script.hpp"
#include "script_error.hpp"

#include <optional>

namespace todod::scripting::validation {

std::optional<error::ScriptError> validateScript(const domain::HandlerScriptDefinition& script);

} // namespace todod::scripting::validation
