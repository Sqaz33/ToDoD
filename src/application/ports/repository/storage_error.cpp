#include "storage_error.hpp"

#include "sqlite3.h"

#include <SQLiteCpp/SQLiteCpp.h>

namespace todod::db::error {

StorageError StorageError::create(const std::string& operation,
                                  const SQLite::Exception& exception) {
    return {.code = mapSqliteError(exception.getErrorCode()),
            .operation = operation,
            .diagnostic = exception.what()};
}

StorageErrorCode mapSqliteError(int errorCode) noexcept {
    switch (errorCode) {
    case SQLITE_BUSY:
    case SQLITE_LOCKED:
        return StorageErrorCode::Busy;
    case SQLITE_CONSTRAINT:
        return StorageErrorCode::ConstraintViolation;
    case SQLITE_CORRUPT:
    case SQLITE_NOTADB:
        return StorageErrorCode::Corrupted;
    case SQLITE_IOERR:
    case SQLITE_CANTOPEN:
    case SQLITE_FULL:
        return StorageErrorCode::IoError;
    default:
        return StorageErrorCode::Unknown;
    }
}

} // namespace todod::db::error
