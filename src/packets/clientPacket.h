#ifndef __CLIENTPACKET__
#define __CLIENTPACKET__

#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;


enum ClientCategories{

    CC_EVENT,
    CC_REQUEST

};

enum ClientTypes{

    CT_REGISTER,
    CT_LOGIN,
    CT_DATA,
    CT_NEWMESSAGE,

    CT_SSP
};

struct clientPacket{
    websocketpp::connection_hdl hdl;
    int category = -1;
    int type = -1;
    int request_id = -1;
    std::string from;
    std::string to;
    ordered_json data;
    
    static clientPacket getPacketFromJson(websocketpp::connection_hdl& r_hdl,ordered_json& r_j){
        clientPacket pack;
        pack.hdl = r_hdl;
        pack.category = r_j["category"];
        pack.type = r_j["type"];
        pack.request_id = r_j["request_id"];
        pack.from = r_j["from"];
        pack.to = r_j["to"];
        pack.data = r_j["data"];    
        return pack;
    }

private:
    clientPacket() = default;
    
};

#endif