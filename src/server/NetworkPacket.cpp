#include "NetworkPacket.h"
#include <cstring>

NetworkPacket::NetworkPacket()
    : magicNumber(0x7E), version(1), commandId(0), statusCode(0), payloadLength(0), checksum(0) {
    checksum = calculateChecksum();
}

// Numeric Constructor
NetworkPacket::NetworkPacket(uint16_t cmd, uint16_t status)
    : magicNumber(0x7E), version(1), commandId(cmd), statusCode(status), payloadLength(0), checksum(0) {
    checksum = calculateChecksum();
}

// String Constructor - FIXES C2665
NetworkPacket::NetworkPacket(uint16_t cmd, const std::string& payloadStr)
    : magicNumber(0x7E), version(1), commandId(cmd), statusCode(0) {
    setPayload(payloadStr.c_str(), static_cast<uint32_t>(payloadStr.length()));
}

void NetworkPacket::setCommandId(uint16_t cmd) {
    commandId = cmd;
    checksum = calculateChecksum();
}

void NetworkPacket::setStatusCode(uint16_t status) {
    statusCode = status;
    checksum = calculateChecksum();
}

void NetworkPacket::setPayload(const char* data, uint32_t length) {
    if (data && length > 0) {
        payload.assign(data, data + length);
        payloadLength = length;
    }
    else {
        payload.clear();
        payloadLength = 0;
    }
    checksum = calculateChecksum();
}

uint16_t NetworkPacket::calculateChecksum() const {
    // Using uint32_t for internal math to prevent overflow before the modulo
    uint32_t sum = magicNumber + version + commandId + statusCode + payloadLength;
    for (char c : payload) {
        sum += static_cast<uint8_t>(c);
    }
    return static_cast<uint16_t>(sum % 65536);
}

bool NetworkPacket::isValid() const {
    return (magicNumber == 0x7E && version == 1 && checksum == calculateChecksum());
}

char* NetworkPacket::serialize(uint32_t& outSize) const {
    outSize = 10 + payloadLength + 2; // Header(10) + Body + Checksum(2)
    char* buffer = new char[outSize];

    buffer[0] = magicNumber;
    buffer[1] = version;
    std::memcpy(buffer + 2, &commandId, 2);
    std::memcpy(buffer + 4, &statusCode, 2);
    std::memcpy(buffer + 6, &payloadLength, 4);

    if (payloadLength > 0) {
        std::memcpy(buffer + 10, payload.data(), payloadLength);
    }

    uint16_t cs = calculateChecksum();
    std::memcpy(buffer + 10 + payloadLength, &cs, 2);

    return buffer;
}

bool NetworkPacket::deserialize(const char* data, uint32_t size) {
    if (!data || size < 12) return false;

    const uint8_t originalMagicNumber = magicNumber;
    const uint8_t originalVersion = version;
    const uint16_t originalCommandId = commandId;
    const uint16_t originalStatusCode = statusCode;
    const uint32_t originalPayloadLength = payloadLength;
    const std::vector<char> originalPayload = payload;
    const uint16_t originalChecksum = checksum;

    uint8_t parsedMagicNumber = data[0];
    uint8_t parsedVersion = data[1];
    uint16_t parsedCommandId = 0;
    uint16_t parsedStatusCode = 0;
    uint32_t parsedPayloadLength = 0;
    uint16_t parsedChecksum = 0;
    std::vector<char> parsedPayload;

    std::memcpy(&parsedCommandId, data + 2, 2);
    std::memcpy(&parsedStatusCode, data + 4, 2);
    std::memcpy(&parsedPayloadLength, data + 6, 4);

    // Bounds check to prevent buffer overflow (use 64-bit math to avoid overflow wrap).
    const uint64_t requiredSize = 10ULL + static_cast<uint64_t>(parsedPayloadLength) + 2ULL;
    if (requiredSize > static_cast<uint64_t>(size)) return false;

    if (parsedPayloadLength > 0) {
        parsedPayload.assign(data + 10, data + 10 + parsedPayloadLength);
    }

    std::memcpy(&parsedChecksum, data + 10 + parsedPayloadLength, 2);

    magicNumber = parsedMagicNumber;
    version = parsedVersion;
    commandId = parsedCommandId;
    statusCode = parsedStatusCode;
    payloadLength = parsedPayloadLength;
    payload = parsedPayload;
    checksum = parsedChecksum;

    const bool valid = isValid();
    if (valid) {
        return true;
    }

    magicNumber = originalMagicNumber;
    version = originalVersion;
    commandId = originalCommandId;
    statusCode = originalStatusCode;
    payloadLength = originalPayloadLength;
    payload = originalPayload;
    checksum = originalChecksum;
    return false;
}
