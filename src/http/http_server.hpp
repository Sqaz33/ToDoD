#pragma once

#include "crow_all.h"
#include "application/use_cases/handler_use_cases.hpp"
#include "application/use_cases/todo_use_cases.hpp"

namespace todod::http {

class HttpServer {
public:
    HttpServer(
        use_cases::TodoUseCases& todoUseCases,
        use_cases::HandlerUseCases& handlerUseCases,
        int port,
        int threads = 2);

    void run();

private:
    int port_;
    int threads_;
    crow::SimpleApp crowApp_;
};

} // namespace todod::http
