#ifndef __SERVERPACKET__
#define __SERVERPACKET__

#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

enum ServerCategories {

SC_EVENT,
SC_RESPONSE

};

enum ServerTypes{

    ST_REGISTER,
    ST_LOGIN,
    ST_DATA,

    ST_NEWMESSAGE,
    ST_UPDATEUSERLIST,

    ST_SSP

};

struct serverPacket{
    websocketpp::connection_hdl hdl;
    int category = -1;
    int type = -1;
    int request_id = -1;
    std::string from;
    std::string to;
    ordered_json data;

    static serverPacket createLoginRPS(websocketpp::connection_hdl hdl,std::string to,std::string rps){
        serverPacket p;
        p.hdl = hdl;
        p.category = SC_RESPONSE;
        p.type = ST_LOGIN;
        p.request_id = 0;
        p.from = "SERVER";
        p.to = to;
        p.data["response"] = rps; // первые n цифр тип операции, последние 3 - код ошибки
        return p;
    }

    static serverPacket createRegisterRPS(websocketpp::connection_hdl hdl,std::string to,std::string rps){
        serverPacket p;
        p.hdl = hdl;
        p.category = SC_RESPONSE;
        p.type = ST_REGISTER;
        p.request_id = 0;
        p.from = "SERVER";
        p.to = to;
        p.data["response"] = rps; // первые n цифр тип операции, последние 3 - код ошибки
        return p;
    }

    static serverPacket createDataRPS(int request_id,websocketpp::connection_hdl& hdl,std::string& to,ordered_json& data,std::string rps){
        serverPacket p;
        p.hdl = hdl;
        p.category = SC_RESPONSE;
        p.type = ST_DATA;
        p.request_id = request_id;
        p.from = "SERVER";
        p.to = to;
        p.data["response"] = rps;
        p.data["data"] = data; 
        return p;
    }
    static serverPacket createNewMessageEVENT(websocketpp::connection_hdl& hdl,std::string& to,ordered_json& data){
        serverPacket p;
        p.hdl = hdl;
        p.category = SC_EVENT;
        p.type = ST_NEWMESSAGE;
        p.from = "SERVER";
        p.to = to;
        p.data = data;
        return p;
    }
    static serverPacket createUpdateUserListPacketEVENT_B(ordered_json& data){
        serverPacket p;
        p.category = SC_EVENT;
        p.type = ST_UPDATEUSERLIST;
        p.from = "SERVER";
        p.data = data;
        return p;
    }
    static serverPacket createSSPPacket(const std::string& to,ordered_json& data){
        serverPacket p;
        // p.hdl = hdl;
        p.category = SC_EVENT;
        p.type = ST_SSP;
        p.from = "SERVER";
        p.to = to;
        p.data = data;
        return p;
    }
};

#endif