// SignalingServer.cpp
#include "SignalingServer.h"
#include <iostream>
#include <algorithm>

SignalingServer::SignalingServer(boost::asio::io_context& io_context)
    : m_io_context(io_context)
    , m_acceptor(io_context) {
        openHandler = nullptr;
        messageHandler = nullptr;
        closeHandler = nullptr;
}

SignalingServer::~SignalingServer() {
    stop();
}

void SignalingServer::run(uint16_t port) {
    try {
        setOpenHandler(bind(&SignalingServer::onOpen, this, placeholders::_1));
        setMessageHandler(bind(&SignalingServer::onMessage, this, placeholders::_1,placeholders::_2));
        setClosehandler(bind(&SignalingServer::onClose, this, placeholders::_1));

        tcp::endpoint endpoint(tcp::v4(), port);
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
        
        startAccept();
        cout << "Signaling server running on port " << port << endl;
    }
    catch (const exception& ex) {
        cerr << "Error running SignalingServer: " << ex.what() << endl;
    }
}

void SignalingServer::stop() {
    boost::system::error_code ec;
    m_acceptor.close(ec);
    
    lock_guard<mutex> lock(m_connectionsMutex);
    for (auto& socket : m_connections) {
        socket->close(ec);
    }
    m_connections.clear();
}

void SignalingServer::send(const vector<uint8_t>& data, shared_ptr<tcp::socket> socket) {
    if (!socket || !socket->is_open()) {
        return;
    }

    vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(data));

    boost::asio::async_write(*socket, buffers,
        [this, socket](boost::system::error_code ec, size_t /*length*/) {
            if (ec) {
                cout << "Error sending binary data: " << ec.message() << endl;
                removeConnection(socket);
            }
        });
}

void SignalingServer::setSignalCallback(SignalCallback callback){
    m_signalCallback = callback; 
}

void SignalingServer::broadcastBinary(const vector<uint8_t>& data) {
    lock_guard<mutex> lock(m_connectionsMutex);
    for (auto& socket : m_connections) {
        if (socket->is_open()) {
            send(data, socket);
        }
    }
}

void SignalingServer::setOpenHandler(function<void(shared_ptr<tcp::socket> socket)> openHandler){
    this->openHandler = openHandler;
}

void SignalingServer::setMessageHandler(function<void(const vector<uint8_t>& data, shared_ptr<tcp::socket> socket)> messageHandler){
    this->messageHandler = messageHandler;
}

void SignalingServer::setClosehandler(function<void(shared_ptr<tcp::socket> socket)> closeHandler){
    this->closeHandler = closeHandler;
}

void SignalingServer::startAccept() {
    auto socket = make_shared<tcp::socket>(m_io_context);
    m_acceptor.async_accept(*socket,
        boost::bind(&SignalingServer::handleAccept, this, socket,
            boost::asio::placeholders::error));
}

void SignalingServer::handleAccept(shared_ptr<tcp::socket> socket, const boost::system::error_code& error) {
    if (!error) {
        {
            lock_guard<mutex> lock(m_connectionsMutex);
            m_connections.insert(socket);
        }
              
        if (openHandler) {
            openHandler(socket);
        }
        
        startRead(socket);
    } else {
        cout << "Accept error: " << error.message() << endl;
    }
    
    startAccept();
}

void SignalingServer::startRead(shared_ptr<tcp::socket> socket) {
    socket->async_read_some(boost::asio::buffer(m_buffer),
        boost::bind(&SignalingServer::handleRead, this, socket,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void SignalingServer::handleRead(shared_ptr<tcp::socket> socket, 
                               const boost::system::error_code& error,
                               size_t bytes_transferred) {
    if (!error) {
        if (messageHandler && bytes_transferred > 0) {
            // Добавляем новые данные в буфер для этого клиента
            static vector<uint8_t> packetBuffer; // Лучше сделать членом класса
            
            packetBuffer.insert(packetBuffer.end(), 
                               m_buffer.begin(), 
                               m_buffer.begin() + bytes_transferred);
            
            // Пока в буфере есть данные, пытаемся распарсить пакеты
            while (!packetBuffer.empty()) {
                // ПРЕДПОЛОЖИМ, что SMPSPacket имеет фиксированный размер
                // или известен размер пакета
                const size_t PACKET_SIZE = SMPSPacketSize; // Замените на реальный размер вашего SMPSPacket
                
                if (packetBuffer.size() >= PACKET_SIZE) {
                    // Извлекаем один полный пакет
                    vector<uint8_t> singlePacket(packetBuffer.begin(), 
                                                 packetBuffer.begin() + PACKET_SIZE);
                    
                    // Удаляем обработанный пакет из буфера
                    packetBuffer.erase(packetBuffer.begin(), 
                                      packetBuffer.begin() + PACKET_SIZE);
                    
                    // Обрабатываем один пакет
                    messageHandler(singlePacket, socket);
                } else {
                    // Недостаточно данных для полного пакета, ждем следующий read
                    break;
                }
            }
        }
        startRead(socket);
    } else {
        if (error != boost::asio::error::eof) {
            cout << "Read error: " << error.message() << endl;
        }
        removeConnection(socket);
    }
}

void SignalingServer::removeConnection(shared_ptr<tcp::socket> socket) {
    lock_guard<mutex> lock(m_connectionsMutex);
    if (m_connections.erase(socket)) {
        if (closeHandler) {
            closeHandler(socket);
        }
    }
}

void SignalingServer::onOpen(shared_ptr<tcp::socket> socket){
    cout << "New client connected" << endl;

}

void SignalingServer::onMessage(const vector<uint8_t>& data, shared_ptr<tcp::socket> socket){
    cout << "New message: " << string(data.begin(),data.end()) << endl;
    cout << "Size: " << data.size() << endl;
    m_signalCallback(data,socket);
}

void SignalingServer::onClose(shared_ptr<tcp::socket> socket){
    cout << "Signaling client disconnected" << endl;
}
