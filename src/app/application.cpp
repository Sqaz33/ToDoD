#include "application.hpp"

#include "application/ports/repository/script_repository.hpp"
#include "application/ports/repository/todo_repository.hpp"
#include "application/ports/scripting/script_engine.hpp"
#include "application/service/script_service.hpp"
#include "application/use_cases/handler_use_cases.hpp"
#include "application/use_cases/todo_use_cases.hpp"
#include "http/http_server.hpp"
#include "infrastructure/database/database.hpp"

#include <memory>

namespace todod::app {

TododApp::TododApp() {
    auto database = std::make_shared<db::DataBase>("./data.db");
    repository::TodoRepository todoRepository{database};
    repository::ScriptRepository scriptRepository{database};
    scripting::engine::ScriptEngine scriptEngine;
    service::HandlerScriptService handlerService{todoRepository, scriptRepository, *database,
                                                 scriptEngine};
    use_cases::TodoUseCases todoUseCases{todoRepository, handlerService};
    use_cases::HandlerUseCases handlerUseCases{scriptRepository};
    http::HttpServer server{todoUseCases, handlerUseCases, 8000, 2};
    server.run();
}

} // namespace todod::app
