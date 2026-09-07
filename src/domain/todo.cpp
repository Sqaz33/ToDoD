#include "todo.hpp"

#include "limits.hpp"
#include "time/iso8601_helper.hpp"

#include <limits>
#include <utility>

namespace todod::domain {

TodoDefinitionResult TodoDefinition::create(const TodoInput& input) {
    if (input.title.empty()) {
        return std::unexpected(TodoValidationError::EmptyTitle);
    }
    if (input.title.size() > limits::TodoTitleMaxBytes) {
        return std::unexpected(TodoValidationError::TitleTooLong);
    }
    if (input.description.size() > limits::TodoDescriptionMaxBytes) {
        return std::unexpected(TodoValidationError::DescriptionTooLong);
    }
    if (input.priority < 0) {
        return std::unexpected(TodoValidationError::NegativePriority);
    }
    if (static_cast<std::uint64_t>(input.priority) > std::numeric_limits<std::uint32_t>::max()) {
        return std::unexpected(TodoValidationError::PriorityTooLarge);
    }
    if (!helpers::isValidIso8601(input.completedAt)) {
        return std::unexpected(TodoValidationError::InvalidCompletedAtFormat);
    }

    return TodoDefinition(input.title, input.description,
                          static_cast<std::uint32_t>(input.priority),
                          helpers::iso8601ToTimePoint(input.completedAt), input.completed);
}

TodoDefinition TodoDefinition::rehydrate(std::string title, std::string description,
                                         std::uint32_t priority,
                                         std::chrono::system_clock::time_point completedAt,
                                         bool completed) {
    return TodoDefinition(std::move(title), std::move(description), priority, completedAt,
                          completed);
}

TodoDefinition::TodoDefinition(std::string title, std::string description, std::uint32_t priority,
                               std::chrono::system_clock::time_point completedAt, bool completed)
    : title_(std::move(title)), description_(std::move(description)), priority_(priority),
      completedAt_(completedAt), completed_(completed) {}

const std::string& TodoDefinition::title() const noexcept {
    return title_;
}
const std::string& TodoDefinition::description() const noexcept {
    return description_;
}
std::uint32_t TodoDefinition::priority() const noexcept {
    return priority_;
}
const std::chrono::system_clock::time_point& TodoDefinition::completedAt() const noexcept {
    return completedAt_;
}
bool TodoDefinition::completed() const noexcept {
    return completed_;
}

} // namespace todod::domain
