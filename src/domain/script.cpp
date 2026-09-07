#include "script.hpp"

#include <limits>
#include <utility>

#include "limits.hpp"

namespace todod::domain {

HandlerScriptResult HandlerScriptDefinition::create(const HandlerScriptInput& input) {
    if (input.name.empty()) {
        return std::unexpected(HandlerScriptValidationError::EmptyName);
    }
    if (input.name.size() > limits::HandlerNameMaxBytes) {
        return std::unexpected(HandlerScriptValidationError::NameTooLong);
    }
    if (input.source.empty()) {
        return std::unexpected(HandlerScriptValidationError::EmptySource);
    }
    if (input.source.size() > limits::HandlerSourceMaxBytes) {
        return std::unexpected(HandlerScriptValidationError::SourceTooLong);
    }
    if (input.event < 0) {
        return std::unexpected(HandlerScriptValidationError::NegativeEvent);
    }
    if (input.event > std::numeric_limits<int>::max()) {
        return std::unexpected(HandlerScriptValidationError::UnknownEvent);
    }

    const auto event = static_cast<TodoEvent>(input.event);
    if (!ALL_EVENTS.contains(event)) {
        return std::unexpected(HandlerScriptValidationError::UnknownEvent);
    }

    return HandlerScriptDefinition(input.name, input.source, event, input.enabled);
}

HandlerScriptDefinition HandlerScriptDefinition::rehydrate(
    std::string name,
    std::string source,
    TodoEvent event,
    bool enabled) {
    return HandlerScriptDefinition(std::move(name), std::move(source), event, enabled);
}

HandlerScriptDefinition::HandlerScriptDefinition(
    std::string name,
    std::string source,
    TodoEvent event,
    bool enabled)
    : name_(std::move(name)), source_(std::move(source)), event_(event), enabled_(enabled) {}

const std::string& HandlerScriptDefinition::name() const noexcept { return name_; }
const std::string& HandlerScriptDefinition::source() const noexcept { return source_; }
TodoEvent HandlerScriptDefinition::event() const noexcept { return event_; }
bool HandlerScriptDefinition::enabled() const noexcept { return enabled_; }

} // namespace todod::domain
