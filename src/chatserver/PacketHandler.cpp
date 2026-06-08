// PacketHandler.cpp
#include "PacketHandler.h"

PacketHandler::PacketHandler(DBManager* dbManager, WebSocketServer* wsServer) 
    : m_dbManager(dbManager), m_wsServer(wsServer), m_running(true) {}

void PacketHandler::addPacket(const clientPacket &packet) {
    lock_guard<mutex> lk(m_handlerMutex);
    m_handlerQueue.push(packet);
    m_newHandlerDataCond.notify_one();
}

void PacketHandler::handlePackets() {
    while (m_running) {
        unique_lock<mutex> lk(m_handlerMutex);
        m_newHandlerDataCond.wait(lk, [this] { return !m_handlerQueue.empty() || !m_running; });
        
        if (!m_running) break;
        
        clientPacket cltPack = m_handlerQueue.front();
        m_handlerQueue.pop();
        lk.unlock();
        
        handlePacket(cltPack);
    }
}

void PacketHandler::stop() {
    m_running = false;
    m_newHandlerDataCond.notify_all();
}

void PacketHandler::handlePacket(const clientPacket &packet) {
    websocketpp::connection_hdl ClientHdl = packet.hdl;
    int Type = packet.type;
    int RequestID = packet.request_id;
    string From = packet.from;
    string To = packet.to;
    ordered_json Data = packet.data;
    
    try {   
        if (Type == CT_NEWMESSAGE) {
            cout << "NewMsg handler" << endl;
            bool online = isUserAuthenticated(From).value_or(0);
            if (!online) {
                return;
            }
            
            string newMsg = Data["message"];
            ordered_json res = m_dbManager->insertHistory(newMsg, From);
            cout << newMsg << " added to history" << endl;
            
            ordered_json jsonMsg = ordered_json::array();
            jsonMsg.push_back(res);
            
            serverPacket srvPacket = serverPacket::createNewMessageEVENT(ClientHdl, From, jsonMsg);
            broadcastPacket(srvPacket);
        }
        else if (Type == CT_REGISTER) {
            cout << "Register handler" << endl;
            string username = Data["username"];
            bool res = m_dbManager->isUserExists(username);
            
            if (res) {
                serverPacket srvPacket = serverPacket::createRegisterRPS(ClientHdl, username, "err");
                sendPacket(srvPacket);
                cout << "User " << username << " already exists!" << endl;
                return;
            }
            
            string password = Data["password"];
            cout << "Register successful!" << endl;
            m_dbManager->registerUser(username, password);

            serverPacket srvPacket = serverPacket::createRegisterRPS(ClientHdl, username, "ack");
            sendPacket(srvPacket);
        }
        else if (Type == CT_LOGIN) {
            cout << "Login handler" << endl;
            string username = Data["username"];
            string password = Data["password"];
            bool res = m_dbManager->verifyUser(username, password);
            
            if (!res) {
                cout << "Wrong username or password!" << endl;
                serverPacket srvPacket = serverPacket::createLoginRPS(ClientHdl, username, "err");
                sendPacket(srvPacket);
                return;
            }
            bool online = isUserAuthenticated(username).value_or(0); 
            if (online) {
                cout << "User already logged in!" << endl;
                serverPacket srvPacket = serverPacket::createLoginRPS(ClientHdl, username, "err");
                sendPacket(srvPacket);
                return;
            }
            
            authenticateUser(ClientHdl, username);

            serverPacket srvPacket = serverPacket::createLoginRPS(ClientHdl, username, "ack");
            sendPacket(srvPacket);

            cout << "Login successful!" << endl;
        }
        else if (Type == CT_DATA) {
            cout << "Data handler" << endl;
            cout << Data.dump() << endl;
            int count = Data["count"];
            int offset = Data["offset"];
            string table = Data["table"];
            vector<string> cols = Data["columns"];
            ordered_json j = m_dbManager->getData(count, offset, table, cols);
            serverPacket dataPacket = serverPacket::createDataRPS(RequestID, ClientHdl, From, j, "data");
            sendPacket(dataPacket);
        }
        else if (Type == CT_SSP) {
            cout << "SSP handler" << endl;
            handleSSPMessage(packet);
        }
    }
    catch (const exception& ex) {
        cout << ex.what() << endl;
    }
}