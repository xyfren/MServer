#ifndef SSP_H
#define SSP_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

using namespace std;

class SSP
{
public:
    enum SSPType{SSP_Invite,SSP_Stop,SSP_Trying,SSP_Ringing,SSP_Accept,SSP_Reject,SSP_Ack,SSP_Error,SSP_End};
    enum SSPReceiver{SSP_CallerSide,SSP_CalleeSide};
    struct Call{
        enum CallState{Call_NoState,Call_Started,Call_Trying,Call_Ringing,Call_Confirming,Call_Preparing,Call_Talking,Call_Stoped};
        
        int state = Call_NoState;
        int id = -1;
        string from = "";
        string to = "";
        uint64_t startTime1 = 0;
        uint64_t startTime2 = 0;

        void setState(int p_state){
            state = p_state;
        }
        
        string getCallStateString() const {
            switch(state) {
            case Call_NoState: return "NoState";
            case Call_Started: return "Started";
            case Call_Trying: return "Trying";
            case Call_Ringing: return "Ringing";
            case Call_Confirming: return "Confirming";
            case Call_Preparing: return "Preparing";
            case Call_Talking: return "Calling";
            case Call_Stoped: return "Stoped";
            default: return "UnknownState";
            }
        }

        string getStringCall(){
            string s =  "State: " + getCallStateString() + "\n" +
                        "Id: " + to_string(id) + "\n" +
                        "From: " + from + "\n" +
                        "To: " + to + "\n" +
                        "StartTime1 " + to_string(startTime1) + "\n";

            return s;
        }
    };

    SSP() = delete;

    static ordered_json createTryingCallJson(Call call){
        ordered_json j;
        j["Type"] = SSP_Trying;
        j["CallID"] = call.id;
        j["From"] = "SERVER";
        j["To"] = call.from;
        j["SDP"] = "MM";
        j["StartTime1"] = call.startTime1;
        return j;
    }
    static ordered_json createErrorCallJson(SSPReceiver receiver, string message, Call call){
        ordered_json j;
        j["Type"] = SSP_Error;
        j["CallID"] = call.id;
        if (receiver == SSPReceiver::SSP_CalleeSide){
            j["From"] = "SERVER";
            j["To"] = call.to;
        }
        else if (receiver == SSPReceiver::SSP_CallerSide) {
            j["From"] = "SERVER";
            j["To"] = call.from;
        }
        j["Msg"] = "Error: " + message;
        return j;
    }

    static Call createCall(int id, string from, string to, uint64_t startTime1){
        Call call;
        call.state = Call::Call_NoState;
        call.id = id;
        call.from = from;
        call.to = to;
        call.startTime1 = startTime1;
        return call;
    }
};

#endif // SSP_H