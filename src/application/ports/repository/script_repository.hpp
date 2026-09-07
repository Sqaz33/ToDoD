#pragma once

#include "infrastructure/database/database.hpp"
#include "repository_results.hpp"

#include <memory>

namespace todod::repository {

class ScriptRepository {
  public:
    explicit ScriptRepository(std::shared_ptr<db::DataBase> db);

    HandlerScriptOrError create(const domain::HandlerScriptDefinition& definition);
    HandlerScriptOrError create(const domain::HandlerScriptDefinition& definition, db::DBAccess&);
    GetHandlersResult getAll();
    GetHandlersResult getAll(db::DBAccess&);
    FindHandlerScriptByEventResult findByEvent(domain::TodoEvent event);
    FindHandlerScriptByEventResult findByEvent(domain::TodoEvent event, db::DBAccess&);

  private:
    std::shared_ptr<db::DataBase> db_;
    SQLite::Statement insertionQuery_;
    SQLite::Statement getAllQuery_;
    SQLite::Statement findByEventQuery_;
};

} // namespace todod::repository
