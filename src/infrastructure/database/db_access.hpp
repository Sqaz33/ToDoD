#pragma once

class DataBase;

namespace todod::db {

class DBAccess {
  private:
    explicit DBAccess() = default;
    DBAccess& operator=(const DBAccess&) = delete;
    DBAccess& operator=(DBAccess&&) = delete;
    DBAccess(const DBAccess&) = delete;
    DBAccess(DBAccess&&) = delete;
    friend class DataBase;
};

} // namespace todod::db
