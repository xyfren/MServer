#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "WebSocketServer.h"
#include "PacketHandler.h"
#include "SSPServer.h"
#include "common/DBManager.h"
#include <memory>

using namespace std;

class ChatServer {
public:
    ChatServer(const shared_ptr<DBManager>& p_dbManger);
    ~ChatServer();
    
    void run(uint16_t port);
    void stop();

    shared_ptr<DBManager> m_dbManager;

    unique_ptr<WebSocketServer> m_webSocketServer;
    unique_ptr<PacketHandler> m_packetHandler;
    unique_ptr<SSPServer> m_sessionServer;

private:
    void setupConnections();

    void broadcastPacket(const serverPacket &packet);

    void authenticateUser(websocketpp::connection_hdl hdl, const std::string& username);

    void deauthenticateUser(websocketpp::connection_hdl hdl);

    bool isUserAuthenticated(const std::string& username);

    map<string, websocketpp::connection_hdl> m_authUsers;
    map<websocketpp::connection_hdl, string, owner_less<websocketpp::connection_hdl>> m_authConnections;
    
};

#endif // CHAT_SERVER_H