#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

namespace todod::db::guard {

class StatementResetGuard {
  public:
    StatementResetGuard(SQLite::Statement& stm) noexcept;
    ~StatementResetGuard();

  private:
    SQLite::Statement& stm_;
};

} // namespace todod::db::guard
