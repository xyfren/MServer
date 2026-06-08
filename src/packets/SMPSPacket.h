#ifndef SMPSPACKET_H
#define SMPSPACKET_H

#include <stdint.h>
#include <cstring>
#include <vector>

#include "chatserver/MDP.h"
#include "chatserver/SDP.h"

using namespace std;

//DataTypes
enum SMPSTypes {SMPS_No,SMPS_Hi,SMPS_Start,SMPS_MediaConfig,SMPS_SyncTime};

#pragma pack(push,1)

struct Hi{
    uint32_t callId = 0;
    uint32_t mediaSessionId = 0;

    char username[32] = {0};
};

struct Start{
    uint32_t callId = 0;
    uint32_t mediaSessionId = 0;

    uint32_t startTimestamp = 0;
    uint64_t startTime = 0;

};

union SMPSPacketData{
    MediaConfig mediaConfig;
    SyncTime syncTime;
    Start start;
    Hi hi;
    SMPSPacketData() {}  // Дефолтный конструктор
    ~SMPSPacketData() {} // Деструктор
};


struct SMPSPacket{
    int type;
    uint32_t ssrcFrom; // По ssrc будет определять пользователя (не шифруем)
    uint32_t ssrcTo; // На всякий случай
    SMPSPacketData data; // Шифруем
    
    SMPSPacket(uint32_t p_ssrcFrom, uint32_t p_ssrcTo, MediaConfig p_mediaConfig): 
        ssrcFrom(p_ssrcFrom), ssrcTo(p_ssrcTo)
    {
        type = SMPS_MediaConfig;
        data.mediaConfig = p_mediaConfig;
    }

    //From bytes constructor
    SMPSPacket(const vector<uint8_t>& bytes)
    {
        if (bytes.size() >= sizeof(SMPSPacket)) {
            memcpy(this, bytes.data(), sizeof(SMPSPacket));
        }
        else {
            type = SMPS_No;
            ssrcFrom = 0;
            ssrcTo = 0;
        }
    }

    static uint32_t getSSRCFromBytes(const vector<uint8_t>& data) {
        if (data.size() < sizeof(SMPSPacket)) {
            return 0; // или выбросить исключение
        }
        
        // SSRC находится после SMPSType (4 байта),
        // Общее смещение: 4 байт
        const size_t ssrcOffset = 4;
        
        uint32_t ssrc;
        memcpy(&ssrc, data.data() + ssrcOffset, sizeof(uint32_t));
        return ssrc;
    }

};
#pragma pack(pop)

const int SMPSPacketSize = sizeof(SMPSPacket);

#endif