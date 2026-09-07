#pragma once

#include <mutex>
#include <type_traits>
#include <functional>

#include <SQLiteCpp/SQLiteCpp.h>

#include "db_access.hpp" 

namespace todod::db {
    
class DataBase { 
public:
    explicit DataBase(const std::string& path);

public:
    SQLite::Database& connection() noexcept;
    
    template <class F>
    auto access(F&& f) {
        std::lock_guard<std::mutex> lk{mutex};
        DBSession session;
        return std::invoke(std::forward<F>(f), session);
    }

    template <class F>
    auto transaction(F&& f) {
        std::lock_guard<std::mutex> lk{mutex};

        DBSession session;

        db_.exec("BEGIN");

        try {
            if constexpr(std::is_void_v<std::invoke_result<F>>>) {
                bool commit = true;
                std::invoke(std::forward<F>(f), session, &commit);
                if (commit) {
                    db_.exec("COMMIT");
                } else {
                    db_.exec("ROLLBACK");
                }
            } else {
                bool commit = true;
                auto result = std::invoke(std::forward<F>(f), session, &commit);
                if (commit) {
                    db_.exec("COMMIT");
                } else {
                    db_.exec("ROLLBACK");
                }
                return result;
            }
        } catch (...) {
            db_.exec("ROLLBACK");
            throw;
        }
    }

private:
    std::mutex mutex_;
    SQLite::Database db_;
};

} // namespace todod::db
