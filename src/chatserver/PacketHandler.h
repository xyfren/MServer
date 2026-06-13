// PacketHandler.h
#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <boost/signals2.hpp>

#include "common/DBManager.h"
#include "packets/ClientPacket.h"
#include "packets/ServerPacket.h"

using ordered_json = nlohmann::ordered_json;

using namespace std;

class DBManager;
class WebSocketServer;

class PacketHandler {
public:
    PacketHandler(DBManager* dbManager, WebSocketServer* wsServer);
    
    void addPacket(const clientPacket &packet);
    void handlePackets();
    void stop();
    
    // Сигналы для внешних событий
    boost::signals2::signal<void(const serverPacket & packet)> sendPacket;
    boost::signals2::signal<void(const serverPacket & packet)> broadcastPacket;

    boost::signals2::signal<bool(const string &username)> isUserAuthenticated;
    boost::signals2::signal<void(websocketpp::connection_hdl hdl, const string &username)> authenticateUser;
    boost::signals2::signal<void(const clientPacket &packet)> handleSSPMessage;

private:
    void handlePacket(const clientPacket &packet);
    
    DBManager* m_dbManager;
    WebSocketServer* m_wsServer;
    
    queue<clientPacket> m_handlerQueue;
    mutex m_handlerMutex;
    condition_variable m_newHandlerDataCond;
    bool m_running;
};

#endif // PACKET_HANDLER_H