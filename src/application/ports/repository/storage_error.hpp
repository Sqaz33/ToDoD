#pragma once

#include <string>

#include <SQLiteCpp/SQLiteCpp.h>

namespace todod::db::error {

enum class StorageErrorCode {
    Busy,
    ConstraintViolation,
    Corrupted,
    IoError,
    Unknown
};

struct StorageError {
    static StorageError create(
        const std::string& operation,
        const SQLite::Exception& exception);
    StorageErrorCode code;
    std::string operation;
    std::string diagnostic;
};

StorageErrorCode mapSqliteError(int errorCode) noexcept;
    
} // namespace todod::db::error
