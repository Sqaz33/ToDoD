#include "todo_routes.hpp"

#include "json_serializer.hpp"

#include <algorithm>
#include <charconv>
#include <limits>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <variant>

namespace todod::http::routes {

namespace {

constexpr std::size_t MaxRequestBytes = 128 * 1024;

bool hasJsonContentType(const crow::request& request) {
    const auto contentType = request.get_header_value("Content-Type");
    return contentType.starts_with("application/json");
}

std::optional<std::int32_t> parseInt32(std::string_view text) {
    std::int64_t value{};
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value < 0 ||
        value > std::numeric_limits<std::int32_t>::max())
        return std::nullopt;
    return static_cast<std::int32_t>(value);
}

std::optional<std::int64_t> jsonInteger(const crow::json::rvalue& value) {
    if (value.t() != crow::json::type::Number)
        return std::nullopt;
    if (value.nt() == crow::json::num_type::Signed_integer)
        return value.i();
    if (value.nt() == crow::json::num_type::Unsigned_integer &&
        value.u() <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return static_cast<std::int64_t>(value.u());
    }
    return std::nullopt;
}

std::optional<domain::TodoInput> parseTodoInput(const crow::json::rvalue& json) {
    if (!json.has("title") || !json.has("description") || !json.has("priority") ||
        !json.has("completedAt") || !json.has("completed"))
        return std::nullopt;
    if (json["title"].t() != crow::json::type::String ||
        json["description"].t() != crow::json::type::String ||
        json["completedAt"].t() != crow::json::type::String)
        return std::nullopt;
    const auto completedType = json["completed"].t();
    if (completedType != crow::json::type::True && completedType != crow::json::type::False)
        return std::nullopt;
    auto priority = jsonInteger(json["priority"]);
    if (!priority)
        return std::nullopt;

    return domain::TodoInput{
        .title = json["title"].s(),
        .description = json["description"].s(),
        .priority = *priority,
        .completedAt = json["completedAt"].s(),
        .completed = json["completed"].b(),
    };
}

} // namespace

void registerTodoRoutes(crow::SimpleApp& app, use_cases::TodoUseCases& useCases) {
    CROW_ROUTE(app, "/api/page")
        .methods(crow::HTTPMethod::GET)([&useCases](const crow::request& request) {
            const char* offsetValue = request.url_params.get("offset");
            const char* limitValue = request.url_params.get("limit");
            if (!offsetValue || !limitValue)
                return errorResponse(400, "INVALID_PAGINATION", "offset and limit are required");
            auto offset = parseInt32(offsetValue);
            auto limit = parseInt32(limitValue);
            if (!offset || !limit)
                return errorResponse(400, "INVALID_PAGINATION",
                                     "offset and limit must be non-negative int32 values");

            auto page = useCases.getPage(*offset, *limit);
            if (!page)
                return errorResponse(500, "INTERNAL_ERROR", "Internal server error");

            crow::json::wvalue json;
            crow::json::wvalue::list todos;
            for (const auto& todo : page->items)
                todos.push_back(todoToJson(todo));
            json["todos"] = std::move(todos);
            json["meta"]["total"] = page->meta.total;
            json["meta"]["offset"] = page->meta.offset;
            json["meta"]["limit"] = page->meta.limit;
            return crow::response{200, json};
        });

    CROW_ROUTE(app, "/api/todos")
        .methods(crow::HTTPMethod::POST)([&useCases](const crow::request& request) {
            if (!hasJsonContentType(request))
                return errorResponse(415, "UNSUPPORTED_MEDIA_TYPE",
                                     "Content-Type must be application/json");
            if (request.body.size() > MaxRequestBytes)
                return errorResponse(413, "PAYLOAD_TOO_LARGE", "Request body is too large");
            auto json = crow::json::load(request.body);
            if (!json)
                return errorResponse(400, "INVALID_JSON", "Request body is not valid JSON");
            auto input = parseTodoInput(json);
            if (!input)
                return errorResponse(422, "VALIDATION_ERROR", "Request validation failed");

            auto result = useCases.createTodo(*input);
            if (!result) {
                return std::visit(
                    [](const auto& error) {
                        using Error = std::decay_t<decltype(error)>;
                        if constexpr (std::is_same_v<Error, domain::TodoValidationError>)
                            return errorResponse(422, "VALIDATION_ERROR",
                                                 "Request validation failed");
                        else
                            return errorResponse(500, "INTERNAL_ERROR", "Internal server error");
                    },
                    result.error());
            }

            crow::json::wvalue responseJson;
            responseJson["todo"] = todoToJson(result->todo);
            responseJson["handlers"] = handlersReportToJson(result->handlers);
            const bool allSucceeded =
                std::ranges::all_of(result->handlers, [](const auto& handler) {
                    return handler.status == service::HandlerExecutionStatus::Success;
                });
            responseJson["status"] = allSucceeded ? "created" : "created_with_handler_errors";
            crow::response response{201, responseJson};
            response.set_header("Content-Type", "application/json");
            response.set_header("Location", "/api/todos/" + std::to_string(result->todo.id.id));
            return response;
        });

    CROW_ROUTE(app, "/api/todos/<string>").methods(crow::HTTPMethod::GET)([](const std::string&) {
        return notImplementedResponse();
    });
    CROW_ROUTE(app, "/api/todos/<string>")
        .methods(crow::HTTPMethod::PATCH)(
            [](const crow::request&, const std::string&) { return notImplementedResponse(); });
    CROW_ROUTE(app, "/api/todos/<string>")
        .methods(crow::HTTPMethod::DELETE)(
            [](const std::string&) { return notImplementedResponse(); });
}

} // namespace todod::http::routes
