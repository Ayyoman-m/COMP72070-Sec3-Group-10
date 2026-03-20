#include "Packet.h"
#include <cstring>

Packet::Packet()
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

Packet::Packet(unsigned short cmd, unsigned short status)
{
    // HEADER
    magicNumber = 0x7E;
    version = 1;
    commandId = cmd;
    statusCode = status;
    payloadLength = 0;

    // BODY
    payload = nullptr;

    // TAIL
    checksum = calculateChecksum();
}

Packet::Packet(const Packet& other)
{
    // HEADER
    magicNumber = other.magicNumber;
    version = other.version;
    commandId = other.commandId;
    statusCode = other.statusCode;
    payloadLength = other.payloadLength;

    // TAIL
    checksum = other.checksum;

    // BODY
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

Packet& Packet::operator=(const Packet& other)
{
    if (this != &other)
    {
        if (payload != nullptr)
        {
            delete[] payload;
            payload = nullptr;
        }

        // HEADER
        magicNumber = other.magicNumber;
        version = other.version;
        commandId = other.commandId;
        statusCode = other.statusCode;
        payloadLength = other.payloadLength;

        // TAIL
        checksum = other.checksum;

        // BODY
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

Packet::~Packet()
{
    if (payload != nullptr)
    {
        delete[] payload;
        payload = nullptr;
    }
}

void Packet::setCommandId(unsigned short cmd)
{
    commandId = cmd;
    checksum = calculateChecksum();
}

void Packet::setStatusCode(unsigned short status)
{
    statusCode = status;
    checksum = calculateChecksum();
}

void Packet::setPayload(const char* data, unsigned int length)
{
    if (payload != nullptr)
    {
        delete[] payload;
        payload = nullptr;
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
unsigned char Packet::getMagicNumber() const
{
    return magicNumber;
}

unsigned char Packet::getVersion() const
{
    return version;
}

unsigned short Packet::getCommandId() const
{
    return commandId;
}

unsigned short Packet::getStatusCode() const
{
    return statusCode;
}

unsigned int Packet::getPayloadLength() const
{
    return payloadLength;
}

const char* Packet::getPayload() const
{
    return payload;
}

unsigned short Packet::getChecksum() const
{
    return checksum;
}

unsigned short Packet::calculateChecksum() const
{
    unsigned int sum = 0;

    // HEADER
    sum += magicNumber;
    sum += version;
    sum += commandId;
    sum += statusCode;
    sum += payloadLength;

    // BODY
    for (unsigned int i = 0; i < payloadLength; i++)
    {
        sum += (unsigned char)payload[i];
    }

    // TAIL is not included in its own checksum
    return (unsigned short)(sum % 65536); //limit to 16-bit range ,65536 = 2^16
}

bool Packet::isValid() const
{
    if (magicNumber != 0x7E)
    {
        return false;
    }

    if (version != 1)
    {
        return false;
    }

    if (checksum != calculateChecksum())
    {
        return false;
    }

    return true;
}

char* Packet::serialize(unsigned int& outSize) const
{
    unsigned short currentChecksum = calculateChecksum();

    // HEADER = 10 bytes
    // BODY = payloadLength bytes
    // TAIL = 2 bytes
    outSize = 10 + payloadLength + 2;

    char* buffer = new char[outSize];

    int index = 0;

    // HEADER

    buffer[index] = magicNumber;
    index += 1;

    buffer[index] = version;
    index += 1;

    memcpy(buffer + index, &commandId, sizeof(commandId));
    index += sizeof(commandId);

    memcpy(buffer + index, &statusCode, sizeof(statusCode));
    index += sizeof(statusCode);

    memcpy(buffer + index, &payloadLength, sizeof(payloadLength));
    index += sizeof(payloadLength);

     
    // BODY

    if (payload != nullptr && payloadLength > 0)
    {
        memcpy(buffer + index, payload, payloadLength);
        index += payloadLength;
    }

    // TAIL

    memcpy(buffer + index, &currentChecksum, sizeof(currentChecksum));

    return buffer;
}

bool Packet::deserialize(const char* data, unsigned int size)
{
    // Minimum packet size = HEADER(10) + TAIL(2)
    if (data == nullptr || size < 12)
    {
        return false;
    }

    if (payload != nullptr)
    {
        delete[] payload;
        payload = nullptr;
    }

    int index = 0;

    // HEADER

    magicNumber = (unsigned char)data[index];
    index += 1;

    version = (unsigned char)data[index];
    index += 1;

    memcpy(&commandId, data + index, sizeof(commandId));
    index += sizeof(commandId);

    memcpy(&statusCode, data + index, sizeof(statusCode));
    index += sizeof(statusCode);

    memcpy(&payloadLength, data + index, sizeof(payloadLength));
    index += sizeof(payloadLength);

    // Validate full size
    if (size != (10 + payloadLength + 2))
    {
        payloadLength = 0;
        return false;
    }

    // BODY

    if (payloadLength > 0)
    {
        payload = new char[payloadLength];
        memcpy(payload, data + index, payloadLength);
        index += payloadLength;
    }
    else
    {
        payload = nullptr;
    }

    // TAIL

    memcpy(&checksum, data + index, sizeof(checksum));

    return isValid();
}