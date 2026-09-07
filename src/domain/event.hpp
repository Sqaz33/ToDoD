#pragma once

#include <set>

#define TODO_EVENTS(X)                                                                             \
    X(ADDED_TODO)                                                                                  \
    X(UPDATED_TODO)                                                                                \
    X(DELETED_TODO)

namespace todod::domain {

enum class TodoEvent : int {
#define X(event) event,
    TODO_EVENTS(X)
#undef X
};

extern const std::set<TodoEvent> ALL_EVENTS;

} // namespace todod::domain
