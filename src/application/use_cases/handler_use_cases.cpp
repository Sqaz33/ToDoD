#include "handler_use_cases.hpp"

namespace todod::use_cases {

HandlerUseCases::HandlerUseCases(repository::ScriptRepository& repository)
    : repository_(repository) {}

CreateHandlerResult HandlerUseCases::createHandler(const domain::HandlerScriptInput& input) {
    auto definition = domain::HandlerScriptDefinition::create(input);
    if (!definition)
        return std::unexpected(CreateHandlerError{definition.error()});

    if (auto validationError = scripting::validation::validateScript(*definition)) {
        return std::unexpected(CreateHandlerError{std::move(*validationError)});
    }

    auto created = repository_.create(*definition);
    if (!created)
        return std::unexpected(CreateHandlerError{created.error()});
    return *created;
}

repository::GetHandlersResult HandlerUseCases::getHandlers() {
    return repository_.getAll();
}

} // namespace todod::use_cases
