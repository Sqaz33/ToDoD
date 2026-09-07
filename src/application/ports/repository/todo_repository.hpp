#pragma once

#include <memory>

#include "infrastructure/database/database.hpp"
#include "repository_results.hpp"

namespace todod::repository {

class TodoRepository {
public:
    explicit TodoRepository(std::shared_ptr<db::DataBase> db);

    TaskOrError create(const domain::TodoDefinition& definition);
    TaskOrError create(const domain::TodoDefinition& definition, db::DBAccess&);

    FindTodoResult findByID(domain::TodoId id);
    FindTodoResult findByID(domain::TodoId id, db::DBAccess&);

    GetTodoPageResult getPage(std::int32_t offset, std::int32_t limit);
    GetTodoPageResult getPage(std::int32_t offset, std::int32_t limit, db::DBAccess&);

    UpdateTodoResult setCompleteStatus(domain::TodoId id, bool status, db::DBAccess&);
    UpdateTodoResult setPriority(domain::TodoId id, int priority, db::DBAccess&);

private:
    GetCountResult getCount(db::DBAccess&);

    std::shared_ptr<db::DataBase> db_;
    SQLite::Statement insertionQuery_;
    SQLite::Statement findByIdQuery_;
    SQLite::Statement setCompleteQuery_;
    SQLite::Statement setPriorityQuery_;
    SQLite::Statement getPageQuery_;
    SQLite::Statement getCountQuery_;
};

} // namespace todod::repository
