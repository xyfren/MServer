#include "SSPServer.h"

SSPServer::SSPServer() {
    lastId = 1;
}

void SSPServer::addCall(SSP::Call call) {
    m_calls[call.id] = call;
    m_userCall[call.to] = call.id;
    m_userCall[call.from] = call.id;
}

void SSPServer::removeCall(int callId) {
    m_calls.erase(callId);
}

bool SSPServer::callExists(int callId) {
    return m_calls.find(callId) != m_calls.end();
}

void SSPServer::deauthenticateUser(const string& username){
    auto userCallIt = m_userCall.find(username);
    if (userCallIt != m_userCall.end()) {
        int callId = userCallIt->second;
        auto callsIt = m_calls.find(callId);
        if (callsIt != m_calls.end()) {
            Call& call = callsIt->second;
            if (call.to == username) {
                ordered_json j = SSP::createErrorCallJson(SSP::SSPReceiver::SSP_CallerSide, "Connection error", call);
                serverPacket srvPacket = serverPacket::createSSPPacket(call.from, j);
                sendPacket(srvPacket);
            }
            else if (call.from == username) {
                ordered_json j = SSP::createErrorCallJson(SSP::SSPReceiver::SSP_CalleeSide, "Connection error", call);
                serverPacket srvPacket = serverPacket::createSSPPacket(call.to, j);
                sendPacket(srvPacket);
            }
            deleteMediaSession(callId);
            m_userCall.erase(call.to);
            m_userCall.erase(call.from);

            m_calls.erase(callId);
        }
        else{
            m_userCall.erase(username);
        }
    }

}

int SSPServer::getNextId(){
    return lastId++;
}

void SSPServer::onSessionStarted(int callId){
    m_calls[callId].state == Call::Call_Started;
}


void SSPServer::handleSSPMessage(const clientPacket &packet){
    try{
        websocketpp::connection_hdl ClientHdl = packet.hdl;
        int Type = packet.type;
        string From = packet.from;
        string To = packet.to;
        ordered_json Data = packet.data;

        int sspType = Data["Type"];
        int sspCallId = Data["CallID"];
        string sspFrom = Data["From"];
        string sspTo = Data["To"];
        

        if (To != "SERVER"){
            return;
        }

        switch (sspType)
        {
        case SSPType::SSP_Invite:
            {
                cout << "INVITE HANDLER" << endl;

                if (m_userCall.find(sspTo) != m_userCall.end()){
                    Call call = SSP::createCall(0,sspFrom,sspTo,0);
                    ordered_json j = SSP::createErrorCallJson(SSP::SSPReceiver::SSP_CallerSide,"Линия занята",call);
                    serverPacket srvPacket = serverPacket::createSSPPacket(From,j);

                    sendPacket(srvPacket);
                    return;
                }

                int newCallId = getNextId();
                
                int sspStartTime1 = stoull(string(Data["SDP"]["StartTime1"]).c_str()); // обработать ошибку

                Call call = SSP::createCall(newCallId,sspFrom,sspTo,sspStartTime1);
                call.setState(SSP::SSP_Trying);

                addCall(call);
                
                ordered_json j = SSP::createTryingCallJson(call);
                serverPacket srvPacket = serverPacket::createSSPPacket(sspFrom,j);
                sendPacket(srvPacket);

                uint32_t mediaSessionId = getMediaSessionId().value_or(0);
                uint32_t ssrc = getSsrc().value_or(0);

                // create MediaSession
                createMediaSession(newCallId, mediaSessionId);
                addCallee(newCallId, ssrc, sspFrom);

                // add TurnServer info in InvitePacket
                ordered_json updatedData = Data;
                updatedData["CallID"] = newCallId;
                updatedData["SDP"]["mediaSessionId"] = mediaSessionId;
                updatedData["SDP"]["ssrc"] = ssrc;
                updatedData["SDP"]["host"] = SERVER_ADDRESS;       

                srvPacket = serverPacket::createSSPPacket(sspTo,updatedData);
                sendPacket(srvPacket);
            }
            break;
        case SSPType::SSP_Ringing:
            {    
                cout << "RINGING HANDLER" << endl;
                if (!(m_calls.find(sspCallId) != m_calls.end())){
                    return ;
                }

                Call& call = m_calls[sspCallId];

                if (!(sspTo == call.from and sspFrom == call.to and call.state == CallState::Call_Trying)){
                    return ;
                }
                call.state = CallState::Call_Ringing;

                serverPacket srvPacket = serverPacket::createSSPPacket(sspTo,Data);
                sendPacket(srvPacket);
            }
            break;
        case SSPType::SSP_Stop:
            {
                cout << "STOP HANDLER" << endl;

                if (m_calls.find(sspCallId) == m_calls.end()){
                    return ;
                }

                Call& call = m_calls[sspCallId];

                if (!(sspFrom == call.from and sspTo == call.to and call.state != CallState::Call_Trying)){
                    return ;
                }

                call.state = CallState::Call_Stoped;

                deleteMediaSession(call.id);

                m_userCall.erase(call.to);

                m_userCall.erase(call.from);
                m_calls.erase(call.id);

                serverPacket srvPacket = serverPacket::createSSPPacket(sspTo,Data);
                sendPacket(srvPacket);
            }
            break;
        case SSPType::SSP_Accept:
            {
                cout << "ACCEPT HANDLER" << endl;
                
                if (m_calls.find(sspCallId) == m_calls.end()){
                    return ;
                }

                Call& call = m_calls[sspCallId];

                if (!(sspTo == call.from and sspFrom == call.to and call.state == CallState::Call_Ringing)){
                    return ;
                }

                int sspStartTime2 = stoull(string(Data["SDP"]["StartTime2"]).c_str()); // обработать ошибку
                
                call.startTime2 = sspStartTime2;

                call.state = CallState::Call_Confirming;
                
                TurnServer::MediaSession& mediaSession = getMediaSession(call.id).value();
                
                if (mediaSession.mediaSessionId == 0){
                    return ;
                }

                uint32_t ssrc = getSsrc().value_or(0);

                addCaller(call.id,ssrc,sspFrom);

                // add TurnServer info in AcceptPacket 
                ordered_json updatedData = Data;
                updatedData["SDP"]["mediaSessionId"] = mediaSession.mediaSessionId;
                updatedData["SDP"]["ssrc"] = ssrc;
                updatedData["SDP"]["host"] = SERVER_ADDRESS;       
                serverPacket srvPacket = serverPacket::createSSPPacket(sspTo,updatedData);
                sendPacket(srvPacket);
            }
            break;
        case SSPType::SSP_Reject:
            {
                cout << "REJECT HANDLER" << endl;

                if (m_calls.find(sspCallId) == m_calls.end()){
                    return ;
                }

                Call& call = m_calls[sspCallId];

                if (!(sspTo == call.from and sspFrom == call.to and call.state == CallState::Call_Ringing)){
                    return ;
                }

                call.state = CallState::Call_Stoped;

                deleteMediaSession(call.id);

                m_userCall.erase(call.to);
                
                m_userCall.erase(call.from);
                m_calls.erase(call.id);

                serverPacket srvPacket = serverPacket::createSSPPacket(sspTo,Data);
                sendPacket(srvPacket);
            }
            break;
        case SSPType::SSP_Ack:
            {
                cout << "ACK HANDLER" << endl;
                
                if (m_calls.find(sspCallId) == m_calls.end()){
                    return ;
                }

                Call& call = m_calls[sspCallId];

                if (!(sspFrom == call.from and sspTo == call.to and call.state == CallState::Call_Confirming)){
                    return ;
                }

                call.state = CallState::Call_Preparing;

                serverPacket srvPacket = serverPacket::createSSPPacket(sspTo,Data);
                sendPacket(srvPacket);
            }
            break;
        case SSPType::SSP_Error:
            {
                cout << "ERROR HANDLER" << endl;
            }
            break;
        case SSPType::SSP_End:
            {
                cout << "END HANDLER" << endl;
                // deleteMediaSession
                if (m_calls.find(sspCallId) == m_calls.end()){
                    return ;
                }

                Call& call = m_calls[sspCallId];

                if (
                    !(
                    (((sspFrom == call.from) and (sspTo == call.to)) or 
                    ((sspFrom == call.to) and (sspTo == call.from))) and 
                    (call.state == CallState::Call_Preparing or call.state == CallState::Call_Talking)
                    )
                )
                {
                    return ;
                }

                call.state == CallState::Call_Stoped;
                
                deleteMediaSession(call.id);

                m_userCall.erase(call.to);
                
                m_userCall.erase(call.from);
                m_calls.erase(call.id);

                serverPacket srvPacket = serverPacket::createSSPPacket(sspTo,Data);
                sendPacket(srvPacket);
            }
            break;
        default:
            break;
        }
    }
    catch (exception &ex){
        cout << ex.what() << endl;
    }
}