#include "database.hpp"

namespace todod::db {

DataBase::DataBase(const std::string& path) : 
    db_(
        path, 
        SQLite::OPEN_READWRITE | 
        SQLite::OPEN_CREATE
    ) 
{
    db_.exec("PRAGMA foreign_keys = ON");
}

SQLite::Database& DataBase::connection() noexcept {
    return db_;
}

} // namespace todod::db
