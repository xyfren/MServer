#include "TurnServer.h"
TurnServer::TurnServer(const shared_ptr<DBManager>& p_dbManger){
    m_dbManager = p_dbManger;
    lastSsrc = 1;
    lastMediaSessionId = 1;
    
}

void TurnServer::run(uint16_t mediaPort,uint16_t signalingPort){
    try {
        m_ioContext;
        m_mediaServer = make_unique<MediaServer>(m_ioContext);
        m_mediaServer->setMediaDataCallback(
            bind(&TurnServer::handleMediaData, this, 
                    placeholders::_1, placeholders::_2)
        );
        m_mediaServer->run(mediaPort);
        

        m_signalingServer = make_unique<SignalingServer>(m_ioContext);
        m_signalingServer->setSignalCallback(
            bind(&TurnServer::handleSignalingMessage, this, 
                    placeholders::_1, placeholders::_2)
        );
        m_signalingServer->run(signalingPort);

        m_ioContext.run();
    }
    catch(const exception& ex){
        cout << "Error running TurnServer: " << ex.what() << endl;
    }
}

void TurnServer::handleSignalingMessage(const vector<uint8_t>& data, shared_ptr<tcp::socket> fromSocket){
    cout << "SIGNALING HANDLER" << endl;
    uint32_t ssrc = SMPSPacket::getSSRCFromBytes(data);
    cout << "ssrc: " << ssrc << endl;
    uint32_t mediaSessionId = m_mediaSessionParticipants[ssrc];
    cout << "mediaSessionId: " << mediaSessionId << endl;
    if (m_mediaSessions.find(mediaSessionId) == m_mediaSessions.end()){
        cout << "n "<< endl;
        return;
    }

    MediaSession& mediaSession = m_mediaSessions[mediaSessionId];

    SMPSPacket packet(data);

    if (packet.type == SMPS_No){
        cout << "Signal packet: " << "No" << endl;
    }

    else if (packet.type == SMPS_SyncTime) {
        cout << "Signal packet: " << "SyncTime" << endl;

        if (mediaSession.calleeConnection.ssrc == ssrc){
            m_signalingServer->send(data,mediaSession.callerConnection.userSocket);
        }
        else if (mediaSession.callerConnection.ssrc == ssrc){
            m_signalingServer->send(data,mediaSession.calleeConnection.userSocket);
        }
    }

    else if (packet.type == SMPS_MediaConfig){
        cout << "Signal packet: " << "MediaConfig" << endl;

        if (mediaSession.calleeConnection.ssrc == ssrc){
            m_signalingServer->send(data,mediaSession.callerConnection.userSocket);
        }
        else if (mediaSession.callerConnection.ssrc == ssrc){
            m_signalingServer->send(data,mediaSession.calleeConnection.userSocket);
        }
    }

    else if (packet.type == SMPS_Start){
        cout << "Signal packet: " << "Start" << endl;
        
        if (mediaSession.calleeConnection.state == UserConnection::State::Authorized && 
            mediaSession.callerConnection.state == UserConnection::State::Authorized && 
            mediaSession.calleeConnection.ssrc == ssrc)
        {
            cout << "user: callee" << endl;

            mediaSession.calleeConnection.state = UserConnection::State::Started;
            m_signalingServer->send(data,mediaSession.callerConnection.userSocket);

        }
        else if(
            mediaSession.calleeConnection.state == UserConnection::State::Started && 
            mediaSession.callerConnection.state == UserConnection::State::Authorized &&     
            mediaSession.callerConnection.ssrc == ssrc) 
        {
            cout << "user: caller" << endl;
            
            mediaSession.callerConnection.state = UserConnection::State::Started;

            mediaSession.started = true;
            sessionStarted(mediaSession.callId);

            m_signalingServer->send(data,mediaSession.calleeConnection.userSocket);
        }
    }

    else if (packet.type == SMPS_Hi){
        cout << endl;
        cout << "Signal packet: " << "Hi" << endl;
        cout << packet.data.hi.callId << endl;
        cout << packet.data.hi.mediaSessionId << endl;
        cout << packet.data.hi.username << endl;
        cout << endl;

        if (mediaSession.callerConnection.ssrc == ssrc){
            cout << "user: caller" << endl;
            mediaSession.callerConnection.userSocket = fromSocket;
            mediaSession.callerConnection.state = UserConnection::State::Authorized;

        }
        else if (mediaSession.calleeConnection.ssrc == ssrc) {
            cout << "user: callee" << endl;
            mediaSession.calleeConnection.userSocket = fromSocket;
            mediaSession.calleeConnection.state = UserConnection::State::Authorized;
        }
    }

    else { 
        cout << "Signal packet: " << "Unknown type";
    }
}

void TurnServer::handleMediaData(const vector<uint8_t>& data, const boost::asio::ip::udp::endpoint& fromEndpoint){
    uint32_t ssrc = SMPAPacket::getSSRCFromBytes(data);

    uint32_t mediaSessionId = m_mediaSessionParticipants[ssrc];
    
    if (m_mediaSessions.find(mediaSessionId) == m_mediaSessions.end()){
        return;
    }
    
    MediaSession& mediaSession = m_mediaSessions[mediaSessionId];
    if (mediaSession.started){
        if (mediaSession.callerConnection.udpConnected && mediaSession.calleeConnection.udpConnected){
            if (mediaSession.callerConnection.ssrc == ssrc){
                m_mediaServer->send(data,mediaSession.calleeConnection.userEndpoint);
            }

            else if (mediaSession.calleeConnection.ssrc == ssrc){
                m_mediaServer->send(data,mediaSession.callerConnection.userEndpoint);
            }
        }
        else{ // нужно сохранить регистрацию места отправления
            cout << "Register user" << endl; 

            if (!mediaSession.callerConnection.udpConnected and mediaSession.callerConnection.ssrc == ssrc){
                cout << "user 1" << endl;
                mediaSession.callerConnection.userEndpoint = fromEndpoint;
                mediaSession.callerConnection.udpConnected = true;
            }
            else if (!mediaSession.calleeConnection.udpConnected and mediaSession.calleeConnection.ssrc == ssrc) {
                cout << "user 2" << endl;
                mediaSession.calleeConnection.userEndpoint = fromEndpoint;
                mediaSession.calleeConnection.udpConnected = true;
            }
            
        }
    }
    
}

void TurnServer::createMediaSession(int callId, uint32_t mediaSessionId){ 
    MediaSession mediaSession;
    mediaSession.callId = callId;
    mediaSession.mediaSessionId = mediaSessionId;

    m_idPairs[callId] = mediaSessionId;
    m_mediaSessions[mediaSessionId] = mediaSession;
    printMediaSession(mediaSessionId);
}

void TurnServer::deleteMediaSession(int callId){
    uint32_t mediaSessionId = m_idPairs[callId];
    MediaSession mediaSession = m_mediaSessions[mediaSessionId];
    
    m_mediaSessionParticipants.erase(mediaSession.callerConnection.ssrc);
    m_mediaSessionParticipants.erase(mediaSession.calleeConnection.ssrc);
    m_mediaSessions.erase(mediaSessionId);
    m_idPairs.erase(callId);
}

void TurnServer::addCaller(int callId, uint32_t ssrc, const string& username){
    uint32_t mediaSessionId = m_idPairs[callId];
    MediaSession & mediaSession = m_mediaSessions[mediaSessionId];

    mediaSession.callerConnection.ssrc = ssrc;
    mediaSession.callerConnection.username = username;
    m_mediaSessionParticipants[ssrc] = mediaSessionId;
    printMediaSession(mediaSessionId);
}

void TurnServer::addCallee(int callId, uint32_t ssrc, const string& username){
    uint32_t mediaSessionId = m_idPairs[callId];
    MediaSession & mediaSession = m_mediaSessions[mediaSessionId];

    mediaSession.calleeConnection.ssrc = ssrc;
    mediaSession.calleeConnection.username = username;
    
    m_mediaSessionParticipants[ssrc] = mediaSessionId;
    printMediaSession(mediaSessionId);
}

void TurnServer::startMediaSession(int callId){
    uint32_t mediaSessionId = m_idPairs[callId];
    MediaSession & mediaSession = m_mediaSessions[mediaSessionId];
    
    mediaSession.started = true;
    printMediaSession(mediaSessionId);
}

void TurnServer::printMediaSession(uint32_t mediaSessionId){
    MediaSession & mediaSession = m_mediaSessions[mediaSessionId];
   
    stringstream info; 
    info << "Media Session" << endl;
    info << "started: " << mediaSession.started << endl;
    info << "MediaSessionId: " << mediaSession.mediaSessionId << endl;
    info << "CallId: " << mediaSession.callId << endl;
    info << "CallerSsrc: " << mediaSession.callerConnection.ssrc << endl;
    info << "CalleeSsrc: " << mediaSession.calleeConnection.ssrc << endl;
    info << "CallerConnected: " << mediaSession.callerConnection.udpConnected << endl;
    info << "CalleeConnected: " << mediaSession.calleeConnection.udpConnected << endl;
    cout << info.str();
}

TurnServer::MediaSession& TurnServer::getMediaSession(int callId){
    uint32_t mediaSessionId = m_idPairs[callId];
    return m_mediaSessions[mediaSessionId];
    cout << "Session started" << endl;
}

uint32_t TurnServer::getMediaSessionId(){
    return lastMediaSessionId++;
}

uint32_t TurnServer::getSsrc(){
    return lastSsrc++;
}