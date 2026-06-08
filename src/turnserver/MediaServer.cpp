#include "MediaServer.h"
#include <iostream>

MediaServer::MediaServer(boost::asio::io_context& io_context) 
    : m_io_context(io_context) {
}

MediaServer::~MediaServer() {
    if (m_udpSocket) {
        m_udpSocket->close();
    }
}

void MediaServer::run(uint16_t port) {
    try {
        m_udpSocket = make_unique<boost::asio::ip::udp::socket>(m_io_context);
        boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), port);
        m_udpSocket->open(endpoint.protocol());
        m_udpSocket->bind(endpoint);
        
        cout << "Media server running on port " << port << endl;
        handleUdpReceive();
    }
    catch (const exception& ex) {
        cerr << "Error running MediaServer: " << ex.what() << endl;
    }
}

void MediaServer::handleUdpReceive() {
    if (!m_udpSocket) return;
    
    m_udpSocket->async_receive_from(
        boost::asio::buffer(m_receiveBuffer), m_remoteEndpoint,
        [this](boost::system::error_code ec, size_t bytes_recvd) {
            if (!ec && bytes_recvd > 0 && m_mediaCallback) {
                m_mediaCallback(
                    vector<uint8_t>(m_receiveBuffer.begin(), 
                                       m_receiveBuffer.begin() + bytes_recvd),
                    m_remoteEndpoint
                );
            }
            
            handleUdpReceive();
        });
}

void MediaServer::send(const vector<uint8_t>& data, const boost::asio::ip::udp::endpoint& targetEndpoint) {
    if (!m_udpSocket) return;

    m_udpSocket->send_to(boost::asio::buffer(data), targetEndpoint);
}

void MediaServer::setMediaDataCallback(MediaDataCallback callback) { 
    m_mediaCallback = callback; 
}