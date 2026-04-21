#pragma once

#include <cstdint>
#include <vector>
#include <string>

/**
 * @class NetworkPacket
 * @brief Represents a structured network packet for communication.
 *
 * Encapsulates packet fields including header, payload, and checksum.
 * Provides serialization and deserialization for transmission.
 */
class NetworkPacket {
private:
    uint8_t magicNumber;        ///< Packet identifier
    uint8_t version;            ///< Protocol version
    uint16_t commandId;         ///< Command identifier
    uint16_t statusCode;        ///< Status code
    uint32_t payloadLength;     ///< Length of payload
    std::vector<char> payload;  ///< Payload data
    uint16_t checksum;          ///< Packet checksum

public:

    /**
     * @brief Default constructor.
     */
    NetworkPacket();

    /**
     * @brief Constructs packet with command and status.
     */
    NetworkPacket(uint16_t cmd, uint16_t status);

    /**
     * @brief Constructs packet with command and string payload.
     */
    NetworkPacket(uint16_t cmd, const std::string& payloadStr);

    // Getters
    uint8_t getMagicNumber() const { return magicNumber; }
    uint8_t getVersion() const { return version; }
    uint16_t getCommandId() const { return commandId; }
    uint16_t getStatusCode() const { return statusCode; }
    uint32_t getPayloadLength() const { return payloadLength; }
    const char* getPayload() const { return payload.empty() ? nullptr : payload.data(); }
    uint16_t getChecksum() const { return checksum; }

    /**
     * @brief Sets command ID.
     */
    void setCommandId(uint16_t cmd);

    /**
     * @brief Sets status code.
     */
    void setStatusCode(uint16_t status);

    /**
     * @brief Sets payload data.
     */
    void setPayload(const char* data, uint32_t length);

    /**
     * @brief Calculates checksum for integrity verification.
     * @return Computed checksum
     */
    uint16_t calculateChecksum() const;

    /**
     * @brief Validates packet integrity.
     * @return true if packet is valid, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Serializes packet into binary format.
     * @param outSize Output buffer size
     * @return Pointer to allocated buffer
     */
    char* serialize(uint32_t& outSize) const;

    /**
     * @brief Deserializes binary data into packet.
     * @param data Input buffer
     * @param size Buffer size
     * @return true if successful, false otherwise
     */
    bool deserialize(const char* data, uint32_t size);
};