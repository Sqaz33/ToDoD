#pragma once

#include <cstdint>
#include <expected>
#include <string>

#include "event.hpp"
#include "ids.hpp"
#include "page.hpp"

namespace todod::domain {

struct HandlerScriptInput {
    std::string name;
    std::string source;
    std::int64_t event;
    bool enabled;
};

enum class HandlerScriptValidationError {
    EmptyName,
    NameTooLong,
    EmptySource,
    SourceTooLong,
    NegativeEvent,
    UnknownEvent,
};

class HandlerScriptDefinition;
using HandlerScriptResult = std::expected<HandlerScriptDefinition, HandlerScriptValidationError>;

class HandlerScriptDefinition {
public:
    static HandlerScriptResult create(const HandlerScriptInput& input);
    static HandlerScriptDefinition rehydrate(
        std::string name,
        std::string source,
        TodoEvent event,
        bool enabled);

    const std::string& name() const noexcept;
    const std::string& source() const noexcept;
    TodoEvent event() const noexcept;
    bool enabled() const noexcept;

private:
    HandlerScriptDefinition(
        std::string name,
        std::string source,
        TodoEvent event,
        bool enabled);

    std::string name_;
    std::string source_;
    TodoEvent event_;
    bool enabled_;
};

struct HandlerScript {
    HandlerScriptId id;
    HandlerScriptDefinition def;
};

using HandlerScriptPage = Page<HandlerScript>;

} // namespace todod::domain
