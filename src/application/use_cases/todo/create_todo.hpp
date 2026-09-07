#pragma once

#include <variant>

#include "domain/todo.hpp"
#include "application/ports/repository/storage_error.hpp"
#include "application/service/script_service.hpp"

namespace todod::use_cases::create_todo {

using TodoCreationError =  std::variant<
    domain::TodoValidationError, 
    db::error::StorageError
>;

struct CreateTodoOutput {
    domain::TodoTask todo;
    service::RunHandlersResult hanldersResult;
};

using TodoCreationResult = std::expected<CreateTodoOutput, TodoCreationError>;

class CreateTodo {
public:
    ...
    TodoCreationResult run(const domain::TodoInput& input);
};

TodoCreationResult createTodo(const domain::TodoInput& input);

} // namespace todod::use_cases::create_todo