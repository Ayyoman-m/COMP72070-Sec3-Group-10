#include "NetworkManager.h"
#include "NetworkPacket.h"
#include <cstring>

/**
 * @brief Sends all bytes over a TCP socket.
 *
 * Ensures complete transmission even if data is fragmented.
 *
 * @param clientSocket Socket used for communication
 * @param data Data buffer to send
 * @param size Number of bytes to send
 * @return true if successful, false otherwise
 */
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

/**
 * @brief Receives all bytes from a TCP socket.
 *
 * Blocks until the requested amount of data is fully received.
 *
 * @param clientSocket Socket used for communication
 * @param buffer Buffer to store received data
 * @param size Number of bytes to receive
 * @return true if successful, false otherwise
 */
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

/**
 * @brief Sends a serialized network packet.
 *
 * Converts the packet into binary format and transmits it.
 *
 * @param clientSocket Socket used for communication
 * @param packet Packet to send
 * @return true if successful, false otherwise
 */
bool NetworkManager::sendPacket(SOCKET clientSocket, const NetworkPacket& packet)
{
    uint32_t packetSize = 0;

    char* buffer = packet.serialize(packetSize);

    bool success = sendAll(clientSocket, buffer, (int)packetSize);

    delete[] buffer;
    return success;
}

/**
 * @brief Receives and reconstructs a network packet.
 *
 * Reads header, determines payload size, and reconstructs the packet.
 *
 * @param clientSocket Socket used for communication
 * @param packet Packet object to populate
 * @return true if successful, false otherwise
 */
bool NetworkManager::receivePacket(SOCKET clientSocket, NetworkPacket& packet)
{
    const int HEADER_SIZE = 10;
    const int TAIL_SIZE = 2;

    char headerBuffer[HEADER_SIZE];

    if (!recvAll(clientSocket, headerBuffer, HEADER_SIZE))
    {
        return false;
    }

    uint32_t payloadLength = 0;
    memcpy(&payloadLength, headerBuffer + 6, sizeof(payloadLength));

    uint32_t remainingSize = payloadLength + TAIL_SIZE;
    uint32_t fullPacketSize = HEADER_SIZE + remainingSize;

    if (fullPacketSize < (HEADER_SIZE + TAIL_SIZE) || fullPacketSize > 2000000)
    {
        return false;
    }

    char* fullBuffer = new char[fullPacketSize];
    memset(fullBuffer, 0, fullPacketSize);

    memcpy(fullBuffer, headerBuffer, HEADER_SIZE);

    if (!recvAll(clientSocket, fullBuffer + HEADER_SIZE, (int)remainingSize))
    {
        delete[] fullBuffer;
        return false;
    }

    bool success = packet.deserialize(fullBuffer, fullPacketSize);

    delete[] fullBuffer;
    return success;
}