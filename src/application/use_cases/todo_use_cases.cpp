#include "todo_use_cases.hpp"

#include <utility>

namespace todod::use_cases {

TodoUseCases::TodoUseCases(repository::TodoRepository& todoRepository,
                           service::HandlerScriptService& handlerService)
    : todoRepository_(todoRepository), handlerService_(handlerService) {}

TodoCreationResult TodoUseCases::createTodo(const domain::TodoInput& input) {
    auto definition = domain::TodoDefinition::create(input);
    if (!definition)
        return std::unexpected(TodoCreationError{definition.error()});

    auto created = todoRepository_.create(*definition);
    if (!created)
        return std::unexpected(TodoCreationError{created.error()});

    auto handlers = handlerService_.runHandlers(*created, domain::TodoEvent::ADDED_TODO);
    if (!handlers)
        return std::unexpected(TodoCreationError{handlers.error()});

    auto finalTodo = todoRepository_.findByID(created->id);
    if (!finalTodo)
        return std::unexpected(TodoCreationError{finalTodo.error()});
    if (!*finalTodo) {
        return std::unexpected(TodoCreationError{db::error::StorageError{
            .code = db::error::StorageErrorCode::Corrupted,
            .operation = "read created todo after handlers",
            .diagnostic = "created todo disappeared",
        }});
    }

    return CreateTodoOutput{
        .todo = std::move(**finalTodo),
        .handlers = std::move(*handlers),
    };
}

repository::GetTodoPageResult TodoUseCases::getPage(std::int32_t offset, std::int32_t limit) {
    return todoRepository_.getPage(offset, limit);
}

} // namespace todod::use_cases
