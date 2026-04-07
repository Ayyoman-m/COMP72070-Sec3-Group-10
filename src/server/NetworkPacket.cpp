#include "NetworkPacket.h"
#include <cstring>

// Default Constructor
NetworkPacket::NetworkPacket()
{
    // HEADER
    magicNumber = 0x7E;
    version = 1;
    commandId = 0;
    statusCode = 0;
    payloadLength = 0;

    // BODY
    payload = nullptr;

    // TAIL
    checksum = calculateChecksum();
}

// Parameterized Constructor
NetworkPacket::NetworkPacket(unsigned short cmd, unsigned short status)
{
    magicNumber = 0x7E;
    version = 1;
    commandId = cmd;
    statusCode = status;
    payloadLength = 0;
    payload = nullptr;
    checksum = calculateChecksum();
}

// Copy Constructor (Deep Copy)
NetworkPacket::NetworkPacket(const NetworkPacket& other)
{
    magicNumber = other.magicNumber;
    version = other.version;
    commandId = other.commandId;
    statusCode = other.statusCode;
    payloadLength = other.payloadLength;
    checksum = other.checksum;

    if (other.payload != nullptr && other.payloadLength > 0)
    {
        payload = new char[other.payloadLength];
        memcpy(payload, other.payload, other.payloadLength);
    }
    else
    {
        payload = nullptr;
    }
}

// Assignment Operator
NetworkPacket& NetworkPacket::operator=(const NetworkPacket& other)
{
    if (this != &other)
    {
        if (payload != nullptr)
        {
            delete[] payload;
        }

        magicNumber = other.magicNumber;
        version = other.version;
        commandId = other.commandId;
        statusCode = other.statusCode;
        payloadLength = other.payloadLength;
        checksum = other.checksum;

        if (other.payload != nullptr && other.payloadLength > 0)
        {
            payload = new char[other.payloadLength];
            memcpy(payload, other.payload, other.payloadLength);
        }
        else
        {
            payload = nullptr;
        }
    }
    return *this;
}

// Destructor
NetworkPacket::~NetworkPacket()
{
    if (payload != nullptr)
    {
        delete[] payload;
        payload = nullptr;
    }
}

// Setters
void NetworkPacket::setCommandId(unsigned short cmd)
{
    commandId = cmd;
    checksum = calculateChecksum();
}

void NetworkPacket::setStatusCode(unsigned short status)
{
    statusCode = status;
    checksum = calculateChecksum();
}

void NetworkPacket::setPayload(const char* data, unsigned int length)
{
    if (payload != nullptr)
    {
        delete[] payload;
    }

    payloadLength = length;
    if (data != nullptr && length > 0)
    {
        payload = new char[length];
        memcpy(payload, data, length);
    }
    else
    {
        payload = nullptr;
        payloadLength = 0;
    }
    checksum = calculateChecksum();
}

// Getters
unsigned char NetworkPacket::getMagicNumber() const { return magicNumber; }
unsigned char NetworkPacket::getVersion() const { return version; }
unsigned short NetworkPacket::getCommandId() const { return commandId; }
unsigned short NetworkPacket::getStatusCode() const { return statusCode; }
unsigned int NetworkPacket::getPayloadLength() const { return payloadLength; }
const char* NetworkPacket::getPayload() const { return payload; }
unsigned short NetworkPacket::getChecksum() const { return checksum; }

// Utility Functions
unsigned short NetworkPacket::calculateChecksum() const
{
    unsigned int sum = 0;
    sum += magicNumber;
    sum += version;
    sum += commandId;
    sum += statusCode;
    sum += payloadLength;

    for (unsigned int i = 0; payload != nullptr && i < payloadLength; i++)
    {
        sum += (unsigned char)payload[i];
    }

    return (unsigned short)(sum % 65536);
}

bool NetworkPacket::isValid() const
{
    if (magicNumber != 0x7E || version != 1) return false;
    return (checksum == calculateChecksum());
}

// Serialization
char* NetworkPacket::serialize(unsigned int& outSize) const
{
    unsigned short currentChecksum = calculateChecksum();
    outSize = 10 + payloadLength + 2; // Header(10) + Body + Tail(2)

    char* buffer = new char[outSize];
    int index = 0;

    buffer[index++] = magicNumber;
    buffer[index++] = version;
    memcpy(buffer + index, &commandId, sizeof(commandId));
    index += sizeof(commandId);
    memcpy(buffer + index, &statusCode, sizeof(statusCode));
    index += sizeof(statusCode);
    memcpy(buffer + index, &payloadLength, sizeof(payloadLength));
    index += sizeof(payloadLength);

    if (payload != nullptr && payloadLength > 0)
    {
        memcpy(buffer + index, payload, payloadLength);
        index += payloadLength;
    }

    memcpy(buffer + index, &currentChecksum, sizeof(currentChecksum));
    return buffer;
}

// Deserialization
bool NetworkPacket::deserialize(const char* data, unsigned int size)
{
    if (data == nullptr || size < 12) return false;

    unsigned char m = (unsigned char)data[0];
    unsigned char v = (unsigned char)data[1];
    if (m != 0x7E || v != 1) return false;

    NetworkPacket temp;
    temp.magicNumber = m;
    temp.version = v;

    int index = 2;
    memcpy(&temp.commandId, data + index, sizeof(temp.commandId));
    index += sizeof(temp.commandId);
    memcpy(&temp.statusCode, data + index, sizeof(temp.statusCode));
    index += sizeof(temp.statusCode);
    memcpy(&temp.payloadLength, data + index, sizeof(temp.payloadLength));
    index += sizeof(temp.payloadLength);

    if (size != (10 + temp.payloadLength + 2)) return false;

    if (temp.payloadLength > 0)
    {
        temp.payload = new char[temp.payloadLength];
        memcpy(temp.payload, data + index, temp.payloadLength);
        index += temp.payloadLength;
    }

    memcpy(&temp.checksum, data + index, sizeof(temp.checksum));

    if (!temp.isValid()) return false;

    *this = temp;
    return true;
}