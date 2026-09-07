#pragma once

#include "application/use_cases/handler_use_cases.hpp"
#include "crow_all.h"

namespace todod::http::routes {

void registerScriptRoutes(crow::SimpleApp& app, use_cases::HandlerUseCases& useCases);

} // namespace todod::http::routes
