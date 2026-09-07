#pragma once

#include "application/use_cases/todo_use_cases.hpp"
#include "crow_all.h"

namespace todod::http::routes {

void registerTodoRoutes(crow::SimpleApp& app, use_cases::TodoUseCases& useCases);

} // namespace todod::http::routes
