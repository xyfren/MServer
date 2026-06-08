#ifndef MEDIA_SERVER_H
#define MEDIA_SERVER_H

#include <boost/asio.hpp>
#include <functional>
#include <vector>
#include <cstdint>

#include "MediaServer.h"

using namespace std;

class MediaServer {
public:
    using MediaDataCallback = function<void(const vector<uint8_t>& data, 
                                                const boost::asio::ip::udp::endpoint& fromEndpoint)>;

    MediaServer(boost::asio::io_context& io_context);
    ~MediaServer();
    
    void run(uint16_t port);
    void send(const vector<uint8_t>& data, const boost::asio::ip::udp::endpoint& targetEndpoint);
    
    void setMediaDataCallback(MediaDataCallback callback);
private:
    void handleUdpReceive();

    boost::asio::io_context& m_io_context;

    unique_ptr<boost::asio::ip::udp::socket> m_udpSocket;
    MediaDataCallback m_mediaCallback;
    
    array<uint8_t, 1500> m_receiveBuffer;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
};

#endif