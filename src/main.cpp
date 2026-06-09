#include "business/auth_service.hpp"
#include "business/message_service.hpp"
#include "business/user_service.hpp"
#include "common/constants.hpp"
#include "middleware/auth_middleware.hpp"
#include "persistence/dao/message_dao.hpp"
#include "persistence/dao/session_dao.hpp"
#include "persistence/dao/user_dao.hpp"
#include "persistence/database.hpp"
#include "presentation/http_handler.hpp"
#include "presentation/websocket_handler.hpp"
#include "presentation/router.hpp"
#include "utils/config.hpp"
#include "utils/logger.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/http.hpp>

#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

int main() {
    try {
        const auto cfg = mserver::utils::loadConfig("config/app.conf");

        mserver::persistence::Database db(cfg.dbPath);
        db.runSchema("db/schema.sql");

        mserver::persistence::dao::UserDao userDao(db);
        mserver::persistence::dao::SessionDao sessionDao(db);
        mserver::persistence::dao::MessageDao messageDao(db);

        mserver::business::AuthService authService(userDao, sessionDao);
        mserver::business::UserService userService(userDao);
        mserver::business::MessageService messageService(messageDao);
        mserver::middleware::AuthMiddleware authMiddleware(authService);

        mserver::presentation::HttpHandler httpHandler(authService, userService, messageService, authMiddleware);
        mserver::presentation::WebSocketHandler websocketHandler;

        boost::asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), cfg.httpPort ? cfg.httpPort : mserver::common::kDefaultHttpPort}};
        mserver::utils::Logger::log(mserver::utils::LogLevel::INFO, "Server started on port " + std::to_string(acceptor.local_endpoint().port()));

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            std::thread([s = std::move(socket), &httpHandler, &websocketHandler]() mutable {
                beast::flat_buffer buffer;
                http::request<http::string_body> req;
                http::read(s, buffer, req);

                if (beast::websocket::is_upgrade(req)) {
                    const auto route = mserver::presentation::splitPathAndQuery(std::string(req.target()));
                    if (route.path == "/chat" || route.path == "/notifications") {
                        websocketHandler.handle(std::move(s), std::move(req));
                    }
                    return;
                }

                const auto response = httpHandler.handle(req);
                http::write(s, response);
                beast::error_code ec;
                s.shutdown(tcp::socket::shutdown_send, ec);
            }).detach();
        }
    } catch (const std::exception& ex) {
        mserver::utils::Logger::log(mserver::utils::LogLevel::ERROR, ex.what());
        return 1;
    }
}
