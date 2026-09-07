#pragma once

#include <expected>
#include <variant>

#include "application/ports/repository/script_repository.hpp"
#include "application/ports/scripting/script_validator.hpp"

namespace todod::use_cases {

using CreateHandlerError = std::variant<
    domain::HandlerScriptValidationError,
    scripting::error::ScriptError,
    db::error::StorageError>;
using CreateHandlerResult = std::expected<domain::HandlerScript, CreateHandlerError>;

class HandlerUseCases {
public:
    explicit HandlerUseCases(repository::ScriptRepository& repository);

    CreateHandlerResult createHandler(const domain::HandlerScriptInput& input);
    repository::GetHandlersResult getHandlers();

private:
    repository::ScriptRepository& repository_;
};

} // namespace todod::use_cases
