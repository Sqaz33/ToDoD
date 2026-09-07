#pragma once

#include "application/service/script_service.hpp"
#include "crow_all.h"
#include "domain/script.hpp"
#include "domain/todo.hpp"

#include <string>

namespace todod::http {

crow::json::wvalue todoToJson(const domain::TodoTask& todo);
crow::json::wvalue handlerToJson(const domain::HandlerScript& handler);
crow::json::wvalue
handlersReportToJson(const std::vector<service::HandlerExecutionResult>& handlers);
crow::response errorResponse(int status, std::string code, std::string message,
                             std::string field = {});
crow::response notImplementedResponse();

} // namespace todod::http
