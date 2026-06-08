#ifndef TURN_SERVER_H
#define TURN_SERVER_H

#include "SignalingServer.h"
#include "MediaServer.h"
#include "common/DBManager.h"
#include "packets/SMPAPacket.h"
#include "packets/SMPSPacket.h"

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <map>
#include <string>
#include <memory>
#include <random>

using namespace std;


class TurnServer {
public:
     struct UserConnection {
        enum class State{
            No, Authorized, Started, Calling
        };

        State state = State::No;
        boost::asio::ip::udp::endpoint userEndpoint;
        shared_ptr<tcp::socket> userSocket;
        string address;
        string username;
        uint32_t ssrc = 0;
        bool udpConnected = false;
    };
    
    struct MediaSession {
        bool started = false;
        int callId = 0;
        uint32_t mediaSessionId = 0;
        UserConnection callerConnection;
        UserConnection calleeConnection;
    };

    TurnServer(const shared_ptr<DBManager>& p_dbManger);

    void run(uint16_t mediaPort, uint16_t signalingPort);

    void printMediaSession(uint32_t mediaSessionId);
    
    //SLOTS

    void createMediaSession(int callId, uint32_t mediaSessionId);
    void deleteMediaSession(int callId);
    void addCaller(int callId, uint32_t ssrc, const string& username);
    void addCallee(int callId, uint32_t ssrc, const string& username);
    void startMediaSession(int callId);

    MediaSession& getMediaSession(int callId);

    uint32_t getMediaSessionId();
    uint32_t getSsrc();

    boost::signals2::signal<void(int callId)> sessionStarted;

private:
    void handleSignalingMessage(const vector<uint8_t>& data, shared_ptr<tcp::socket> fromSocket);
    void handleMediaData(const vector<uint8_t>& data, const boost::asio::ip::udp::endpoint& fromEndpoint);
    
    shared_ptr<DBManager> m_dbManager;

    boost::asio::io_context m_ioContext;
    unique_ptr<SignalingServer> m_signalingServer;
    unique_ptr<MediaServer> m_mediaServer;
    
    // map<shared_ptr<tcp::socket>, string, owner_less<shared_ptr<tcp::socket>> > m_socketToUser;
    // map<boost::asio::ip::udp::endpoint, string> m_endpointToUser;
    map<int,uint32_t> m_idPairs; // callId -> mediaSessionId

    map<uint32_t, MediaSession> m_mediaSessions; // mediaSessionId -> MediaSession
    map<uint32_t, uint32_t> m_mediaSessionParticipants; // ssrc -> mediaSessionId

    uint32_t lastSsrc;
    uint32_t lastMediaSessionId;
};

#endif