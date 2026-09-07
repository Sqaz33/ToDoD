#pragma once

#include "domain/script.hpp"
#include "domain/todo.hpp"
#include "storage_error.hpp"

#include <cstdint>
#include <expected>
#include <optional>
#include <vector>

namespace todod::repository {

using MaybeError = std::optional<db::error::StorageError>;
using TaskOrError = std::expected<domain::TodoTask, db::error::StorageError>;
using FindTodoResult = std::expected<std::optional<domain::TodoTask>, db::error::StorageError>;
using GetTodoPageResult = std::expected<domain::TodoPage, db::error::StorageError>;
using GetCountResult = std::expected<std::int32_t, db::error::StorageError>;
using UpdateTodoResult = std::expected<bool, db::error::StorageError>;
using HandlerScriptOrError = std::expected<domain::HandlerScript, db::error::StorageError>;
using GetHandlersResult =
    std::expected<std::vector<domain::HandlerScript>, db::error::StorageError>;
using FindHandlerScriptByEventResult = GetHandlersResult;

} // namespace todod::repository
