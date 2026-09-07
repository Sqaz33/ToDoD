#include "script_routes.hpp"

#include "domain/event.hpp"
#include "json_serializer.hpp"

#include <limits>
#include <optional>
#include <type_traits>

namespace todod::http::routes {

namespace {

constexpr std::size_t MaxRequestBytes = 128 * 1024;

bool hasJsonContentType(const crow::request& request) {
    return request.get_header_value("Content-Type").starts_with("application/json");
}

std::optional<std::int64_t> jsonInteger(const crow::json::rvalue& value) {
    if (value.t() != crow::json::type::Number)
        return std::nullopt;
    if (value.nt() == crow::json::num_type::Signed_integer)
        return value.i();
    if (value.nt() == crow::json::num_type::Unsigned_integer &&
        value.u() <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
        return static_cast<std::int64_t>(value.u());
    return std::nullopt;
}

std::optional<domain::HandlerScriptInput> parseHandlerInput(const crow::json::rvalue& json) {
    if (!json.has("name") || !json.has("source") || !json.has("event") || !json.has("enabled"))
        return std::nullopt;
    if (json["name"].t() != crow::json::type::String ||
        json["source"].t() != crow::json::type::String)
        return std::nullopt;
    const auto enabledType = json["enabled"].t();
    if (enabledType != crow::json::type::True && enabledType != crow::json::type::False)
        return std::nullopt;
    auto event = jsonInteger(json["event"]);
    if (!event)
        return std::nullopt;
    return domain::HandlerScriptInput{
        .name = json["name"].s(),
        .source = json["source"].s(),
        .event = *event,
        .enabled = json["enabled"].b(),
    };
}

} // namespace

void registerScriptRoutes(crow::SimpleApp& app, use_cases::HandlerUseCases& useCases) {
    CROW_ROUTE(app, "/api/scripts/events").methods(crow::HTTPMethod::GET)([] {
        crow::json::wvalue json;
        crow::json::wvalue::list events;
        events.push_back(crow::json::wvalue{{"id", 0}, {"name", "ADDED_TODO"}});
        events.push_back(crow::json::wvalue{{"id", 1}, {"name", "UPDATED_TODO"}});
        events.push_back(crow::json::wvalue{{"id", 2}, {"name", "DELETED_TODO"}});
        json["events"] = std::move(events);
        return crow::response{200, json};
    });

    CROW_ROUTE(app, "/api/scripts/handlers").methods(crow::HTTPMethod::GET)([&useCases] {
        auto handlers = useCases.getHandlers();
        if (!handlers)
            return errorResponse(500, "INTERNAL_ERROR", "Internal server error");
        crow::json::wvalue json;
        crow::json::wvalue::list values;
        for (const auto& handler : *handlers)
            values.push_back(handlerToJson(handler));
        json["handlers"] = std::move(values);
        return crow::response{200, json};
    });

    CROW_ROUTE(app, "/api/scripts/handlers")
        .methods(crow::HTTPMethod::POST)([&useCases](const crow::request& request) {
            if (!hasJsonContentType(request))
                return errorResponse(415, "UNSUPPORTED_MEDIA_TYPE",
                                     "Content-Type must be application/json");
            if (request.body.size() > MaxRequestBytes)
                return errorResponse(413, "PAYLOAD_TOO_LARGE", "Request body is too large");
            auto json = crow::json::load(request.body);
            if (!json)
                return errorResponse(400, "INVALID_JSON", "Request body is not valid JSON");
            auto input = parseHandlerInput(json);
            if (!input)
                return errorResponse(422, "VALIDATION_ERROR", "Request validation failed");

            auto result = useCases.createHandler(*input);
            if (!result) {
                return std::visit(
                    [](const auto& error) {
                        using Error = std::decay_t<decltype(error)>;
                        if constexpr (std::is_same_v<Error, db::error::StorageError>)
                            return errorResponse(500, "INTERNAL_ERROR", "Internal server error");
                        else if constexpr (std::is_same_v<Error, scripting::error::ScriptError>)
                            return errorResponse(422, "SCRIPT_VALIDATION_FAILED", error.diagnostic,
                                                 "source");
                        else
                            return errorResponse(422, "VALIDATION_ERROR",
                                                 "Request validation failed");
                    },
                    result.error());
            }

            crow::response response{201, handlerToJson(*result)};
            response.set_header("Content-Type", "application/json");
            response.set_header("Location",
                                "/api/scripts/handlers/" + std::to_string(result->id.id));
            return response;
        });

    CROW_ROUTE(app, "/api/scripts/handlers/<string>")
        .methods(crow::HTTPMethod::PATCH)(
            [](const crow::request&, const std::string&) { return notImplementedResponse(); });
    CROW_ROUTE(app, "/api/scripts/handlers/<string>")
        .methods(crow::HTTPMethod::DELETE)(
            [](const std::string&) { return notImplementedResponse(); });
}

} // namespace todod::http::routes
