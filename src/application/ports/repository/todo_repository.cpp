#include "todo_repository.hpp"

#include "infrastructure/database/sqlite_statement_guard.hpp"
#include "time/timestamp_helpers.hpp"

#include <format>
#include <utility>

namespace {

constexpr auto TABLE_NAME = "todos";

todod::domain::TodoTask readTask(SQLite::Statement& query) {
    return {
        .id = {query.getColumn(0).getInt64()},
        .def = todod::domain::TodoDefinition::rehydrate(
            query.getColumn(1).getString(), query.getColumn(2).getString(),
            query.getColumn(3).getUInt(),
            todod::helpers::fromTimestamp(query.getColumn(4).getInt64()),
            query.getColumn(5).getInt() != 0),
    };
}

} // namespace

namespace todod::repository {

TodoRepository::TodoRepository(std::shared_ptr<db::DataBase> db)
    : db_(std::move(db)),
      insertionQuery_(db_->connection(), "INSERT INTO todos (title, description, priority, "
                                         "completed_at, completed) VALUES (?, ?, ?, ?, ?)"),
      findByIdQuery_(db_->connection(), "SELECT id, title, description, priority, completed_at, "
                                        "completed FROM todos WHERE id = ?"),
      setCompleteQuery_(db_->connection(), "UPDATE todos SET completed = ? WHERE id = ?"),
      setPriorityQuery_(db_->connection(), "UPDATE todos SET priority = ? WHERE id = ?"),
      getPageQuery_(db_->connection(), "SELECT id, title, description, priority, completed_at, "
                                       "completed FROM todos ORDER BY id DESC LIMIT ? OFFSET ?"),
      getCountQuery_(db_->connection(), "SELECT COUNT(*) FROM todos") {}

TaskOrError TodoRepository::create(const domain::TodoDefinition& definition) {
    return db_->access([&](db::DBAccess& access) { return create(definition, access); });
}

TaskOrError TodoRepository::create(const domain::TodoDefinition& definition, db::DBAccess&) {
    db::guard::StatementResetGuard guard{insertionQuery_};
    try {
        insertionQuery_.bind(1, definition.title());
        insertionQuery_.bind(2, definition.description());
        insertionQuery_.bind(3, definition.priority());
        insertionQuery_.bind(4, helpers::toTimestamp(definition.completedAt()));
        insertionQuery_.bind(5, definition.completed());
        insertionQuery_.exec();

        return domain::TodoTask{
            .id = {db_->connection().getLastInsertRowid()},
            .def = definition,
        };
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("create todo", exception));
    }
}

FindTodoResult TodoRepository::findByID(domain::TodoId id) {
    return db_->access([&](db::DBAccess& access) { return findByID(id, access); });
}

FindTodoResult TodoRepository::findByID(domain::TodoId id, db::DBAccess&) {
    db::guard::StatementResetGuard guard{findByIdQuery_};
    try {
        findByIdQuery_.bind(1, id.id);
        if (!findByIdQuery_.executeStep()) {
            return std::optional<domain::TodoTask>{};
        }
        return std::optional<domain::TodoTask>{readTask(findByIdQuery_)};
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("find todo by id", exception));
    }
}

GetTodoPageResult TodoRepository::getPage(std::int32_t offset, std::int32_t limit) {
    return db_->access([&](db::DBAccess& access) { return getPage(offset, limit, access); });
}

GetTodoPageResult TodoRepository::getPage(std::int32_t offset, std::int32_t limit,
                                          db::DBAccess& access) {
    db::guard::StatementResetGuard guard{getPageQuery_};
    try {
        auto count = getCount(access);
        if (!count)
            return std::unexpected(count.error());

        domain::TodoPage page{
            .items = {},
            .meta = {.total = *count, .offset = offset, .limit = limit},
        };

        getPageQuery_.bind(1, limit);
        getPageQuery_.bind(2, offset);
        while (getPageQuery_.executeStep()) {
            page.items.push_back(readTask(getPageQuery_));
        }
        return page;
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("get todo page", exception));
    }
}

UpdateTodoResult TodoRepository::setCompleteStatus(domain::TodoId id, bool status, db::DBAccess&) {
    db::guard::StatementResetGuard guard{setCompleteQuery_};
    try {
        setCompleteQuery_.bind(1, status);
        setCompleteQuery_.bind(2, id.id);
        setCompleteQuery_.exec();
        return db_->connection().getChanges() != 0;
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(
            db::error::StorageError::create("set todo completed status", exception));
    }
}

UpdateTodoResult TodoRepository::setPriority(domain::TodoId id, int priority, db::DBAccess&) {
    db::guard::StatementResetGuard guard{setPriorityQuery_};
    try {
        setPriorityQuery_.bind(1, priority);
        setPriorityQuery_.bind(2, id.id);
        setPriorityQuery_.exec();
        return db_->connection().getChanges() != 0;
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("set todo priority", exception));
    }
}

GetCountResult TodoRepository::getCount(db::DBAccess&) {
    db::guard::StatementResetGuard guard{getCountQuery_};
    try {
        getCountQuery_.executeStep();
        return getCountQuery_.getColumn(0).getInt();
    } catch (const SQLite::Exception& exception) {
        return std::unexpected(db::error::StorageError::create("count todos", exception));
    }
}

} // namespace todod::repository
