#pragma once

#include "application/ports/repository/todo_repository.hpp"
#include "application/service/script_service.hpp"

#include <expected>
#include <variant>

namespace todod::use_cases {

using TodoCreationError = std::variant<domain::TodoValidationError, db::error::StorageError>;

struct CreateTodoOutput {
    domain::TodoTask todo;
    std::vector<service::HandlerExecutionResult> handlers;
};

using TodoCreationResult = std::expected<CreateTodoOutput, TodoCreationError>;

class TodoUseCases {
  public:
    TodoUseCases(repository::TodoRepository& todoRepository,
                 service::HandlerScriptService& handlerService);

    TodoCreationResult createTodo(const domain::TodoInput& input);
    repository::GetTodoPageResult getPage(std::int32_t offset, std::int32_t limit);

  private:
    repository::TodoRepository& todoRepository_;
    service::HandlerScriptService& handlerService_;
};

} // namespace todod::use_cases
