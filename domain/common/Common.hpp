#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/url/url.hpp>
#include <string>
#include <optional>

namespace net = boost::asio;
namespace urls = boost::urls;

struct connection;
using handler = std::function<void(connection&, urls)>;
