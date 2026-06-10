
#include <domain/services/AuthService.h"

#include <controller/AuthController.h>

#include <network/HttpServer.h> // Класс, запускающий acceptor

int main() {
    try {
        std::cout << "Starting server..." << std::endl;

        // ---------------------------------------------------------
        // ШАГ 0: Базовая инфраструктура (Boost.Asio, Конфиги)
        // ---------------------------------------------------------
        net::io_context ioc{std::thread::hardware_concurrency()};
        
        // В реальности здесь будут параметры из конфигурационного файла
        PostgresConnectionPool pg_pool("host=localhost dbname=chatdb user=admin password=secret");
        RedisClient redis_client(ioc, "127.0.0.1", 6379);

        // ---------------------------------------------------------
        // ШАГ 1: Сборка Слоя 4 (DAO и Репозитории)
        // ---------------------------------------------------------
        // Создаем DAO, передаем ему пул соединений с БД
        PostgresSessionDao session_dao(pg_pool); 
        
        // Создаем Репозиторий, передаем ему DAO и Redis
        PersistentSessionRepository session_repo(session_dao, redis_client);
        
        // (Здесь же создали бы UserRepository и т.д.)
        // DummyUserRepository user_repo; 

        // ---------------------------------------------------------
        // ШАГ 2: Сборка Слоя 3 (Бизнес-логика / Use Cases)
        // ---------------------------------------------------------
        
        AuthService auth_service(user_repo, session_repo);

        // ---------------------------------------------------------
        // ШАГ 3: Сборка Слоя 2 (Контроллеры)
        // ---------------------------------------------------------

        AuthController auth_controller(auth_service);

        // ---------------------------------------------------------
        // ШАГ 4: Сборка Слоя 1 (Сеть / TCP Сервер)
        // ---------------------------------------------------------
        auto endpoint = net::ip::tcp::endpoint(net::ip::tcp::v4(), 8080);
        
        HttpServer http_server(ioc, endpoint, auth_controller); // Сервер получает io_context для сети и контроллер для роутинга
        
        http_server.Start(); // Запускаем корутину прослушивания порта (co_spawn)

        // ---------------------------------------------------------
        // ШАГ 5: Запуск цикла обработки событий (Event Loop)
        // ---------------------------------------------------------
        std::cout << "Server is listening on port 8080" << std::endl;

        // Создаем пул потоков для обработки Asio событий
        std::vector<std::thread> threads;
        for(int i = 0; i < std::thread::hardware_concurrency() - 1; ++i) {
            threads.emplace_back([&ioc]{ ioc.run(); });
        }
        
        // Главный поток тоже участвует в обработке
        ioc.run();

        // Ожидание завершения потоков (при штатном выключении)
        for(auto& t : threads) {
            t.join();
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}