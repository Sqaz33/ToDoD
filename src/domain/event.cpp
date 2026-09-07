#include "event.hpp"

namespace todod::domain {

const std::set<TodoEvent> ALL_EVENTS{
#define X(event) TodoEvent::event,
    TODO_EVENTS(X)
#undef X
};

} // namespace todod::domain
