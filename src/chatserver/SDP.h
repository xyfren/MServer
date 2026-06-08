#ifndef SDP_H
#define SDP_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

using namespace std;

#pragma pack(push,1)
struct SyncTime{
    uint32_t startTimestamp = 0;
    uint64_t startTime = 0;
};
#pragma pack(pop)

inline ordered_json createSDPJson(uint32_t mediaSessionId, uint32_t ssrc, const string& host){
    ordered_json j;
    j["mediaSessionId"] = mediaSessionId;
    j["ssrc"] = ssrc;
    j["host"] = host;
    return j;
}

#endif