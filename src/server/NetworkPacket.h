#pragma once
#include <cstdint>
#include <vector>
#include <string>

class NetworkPacket {
private:
    uint8_t magicNumber;
    uint8_t version;
    uint16_t commandId;
    uint16_t statusCode;
    uint32_t payloadLength;
    std::vector<char> payload;
    uint16_t checksum;

public:
    NetworkPacket();
    NetworkPacket(uint16_t cmd, uint16_t status);

    // Getters - Names matched exactly to your Tests and Manager
    uint8_t getMagicNumber() const { return magicNumber; }
    uint8_t getVersion() const { return version; }
    uint16_t getCommandId() const { return commandId; }
    uint16_t getStatusCode() const { return statusCode; }
    uint32_t getPayloadLength() const { return payloadLength; }
    const char* getPayload() const { return payload.data(); }
    uint16_t getChecksum() const { return checksum; }

    // Setters
    void setCommandId(uint16_t cmd);
    void setStatusCode(uint16_t status);
    void setPayload(const char* data, uint32_t length);

    // Core Logic
    uint16_t calculateChecksum() const;
    bool isValid() const;

    // Compatibility methods for NetworkManager
    char* serialize(uint32_t& outSize) const;
    bool deserialize(const char* data, uint32_t size);
};