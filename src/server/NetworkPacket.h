#pragma once

class NetworkPacket {
private:
    unsigned char magicNumber;
    unsigned char version;
    unsigned short commandId;
    unsigned short statusCode;
    unsigned int payloadLength;
    char* payload;
    unsigned short checksum;

public:
    NetworkPacket();
    NetworkPacket(unsigned short cmd, unsigned short status);
    NetworkPacket(const NetworkPacket& other);
    NetworkPacket& operator=(const NetworkPacket& other);
    ~NetworkPacket();

    void setCommandId(unsigned short cmd);
    void setStatusCode(unsigned short status);
    void setPayload(const char* data, unsigned int length);

    unsigned char getMagicNumber() const;
    unsigned char getVersion() const;
    unsigned short getCommandId() const;
    unsigned short getStatusCode() const;
    unsigned int getPayloadLength() const;
    const char* getPayload() const;
    unsigned short getChecksum() const;

    unsigned short calculateChecksum() const;
    bool isValid() const;
    char* serialize(unsigned int& outSize) const;
    bool deserialize(const char* data, unsigned int size);
};