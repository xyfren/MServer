// ChatServer.cpp
#include "ChatServer.h"
#include <iostream>

ChatServer::ChatServer(const shared_ptr<DBManager>& p_dbManger) {
    m_dbManager = p_dbManger;
    m_webSocketServer = make_unique<WebSocketServer>();
    m_sessionServer = make_unique<SSPServer>();
    m_packetHandler = make_unique<PacketHandler>(m_dbManager.get(), m_webSocketServer.get());
    
    setupConnections();
}

ChatServer::~ChatServer() {
    stop();
}

void ChatServer::setupConnections() {
    // Связываем получение пакетов от WebSocketServer с PacketHandler
    m_webSocketServer->addPacket.connect(
        [this](const clientPacket& packet) {
            m_packetHandler->addPacket(packet);
        });
    
    // Связываем готовые к отправке пакеты от PacketHandler с WebSocketServer
    m_packetHandler->sendPacket.connect(
        [this](const serverPacket& packet) {
            m_webSocketServer->sendPacket(packet);
        });
    m_sessionServer->sendPacket.connect(
        [this](const serverPacket& packet) {
            m_webSocketServer->sendPacket(packet);
    });
    
    m_packetHandler->isUserAuthenticated.connect(
        [this](const string & username) {
            return isUserAuthenticated(username);
    });

    m_webSocketServer->isUserAuthenticated.connect(
        [this](const string & username) {
            return isUserAuthenticated(username);
    });

    m_packetHandler->authenticateUser.connect(
        [this](websocketpp::connection_hdl hdl,const string & username) {
            authenticateUser(hdl,username);
    });

    m_webSocketServer->deauthenticateUser.connect(
        [this](websocketpp::connection_hdl hdl) {
            deauthenticateUser(hdl);
    });

    m_webSocketServer->getAuthUsers.connect(
        [this](){
            return ref(m_authUsers);
    });

    m_packetHandler->broadcastPacket.connect(
        [this](const serverPacket &packet) {
            broadcastPacket(packet);
    });

    m_packetHandler->handleSSPMessage.connect(
        [this](const clientPacket &packet) {
            m_sessionServer->handleSSPMessage(packet);
    });
}

void ChatServer::broadcastPacket(const serverPacket &packet) {
    for (auto& [hdl, username] : m_authConnections) {
        serverPacket broadcastPacket = packet;
        broadcastPacket.hdl = hdl;
        broadcastPacket.to = m_authConnections[hdl];
        m_webSocketServer->sendPacket(broadcastPacket);
    }
}

void ChatServer::authenticateUser(websocketpp::connection_hdl hdl, const string& username) {
    m_authUsers[username] = hdl;
    m_authConnections[hdl] = username;
    m_dbManager->setOnline(username, true);

    ordered_json j = m_dbManager->getOnlineUsers();
    serverPacket srvPacket = serverPacket::createUpdateUserListPacketEVENT_B(j);
    broadcastPacket(srvPacket);
}

void ChatServer::deauthenticateUser(websocketpp::connection_hdl hdl) {
    auto it = m_authConnections.find(hdl);
    if (it != m_authConnections.end()) {
        string username = it->second;

        m_dbManager->setOnline(username, false);
        m_sessionServer->deauthenticateUser(username);

        m_authUsers.erase(it->second);
        m_authConnections.erase(it);

        ordered_json j = m_dbManager->getOnlineUsers();
        serverPacket srvPacket = serverPacket::createUpdateUserListPacketEVENT_B(j);
        broadcastPacket(srvPacket);
    }
}

bool ChatServer::isUserAuthenticated(const string& username) {
    for (auto& el : m_authUsers){
        cout << el.first << endl;
    }
    cout << (m_authUsers.find(username) != m_authUsers.end()) << endl;
    return m_authUsers.find(username) != m_authUsers.end();
}

void ChatServer::run(uint16_t port) {
    try {
        // Запускаем обработку пакетов в отдельном потоке
        thread packetThread([this]() {
            m_packetHandler->handlePackets();
        });
        packetThread.detach();
        
        // Запускаем WebSocket сервер
        thread wsThread([this,port]() {
            m_webSocketServer->run(port);
        });
        wsThread.detach();
        cout << "WebSocketServer running on port " << port << endl;
    } catch (const exception& ex) {
        cerr << "Error running ChatServer: " << ex.what() << endl;
    }
}

void ChatServer::stop() {
    m_packetHandler->stop();
    m_webSocketServer->stop();
}