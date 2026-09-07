#include "http_server.hpp"

#include "docs_routes.hpp"
#include "script_routes.hpp"
#include "todo_routes.hpp"

namespace todod::http {

HttpServer::HttpServer(use_cases::TodoUseCases& todoUseCases,
                       use_cases::HandlerUseCases& handlerUseCases, int port, int threads)
    : port_(port), threads_(threads) {
    routes::registerTodoRoutes(crowApp_, todoUseCases);
    routes::registerScriptRoutes(crowApp_, handlerUseCases);
    routes::registerDocsRoutes(crowApp_);
}

void HttpServer::run() {
    crowApp_.bindaddr("127.0.0.1").port(port_).concurrency(threads_).run();
}

} // namespace todod::http
