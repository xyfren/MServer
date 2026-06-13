#include "business/auth_service.hpp"
#include "business/Message_service.hpp"
#include "business/user_service.hpp"
#include "common/constants.hpp"
#include "middleware/auth_middleware.hpp"
#include "persistence/dao/MessageDao.hpp"
#include "persistence/dao/SessionDao.hpp"
#include "persistence/dao/UserDao.hpp"
#include "persistence/Database.hpp"
#include "presentation/HttpHandler.hpp"
#include "presentation/WebSocketHandler.hpp"
#include "presentation/Router.hpp"
#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/http.hpp>

#include <atomic>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

int main(int argc, char** argv) {
    try {
        const std::string configPath = argc > 1 ? argv[1] : "config/app.conf";
        const auto cfg = mserver::utils::loadConfig(configPath);

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
        constexpr int kMaxConcurrentConnections = 64;
        std::atomic<int> activeConnections{0};

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            if (activeConnections.load() >= kMaxConcurrentConnections) {
                http::response<http::string_body> busy{http::status::service_unavailable, 11};
                busy.set(http::field::content_type, "application/json");
                busy.body() = R"({"error":"server is busy"})";
                busy.prepare_payload();
                http::write(socket, busy);
                beast::error_code ec;
                socket.shutdown(tcp::socket::shutdown_send, ec);
                continue;
            }

            activeConnections.fetch_add(1);
            std::thread([s = std::move(socket), &httpHandler, &websocketHandler, &activeConnections]() mutable {
                struct ConnectionGuard {
                    std::atomic<int>& counter;
                    ~ConnectionGuard() { counter.fetch_sub(1); }
                } guard{activeConnections};
                try {
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
                } catch (const std::exception& ex) {
                    mserver::utils::Logger::log(mserver::utils::LogLevel::WARN, ex.what());
                }
            }).detach();
        }
    } catch (const std::exception& ex) {
        mserver::utils::Logger::log(mserver::utils::LogLevel::ERROR, ex.what());
        return 1;
    }
}
