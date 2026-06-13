#include <domain/common/Common.hpp>

#include <controller/AuthController.hpp>

#include <iostream>
#include <memory>

namespace net = boost::asio;
using tcp = net::ip::tcp;

// Наш класс сервера
class HttpServer {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    AuthController& auth_controller_; // Ссылка на Слой 2 (Контроллеры)

public:
    HttpServer(net::io_context& ioc, const tcp::endpoint& endpoint, AuthController& auth_controller)
        : ioc_(ioc), acceptor_(ioc, endpoint), auth_controller_(auth_controller) {}

    // Эту функцию мы вызывали в main()
    void Start() {
        // Запускаем бесконечный цикл прослушивания как отдельную корутину
        net::co_spawn(ioc_, AcceptLoop(), net::detached);
    }

private:
    // Главная корутина сервера
    net::awaitable<void> AcceptLoop() {
        try {
            while (true) {
                // 1. Асинхронно спим, пока не придет новый клиент
                tcp::socket socket = co_await acceptor_.async_accept(net::use_awaitable);

                // 2. Клиент пришел! Создаем изолированный контекст (strand) для потокобезопасности
                auto executor = net::make_strand(ioc_);

                // 3. Создаем тот самый "несчастный" HttpConnection
                // Используем shared_ptr, чтобы объект жил, пока работает его корутина
                auto connection = std::make_shared<HttpConnection>(auth_controller_);

                // 4. Запускаем корутину обработки этого соединения
                // Сервер делегирует работу и мгновенно возвращается в начало цикла while (true)
                net::co_spawn(executor, 
                              [conn = connection, sock = std::move(socket)]() mutable -> net::awaitable<void> {
                                  co_await conn->HandleConnection(std::move(sock));
                              }, 
                              net::detached);
            }
        } catch (const std::exception& e) {
            std::cerr << "Слушатель сервера упал: " << e.what() << "\n";
        }
    }
};