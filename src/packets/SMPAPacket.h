#ifndef SMPAPACKET_H
#define SMPAPACKET_H

#include <stdint.h>
#include <cstring>
#include <vector>

using namespace std;

#pragma pack(push,1)
struct SMPAPacket{
    uint32_t infoFlags;
    uint16_t dataLength;
    uint16_t seqNum;
    uint32_t timestamp;
    uint32_t ssrc;
    uint8_t* payload; // Указатель на массив в динамической памяти

    // Конструктор по умолчанию
    SMPAPacket() : payload(nullptr) {
        infoFlags = 0;
        dataLength = 0;
        seqNum = 0;
        timestamp = 0;
        ssrc = 0;
        headerSize = sizeof(SMPAPacket) - sizeof(uint8_t*)- sizeof(size_t);
        // qDebug() << "headerSize: " << headerSize;
    }

    // Конструктор копирования
    SMPAPacket(const SMPAPacket& other) : payload(nullptr) {
        headerSize = other.headerSize;
        memcpy(this, &other, headerSize);

        if (other.payload && other.dataLength > 0) {
            payload = new uint8_t[other.dataLength];
            memcpy(payload, other.payload, other.dataLength);
        }
    }

    // Оператор присваивания
    SMPAPacket& operator=(const SMPAPacket& other) {
        if (this != &other) {
            // Освобождаем старую память
            if (payload) {
                delete[] payload;
                payload = nullptr;
            }

            // Копируем заголовок
            headerSize = other.headerSize;
            memcpy(this, &other, headerSize);

            // Копируем payload
            if (other.payload && other.dataLength > 0) {
                payload = new uint8_t[other.dataLength];
                memcpy(payload, other.payload, other.dataLength);
            }
        }
        return *this;
    }

    static uint32_t getSSRCFromBytes(const vector<uint8_t>& data) {
        if (data.size() < sizeof(SMPAPacket) - sizeof(uint8_t*) - sizeof(size_t)) {
            return 0; // или выбросить исключение
        }
        
        // SSRC находится после infoFlags (4 байта), dataLength (2 байта), seqNum (2 байта), timestamp (4 байта)
        // Общее смещение: 4 + 2 + 2 + 4 = 12 байт
        const size_t ssrcOffset = 12;
        
        uint32_t ssrc;
        memcpy(&ssrc, data.data() + ssrcOffset, sizeof(uint32_t));
        return ssrc;
    }

private:
    size_t headerSize;
};
#pragma pack(pop)

#endif