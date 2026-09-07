#include "docs_routes.hpp"

#include <filesystem>
#include <string>

#ifndef TODOD_DOCS_DIR
#define TODOD_DOCS_DIR "./docs"
#endif

namespace todod::http::routes {

namespace {

const std::filesystem::path DOCS_DIR{TODOD_DOCS_DIR};

void addSecurityHeaders(crow::response& res) {
    res.add_header("Content-Security-Policy",
                   "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; "
                   "img-src 'self' data:; connect-src 'self'; object-src 'none'; "
                   "base-uri 'none'; frame-ancestors 'none'");
    res.add_header("Referrer-Policy", "no-referrer");
    res.add_header("X-Content-Type-Options", "nosniff");
    res.add_header("X-Frame-Options", "DENY");
}

void serveFile(crow::response& res, const std::filesystem::path& path,
               const std::string& contentType, const std::string& cacheControl) {
    addSecurityHeaders(res);
    res.add_header("Cache-Control", cacheControl);
    // The path is assembled only from compile-time constants. Crow's safe
    // variant rewrites the leading slash of an absolute Unix path.
    res.set_static_file_info_unsafe(path.string(), contentType);
    res.end();
}

void serveDocs(crow::response& res) {
    serveFile(res, DOCS_DIR / "swagger.html", "text/html; charset=utf-8", "no-store");
}

} // namespace

void registerDocsRoutes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/docs")(serveDocs);

    CROW_ROUTE(app, "/openapi.json")
    ([](crow::response& res) {
        serveFile(res, DOCS_DIR / "openapi.json", "application/json; charset=utf-8", "no-store");
    });

    CROW_ROUTE(app, "/docs/assets/swagger-ui.css")
    ([](crow::response& res) {
        serveFile(res, DOCS_DIR / "swagger-ui" / "swagger-ui.css", "text/css; charset=utf-8",
                  "public, max-age=86400");
    });

    CROW_ROUTE(app, "/docs/assets/swagger-ui-bundle.js")
    ([](crow::response& res) {
        serveFile(res, DOCS_DIR / "swagger-ui" / "swagger-ui-bundle.js",
                  "text/javascript; charset=utf-8", "public, max-age=86400");
    });

    CROW_ROUTE(app, "/docs/assets/todod-swagger.css")
    ([](crow::response& res) {
        serveFile(res, DOCS_DIR / "todod-swagger.css", "text/css; charset=utf-8", "no-cache");
    });

    CROW_ROUTE(app, "/docs/assets/swagger-initializer.js")
    ([](crow::response& res) {
        serveFile(res, DOCS_DIR / "swagger-initializer.js", "text/javascript; charset=utf-8",
                  "no-cache");
    });
}

} // namespace todod::http::routes
