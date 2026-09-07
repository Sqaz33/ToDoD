#include "script_repository.hpp"

#include <utility>

#include "infrastructure/database/sqlite_statement_guard.hpp"

namespace {

todod::domain::HandlerScript readScript(SQLite::Statement& query) {
    return {
        .id = {query.getColumn(0).getInt64()},
        .def = todod::domain::HandlerScriptDefinition::rehydrate(
            query.getColumn(1).getString(),
            query.getColumn(2).getString(),
            static_cast<todod::domain::TodoEvent>(query.getColumn(3).getInt()),
            query.getColumn(4).getInt() != 0),
    };
}

} // namespace

namespace todod::repository {

ScriptRepository::ScriptRepository(std::shared_ptr<db::DataBase> db)
    : db_(std::move(db)),
      insertionQuery_(db_->connection(),
          "INSERT INTO scripts (name, source, event, enabled) VALUES (?, ?, ?, ?)"),
      getAllQuery_(db_->connection(),
          "SELECT id, name, source, event, enabled FROM scripts ORDER BY id"),
      findByEventQuery_(db_->connection(),
          "SELECT id, name, source, event, enabled FROM scripts WHERE event = ? AND enabled = 1 ORDER BY id") {}

HandlerScriptOrError ScriptRepository::create(const domain::HandlerScriptDefinition& definition) {
    return db_->access([&](db::DBAccess& access) { return create(definition, access); });
}

HandlerScriptOrError ScriptRepository::create(
    const domain::HandlerScriptDefinition& definition, db::DBAccess&) {
    db::guard::StatementResetGuard guard{insertionQuery_};
    try {
        insertionQuery_.bind(1, definition.name());
        insertionQuery_.bind(2, definition.source());
        insertionQuery_.bind(3, static_cast<int>(definition.event()));
        insertionQuery_.bind(4, definition.enabled());
        insertionQuery_.exec();
        return domain::HandlerScript{
            .id = {db_->connection().getLastInsertRowid()},
            .def = definition,
        };
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("create handler", exception));
    }
}

GetHandlersResult ScriptRepository::getAll() {
    return db_->access([&](db::DBAccess& access) { return getAll(access); });
}

GetHandlersResult ScriptRepository::getAll(db::DBAccess&) {
    db::guard::StatementResetGuard guard{getAllQuery_};
    try {
        std::vector<domain::HandlerScript> handlers;
        while (getAllQuery_.executeStep()) handlers.push_back(readScript(getAllQuery_));
        return handlers;
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("get handlers", exception));
    }
}

FindHandlerScriptByEventResult ScriptRepository::findByEvent(domain::TodoEvent event) {
    return db_->access([&](db::DBAccess& access) { return findByEvent(event, access); });
}

FindHandlerScriptByEventResult ScriptRepository::findByEvent(
    domain::TodoEvent event, db::DBAccess&) {
    db::guard::StatementResetGuard guard{findByEventQuery_};
    try {
        std::vector<domain::HandlerScript> handlers;
        findByEventQuery_.bind(1, static_cast<int>(event));
        while (findByEventQuery_.executeStep()) handlers.push_back(readScript(findByEventQuery_));
        return handlers;
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("find handlers by event", exception));
    }
}

} // namespace todod::repository
