#include "WebSocketServer.h"
#include <iostream>

WebSocketServer::WebSocketServer() : m_running(false) {
    m_server.init_asio();
    
    m_server.set_open_handler(bind(&WebSocketServer::onOpen, this, placeholders::_1));
    m_server.set_close_handler(bind(&WebSocketServer::onClose, this, placeholders::_1));
    m_server.set_message_handler(bind(&WebSocketServer::onMessage, this, placeholders::_1, placeholders::_2));
    m_server.set_fail_handler(bind(&WebSocketServer::onFail, this, placeholders::_1));
}

WebSocketServer::~WebSocketServer() {
    stop();
}

void WebSocketServer::run(uint16_t port) {
    try {
        m_running = true;
        m_senderThread = thread(&WebSocketServer::senderThread, this);
        m_senderThread.detach();
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    } 
    catch (const exception& ex) {
        cerr << ex.what() << endl;
    }
}

void WebSocketServer::stop() {
    m_running = false;
    m_newSenderDataCond.notify_all();
    
    if (m_senderThread.joinable()) {
        m_senderThread.join();
    }
    
    m_server.stop();
}

void WebSocketServer::onOpen(websocketpp::connection_hdl hdl) {
    m_connections.insert(hdl);

    cout << "New client connected. Total clients: " << m_connections.size() << endl;
}

void WebSocketServer::onClose(websocketpp::connection_hdl hdl) {
    deauthenticateUser(hdl);
    m_connections.erase(hdl);
    cout << "Client disconnected. Total clients: " << m_connections.size() << endl;
}

void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    try {
        string message = msg->get_payload();
        cout << "Received: " << message << endl;
        
        ordered_json jmsg = ordered_json::parse(message);
        clientPacket pack = clientPacket::getPacketFromJson(hdl, jmsg);
        
        addPacket(pack);
    }
    catch (ordered_json::parse_error& err) {
        cout << "JSON parse error: " << err.what() << endl;
    }
    catch (const exception& ex) {
        cout << "Message handling error: " << ex.what() << endl;
    }
}

void WebSocketServer::onFail(websocketpp::connection_hdl hdl){
    cout << "Connection failed" << endl;
    onClose(hdl); // Вызываем закрытие при ошибке
}

void WebSocketServer::sendPacket(const serverPacket &packet) {
    lock_guard<mutex> lk(m_senderMutex);
    m_senderQueue.push(packet);
    m_newSenderDataCond.notify_one();
}

void WebSocketServer::senderThread() {
    while (m_running) {
        try {
            unique_lock<mutex> lk(m_senderMutex);
            m_newSenderDataCond.wait(lk, [this] { return !m_senderQueue.empty() || !m_running; });
            
            if (!m_running) break;
            
            serverPacket data = m_senderQueue.front();
            m_senderQueue.pop();
            lk.unlock();
            
            ordered_json j;
            j["category"] = data.category;
            j["type"] = data.type;
            j["request_id"] = data.request_id;
            j["from"] = data.from;
            j["to"] = data.to;
            j["data"] = data.data;
            
            cout << "Send: " << j.dump() << endl;
            
            websocketpp::connection_hdl sendHdl;
            
            bool online = isUserAuthenticated(data.to).value_or(0);
            if (online and data.type != ST_LOGIN and data.type != ST_REGISTER){
                for (auto [key,val]: getAuthUsers().value()){
                    cout << key << endl;
                }
                sendHdl = getAuthUsers().value()[data.to];
            }
            else{
                sendHdl = data.hdl;
            }

            auto conn = m_server.get_con_from_hdl(sendHdl);
            if (conn->get_state() == websocketpp::session::state::open) {
                m_server.send(sendHdl, j.dump(), websocketpp::frame::opcode::text);
            }
        }
        catch (exception &ex) {
            cout << "Sender error: " << ex.what() << endl;
        }
    }
}