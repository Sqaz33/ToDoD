#pragma once

#include "db_access.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <functional>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>

namespace todod::db {

class DataBase {
  public:
    explicit DataBase(const std::string& path);

    SQLite::Database& connection() noexcept;

    template <class F> auto access(F&& function) {
        std::lock_guard<std::mutex> lock{mutex_};
        DBAccess access;
        return std::invoke(std::forward<F>(function), access);
    }

    template <class F> auto transaction(F&& function) {
        std::lock_guard<std::mutex> lock{mutex_};
        DBAccess access;
        SQLite::Transaction transaction{db_};
        bool commit = true;

        using Result = std::invoke_result_t<F, DBAccess&, bool*>;
        if constexpr (std::is_void_v<Result>) {
            std::invoke(std::forward<F>(function), access, &commit);
            if (commit)
                transaction.commit();
        } else {
            auto result = std::invoke(std::forward<F>(function), access, &commit);
            if (commit)
                transaction.commit();
            return result;
        }
    }

  private:
    std::mutex mutex_;
    SQLite::Database db_;
};

} // namespace todod::db
