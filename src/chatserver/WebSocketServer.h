// WebSocketServer.h
#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <boost/signals2.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/logger/stub.hpp>
#include <websocketpp/logger/syslog.hpp>
#include <condition_variable>
#include <queue>
#include <vector>
#include <mutex>
#include <map>
#include <set>
#include <memory>
#include <thread>

#include "packets/serverPacket.h"
#include "packets/clientPacket.h"

using ordered_json = nlohmann::ordered_json;

using namespace std;

struct my_custom_config : public websocketpp::config::asio {
    typedef websocketpp::log::syslog<concurrency_type, websocketpp::log::elevel> elog_type;
    typedef websocketpp::log::syslog<concurrency_type, websocketpp::log::alevel> alog_type;
};

typedef websocketpp::server<my_custom_config> server;
typedef set<websocketpp::connection_hdl, owner_less<websocketpp::connection_hdl>> connection_list;

class PacketHandler;

class WebSocketServer {
public:
    WebSocketServer();
    ~WebSocketServer();
    
    void run(uint16_t port);
    void stop();
    
    void sendPacket(const serverPacket &packet);
    // Сигналы для внешних событий
    boost::signals2::signal<void(const clientPacket&)> addPacket;
    boost::signals2::signal<void(websocketpp::connection_hdl hdl)> deauthenticateUser;
    boost::signals2::signal<bool(const string& username)> isUserAuthenticated;
    boost::signals2::signal<map<string,websocketpp::connection_hdl> &()> getAuthUsers;

private:
    void onOpen(websocketpp::connection_hdl hdl);
    void onClose(websocketpp::connection_hdl hdl);
    void onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg);
    void onFail(websocketpp::connection_hdl hdl); 
    
    void senderThread();

    server m_server;
    connection_list m_connections;
    
    queue<serverPacket> m_senderQueue;
    mutex m_senderMutex;
    condition_variable m_newSenderDataCond;
    
    thread m_senderThread;
    bool m_running;
};

#endif // WEBSOCKET_SERVER_H