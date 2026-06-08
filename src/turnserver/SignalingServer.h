// SignalingServer.h
#ifndef SIGNALING_SERVER_H
#define SIGNALING_SERVER_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <functional>
#include <vector>
#include <memory>
#include <set>
#include <mutex>

#include "packets/SMPSPacket.h"

using boost::asio::ip::tcp;

using namespace std;

class SignalingServer {
public:
    using SignalCallback = function<void(const vector<uint8_t>& data, shared_ptr<tcp::socket> fromSocket)>;

    SignalingServer(boost::asio::io_context& io_context);
    ~SignalingServer();

    void run(uint16_t port);
    void stop();
    
    void send(const vector<uint8_t>& data, shared_ptr<tcp::socket> socket);
    void broadcastBinary(const vector<uint8_t>& data);

    void setSignalCallback(SignalCallback callback);

    void onOpen(shared_ptr<tcp::socket> socket);

    void onMessage(const vector<uint8_t>& data, shared_ptr<tcp::socket> socket);

    void onClose(shared_ptr<tcp::socket> socket);

    // Обработчики событий (переопределяются пользователем)
    void setOpenHandler(function<void(shared_ptr<tcp::socket> socket)> openHandler);
    void setMessageHandler(function<void(const vector<uint8_t>& data, shared_ptr<tcp::socket> socket)> messageHandler);
    void setClosehandler(function<void(shared_ptr<tcp::socket> socket)> closeHandler);
   
private:
    void startAccept();
    void handleAccept(shared_ptr<tcp::socket> socket, const boost::system::error_code& error);
    void startRead(shared_ptr<tcp::socket> socket);
    void handleRead(shared_ptr<tcp::socket> socket, const boost::system::error_code& error, size_t bytes_transferred);
    void removeConnection(shared_ptr<tcp::socket> socket);

    function<void(const vector<uint8_t>& data, shared_ptr<tcp::socket> socket)> messageHandler;
    function<void(shared_ptr<tcp::socket> socket)> openHandler;
    function<void(shared_ptr<tcp::socket> socket)> closeHandler;

    SignalCallback m_signalCallback; 

    boost::asio::io_context& m_io_context;
    tcp::acceptor m_acceptor;

    set<shared_ptr<tcp::socket>> m_connections;
    mutex m_connectionsMutex;

    static const size_t BUFFER_SIZE = 8192;
    array<uint8_t, BUFFER_SIZE> m_buffer;
};

#endif