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
    checksum = 0;
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
    checksum = 0;
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

char* Packet::getPayload() const
{
    return payload;
}

unsigned short Packet::getChecksum() const
{
    return checksum;
}
