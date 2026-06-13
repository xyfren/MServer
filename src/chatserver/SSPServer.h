#ifndef SESSION_SERVER_H
#define SESSION_SERVER_H

#include <map>
#include <string>
#include <iostream>
#include <websocketpp/server.hpp>
#include <boost/signals2.hpp>

#include "SSP.h"
#include "common/Constants.h"
#include "SDP.h"
#include "packets/ClientPacket.h"
#include "packets/ServerPacket.h"
#include "turnserver/TurnServer.h"

using namespace std;

using CallState = SSP::Call::CallState;
using SSPType = SSP::SSPType;
using Call = SSP::Call;

class SSPServer {

public:
    SSPServer();

    void addCall(SSP::Call call);
    void removeCall(int callId);
    bool callExists(int callId);
    void deauthenticateUser(const string& username);

    void handleSSPMessage(const clientPacket &packet);

    void onSessionStarted(int callId);

    // ->WebSocketServer
    boost::signals2::signal<void(const serverPacket & packet)> sendPacket;
    // ->TurnServer
    boost::signals2::signal<void(int callId, uint32_t mediaSessionId)> createMediaSession;
    boost::signals2::signal<void(int callId)> deleteMediaSession;
    boost::signals2::signal<void(int callId, uint32_t ssrc, const string& username)> addCaller;
    boost::signals2::signal<void(int callId, uint32_t ssrc, const string& username)> addCallee;
    boost::signals2::signal<void(int callId)> startMediaSession;
    boost::signals2::signal<TurnServer::MediaSession&(int callId)> getMediaSession;

    boost::signals2::signal<uint32_t()> getMediaSessionId;
    boost::signals2::signal<uint32_t()> getSsrc;
      
private:
    int getNextId();

    int lastId;

    map<int ,SSP::Call> m_calls; // callId -> Call
    map<string,int> m_userCall;
};

#endif // SESSION_SERVER_H