#include "presentation/WebSocketHandler.hpp"

#include "utils/Logger.hpp"
#include <boost/beast/core/flat_buffer.hpp>

namespace mserver::presentation {

void WebSocketHandler::handle(tcp::socket socket, http::request<http::string_body> req) const {
    websocket::stream<tcp::socket> ws(std::move(socket));
    ws.accept(req);

    // NOTE: important - both /chat and /notifications are intentionally supported with shared baseline behavior.
    const std::string target = std::string(req.target());
    const std::string hello = target == "/notifications" ? "notifications connected" : "chat connected";
    ws.write(boost::asio::buffer(hello));

    for (;;) {
        boost::beast::flat_buffer buffer;
        boost::beast::error_code ec;
        ws.read(buffer, ec);
        if (ec == websocket::error::closed) {
            break;
        }
        if (ec) {
            break;
        }
        ws.text(true);
        ws.write(buffer.data());
    }
}

} // namespace mserver::presentation
