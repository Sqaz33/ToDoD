#include "sqlite_statement_guard.hpp"

namespace todod::db::guard {

StatementResetGuard::StatementResetGuard(SQLite::Statement& stm) noexcept : stm_(stm) {}

StatementResetGuard::~StatementResetGuard() {
    try {
        stm_.reset();
    } catch (...) {}

    try {
        stm_.clearBindings();
    } catch (...) {}
}

} // namespace todod::db::guard
