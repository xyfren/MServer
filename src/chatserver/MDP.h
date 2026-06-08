#ifndef MDP_H
#define MDP_H

#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

#pragma pack(push,1)
struct MediaConfig{
    uint16_t sampleRate = 0;
    uint8_t channelCount = 0;
};
#pragma pack(pop)

inline MediaConfig createMediaConfigStruct(uint16_t sampleRate, uint8_t channelCount){
    MediaConfig mediaConfig;

    mediaConfig.sampleRate = sampleRate;
    mediaConfig.channelCount = channelCount;

    return mediaConfig;
}

inline ordered_json createMediaConfigJson(uint16_t sampleRate, uint8_t channelCount){
    ordered_json j;
    
    j["sampleRate"] = sampleRate;
    j["channelCount"] = channelCount;
    
    return j;
}

#endif