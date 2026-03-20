#include "NetworkManager.h"
#include <cstring>

bool NetworkManager::sendAll(SOCKET clientSocket, const char* data, int size)
{
    int totalSent = 0;

    while (totalSent < size)
    {
        int bytesSent = send(clientSocket, data + totalSent, size - totalSent, 0);

        if (bytesSent <= 0)
        {
            return false;
        }

        totalSent += bytesSent;
    }

    return true;
}

bool NetworkManager::recvAll(SOCKET clientSocket, char* buffer, int size)
{
    int totalReceived = 0;

    while (totalReceived < size)
    {
        int bytesReceived = recv(clientSocket, buffer + totalReceived, size - totalReceived, 0);

        if (bytesReceived <= 0)
        {
            return false;
        }

        totalReceived += bytesReceived;
    }

    return true;
}

bool NetworkManager::sendPacket(SOCKET clientSocket, const Packet& packet)
{
    unsigned int packetSize = 0;
    char* buffer = packet.serialize(packetSize);

    bool success = sendAll(clientSocket, buffer, packetSize);

    delete[] buffer;
    return success;
}

bool NetworkManager::receivePacket(SOCKET clientSocket, Packet& packet)
{
    const int HEADER_SIZE = 10;
    const int TAIL_SIZE = 2;

    char headerBuffer[HEADER_SIZE];

    // Step 1: receive fixed header
    if (!recvAll(clientSocket, headerBuffer, HEADER_SIZE))
    {
        return false;
    }

    // Step 2: extract payload length from header
    unsigned int payloadLength = 0;
    memcpy(&payloadLength, headerBuffer + 6, sizeof(payloadLength));

    // Step 3: compute remaining bytes = body + tail
    unsigned int remainingSize = payloadLength + TAIL_SIZE;

    // Step 4: make full packet buffer
    unsigned int fullPacketSize = HEADER_SIZE + remainingSize;
    char* fullBuffer = new char[fullPacketSize];

    // copy header into full buffer
    memcpy(fullBuffer, headerBuffer, HEADER_SIZE);

    // Step 5: receive body + tail
    if (!recvAll(clientSocket, fullBuffer + HEADER_SIZE, remainingSize))
    {
        delete[] fullBuffer;
        return false;
    }

    // Step 6: deserialize into packet object
    bool success = packet.deserialize(fullBuffer, fullPacketSize);

    delete[] fullBuffer;
    return success;
}