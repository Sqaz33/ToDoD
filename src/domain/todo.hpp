#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <string>

#include "ids.hpp"
#include "page.hpp"

namespace todod::domain {

struct TodoInput {
    std::string title;
    std::string description;
    std::int64_t priority;
    std::string completedAt;
    bool completed;
};

enum class TodoValidationError {
    EmptyTitle,
    TitleTooLong,
    DescriptionTooLong,
    NegativePriority,
    PriorityTooLarge,
    InvalidCompletedAtFormat,
};

class TodoDefinition;
using TodoDefinitionResult = std::expected<TodoDefinition, TodoValidationError>;

class TodoDefinition {
public:
    static TodoDefinitionResult create(const TodoInput& input);
    static TodoDefinition rehydrate(
        std::string title,
        std::string description,
        std::uint32_t priority,
        std::chrono::system_clock::time_point completedAt,
        bool completed);

    const std::string& title() const noexcept;
    const std::string& description() const noexcept;
    std::uint32_t priority() const noexcept;
    const std::chrono::system_clock::time_point& completedAt() const noexcept;
    bool completed() const noexcept;

private:
    TodoDefinition(
        std::string title,
        std::string description,
        std::uint32_t priority,
        std::chrono::system_clock::time_point completedAt,
        bool completed);

    std::string title_;
    std::string description_;
    std::uint32_t priority_;
    std::chrono::system_clock::time_point completedAt_;
    bool completed_;
};

struct TodoTask {
    TodoId id;
    TodoDefinition def;
};

using TodoPage = Page<TodoTask>;

} // namespace todod::domain
