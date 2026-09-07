#include "script_repository.hpp"

#include <format>
#include <utility>

#include "infrastructure/database/sqlite_statement_guard.hpp"

namespace {

const std::string ID_COLUMN = "id";
const std::string NAME_COLUMN = "name";
const std::string SOURCE_COLUMN = "source";
const std::string EVENT_COLUMN = "event";
const std::string ENABLED_COLUMN = "enabled";
const std::string TABLE_NAME = "scripts";

todod::HandlerScript readScript(SQLite::Statement& query) {
    return todod::domain::HandlerScript{
        .id = {query.getColumn(0).getInt64()},
        .def = todod::domain::HandlerScriptDefinition::rehydrate(
            query.getColumn(1).getString(),
            query.getColumn(2).getString(),
            static_cast<todod::domain::TodoEvent>(query.getColumn(3).getInt()),
            query.getColumn(4).getInt() != 0
        )
    };
}

} // namespace

namespace todod::repository {

ScriptRepository::ScriptRepository(std::shared_ptr<db::DataBase> db) :
    db_(std::move(db)),
    insertionQuery_(db_->connection(), std::format(
        "INSERT INTO {} ({}, {}, {}, {}) VALUES (?, ?, ?, ?)",
        TABLE_NAME,
        NAME_COLUMN,
        SOURCE_COLUMN,
        EVENT_COLUMN,
        ENABLED_COLUMN))
    , getAllQuery_(db_->connection(), std::format(
        "SELECT {}, {}, {}, {}, {} FROM {}",
        ID_COLUMN,
        NAME_COLUMN,
        SOURCE_COLUMN,
        EVENT_COLUMN,
        ENABLED_COLUMN,
        TABLE_NAME))
    , updateQuery_(db_->connection(), std::format(
        R"(
            UPDATE {}
            SET {} = ?,
                {} = ?,
                {} = ?,
                {} = ?
            WHERE {} = ?
        )",
        TABLE_NAME,
        NAME_COLUMN,
        SOURCE_COLUMN,
        EVENT_COLUMN,
        ENABLED_COLUMN,
        ID_COLUMN))
    , findByIdQuery_(db_->connection(), std::format(
        "SELECT {}, {}, {}, {}, {} FROM {} WHERE {} = ?",
        ID_COLUMN,
        NAME_COLUMN,
        SOURCE_COLUMN,
        EVENT_COLUMN,
        ENABLED_COLUMN,
        TABLE_NAME,
        ID_COLUMN))
    , removeQuery_(db_->connection(), std::format(
        "DELETE FROM {} WHERE {} = ?",
        TABLE_NAME,
        ID_COLUMN))
    , findByEventQuery_(db_->connection(), std::format(
        "SELECT * FROM {} WHERE {} = ?",
        TABLE_NAME,
        EVENT_COLUMN
    ))
{}


HandlerScriptOrError ScriptRepository::create(const domain::HandlerScriptDefinition& def) {
    return db_->access([&](db::DBAccess& access) { return create(def, access); });
}

HandlerScriptOrError ScriptRepository::create(const domain::HandlerScriptDefinition& def, db::DBAccess&) {
    auto err = insert_(
        def.name(), 
        def.source(), 
        def.event(), 
        def.enabled());

    if (err) {
        return std::unexpect(err.value());
    }

    return domain::HandlerScript{{db_->connection().getLastInsertRowid()}, def};
}

// std::vector<HandlerScript> ScriptRepository::getAll() {
//     std::lock_guard<std::mutex> lk(db_->mutex());

//     std::vector<HandlerScript> scripts;

//     while (getAllQuery_.executeStep()) {
//         scripts.emplace_back(readScript(getAllQuery_));
//     }

//     getAllQuery_.reset();
//     getAllQuery_.clearBindings();
//     return scripts;
// }

// bool ScriptRepository::updateTodo(const HandlerScript& script) {
//     std::lock_guard<std::mutex> lk(db_->mutex());

//     updateQuery_.bind(1, script.name);
//     updateQuery_.bind(2, script.source);
//     updateQuery_.bind(3, static_cast<int>(script.event));
//     updateQuery_.bind(4, script.enabled);
//     updateQuery_.bind(5, script.id);
//     updateQuery_.exec();

//     const bool updated = db_->connection().getChanges() != 0;

//     updateQuery_.reset();
//     updateQuery_.clearBindings();
//     return updated;
// }

// std::optional<HandlerScript> ScriptRepository::findByID(std::int64_t id) {
//     std::lock_guard<std::mutex> lk(db_->mutex());

//     findByIdQuery_.bind(1, id);

//     if (!findByIdQuery_.executeStep()) {
//         findByIdQuery_.reset();
//         findByIdQuery_.clearBindings();
//         return std::nullopt;
//     }

//     auto script = readScript(findByIdQuery_);

//     findByIdQuery_.reset();
//     findByIdQuery_.clearBindings();
//     return script;
// }

// bool ScriptRepository::removeTodo(std::int64_t id) {
//     std::lock_guard<std::mutex> lk(db_->mutex());
    
//     removeQuery_.bind(1, id);
//     removeQuery_.exec();

//     const bool removed = db_->connection().getChanges() != 0;

//     removeQuery_.reset();
//     removeQuery_.clearBindings();
//     return removed;
// }

FindHandlerScriptByEventResult ScriptRepository::findByEvent(domain::TodoEvent event) {
    return db_->access([&](db::DBAccess& access) { return findByEvent(event, access); });
}

FindHandlerScriptByEventResult ScriptRepository::findByEvent(domain::TodoEvent event, db::DBAccess&) {
    db::guard::StatementResetGuard guard { findByEventQuery_ };

    try {
        std::vector<domain::HandlerScript> scripts;
        findByEventQuery_.bind(1, static_cast<int>(event));

        while (findByEventQuery_.executeStep()) {
            scripts.emplace_back(readScript(findByEventQuery_));
        }
        
        return scripts;
    } catch (const SQLite::Exception& e) {
        return std::unexpect(db::error::StorageError::create("find handler script by event", e));
    }
}

} // namespace todod::repository