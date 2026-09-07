#pragma once

#include <cstdint>

namespace todod::domain {

struct TodoId {
    std::int64_t id;

    friend bool operator==(const TodoId&, const TodoId&) = default;
};

struct HandlerScriptId {
    std::int64_t id;

    friend bool operator==(const HandlerScriptId&, const HandlerScriptId&) = default;
};

} // namespace todod::domain
