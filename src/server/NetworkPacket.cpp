#include "NetworkPacket.h"
#include <cstring>

/**
 * @brief Default constructor for NetworkPacket.
 *
 * Initializes packet with default values and computes checksum.
 */
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

/**
 * @brief Constructs a packet with command and status.
 *
 * @param cmd Command ID
 * @param status Status code
 */
void NetworkPacket::setCommandId(uint16_t cmd) {
    commandId = cmd;
    checksum = calculateChecksum();
}


/**
 * @brief Constructs a packet with command and string payload.
 *
 * @param cmd Command ID
 * @param payloadStr Payload data as string
 */
void NetworkPacket::setStatusCode(uint16_t status) {
    statusCode = status;
    checksum = calculateChecksum();
}

/**
 * @brief Sets packet payload data.
 *
 * Updates payload and recalculates checksum.
 *
 * @param data Pointer to payload data
 * @param length Length of payload
 */
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

/**
 * @brief Calculates checksum for packet integrity.
 *
 * Combines header fields and payload bytes to generate checksum.
 *
 * @return Computed checksum value
 */
uint16_t NetworkPacket::calculateChecksum() const {
    // Using uint32_t for internal math to prevent overflow before the modulo
    uint32_t sum = magicNumber + version + commandId + statusCode + payloadLength;
    for (char c : payload) {
        sum += static_cast<uint8_t>(c);
    }
    return static_cast<uint16_t>(sum % 65536);
}

/**
 * @brief Validates packet integrity.
 *
 * Checks magic number, version, and checksum.
 *
 * @return true if packet is valid, false otherwise
 */
bool NetworkPacket::isValid() const {
    return (magicNumber == 0x7E && version == 1 && checksum == calculateChecksum());
}

/**
 * @brief Serializes packet into binary format.
 *
 * Converts packet fields into a byte buffer for transmission.
 *
 * @param outSize Output size of serialized data
 * @return Pointer to allocated buffer (must be freed by caller)
 */
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

/**
 * @brief Deserializes binary data into packet.
 *
 * Parses raw buffer, reconstructs packet fields, and validates checksum.
 *
 * @param data Input buffer
 * @param size Buffer size
 * @return true if successful, false otherwise
 */
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

    // Bounds check to prevent buffer overflow
    if (parsedPayloadLength > (size - 12)) {
        return false;
    }

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
