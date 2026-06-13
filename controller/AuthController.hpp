#include <domain/common/Common.hpp>
#include <domain/services/AuthService.hpp>

struct connection;

class AuthController {
    AuthService& authService;
public:
    AuthController(AuthService& srv): authService(srv){}

    void handlePostAuth(connection& c, boost::urls const&m)

}