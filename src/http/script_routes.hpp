#pragma once

#include "crow_all.h"
#include "application/use_cases/handler_use_cases.hpp"

namespace todod::http::routes {

void registerScriptRoutes(crow::SimpleApp& app, use_cases::HandlerUseCases& useCases);

} // namespace todod::http::routes
