#include "NetworkManager.h"
#include "NetworkPacket.h"
#include <cstring>

/**
 * @brief Sends all data over a TCP socket.
 *
 * Ensures that all bytes are transmitted, even if multiple send operations
 * are required due to TCP fragmentation.
 *
 * @param clientSocket Socket used for communication
 * @param data Buffer containing data to send
 * @param size Number of bytes to send
 * @return true if all data was sent successfully, false otherwise
 */

bool NetworkManager::sendAll(SOCKET clientSocket, const char* data, int size)
{
    int totalSent = 0;

    while (totalSent < size)
    {
        // Attempt to send the remaining slice of the buffer
        int bytesSent = send(clientSocket, data + totalSent, size - totalSent, 0);

        // A return of 0 or SOCKET_ERROR indicates a lost connection
        if (bytesSent <= 0)
        {
            return false;
        }

        totalSent += bytesSent;
    }

    return true;
}

/**
 * @brief Receives all data from a TCP socket.
 *
 * Blocks until the requested number of bytes are fully received,
 * ensuring complete data before processing.
 *
 * @param clientSocket Socket used for communication
 * @param buffer Buffer to store received data
 * @param size Number of bytes to receive
 * @return true if all data was received successfully, false otherwise
 */
bool NetworkManager::recvAll(SOCKET clientSocket, char* buffer, int size)
{
    int totalReceived = 0;

    while (totalReceived < size)
    {
        // Wait for the next chunk of the data stream
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
 * @brief Sends a NetworkPacket over the socket.
 *
 * Serializes the packet into binary format and transmits it using sendAll().
 *
 * @param clientSocket Socket used for communication
 * @param packet Packet to be sent
 * @return true if packet was sent successfully, false otherwise
 */
bool NetworkManager::sendPacket(SOCKET clientSocket, const NetworkPacket& packet)
{
    uint32_t packetSize = 0;

    // Convert the object into its binary wire format
    char* buffer = packet.serialize(packetSize);

    // Transmit the complete serialized blob
    bool success = sendAll(clientSocket, buffer, (int)packetSize);

    // Always free the temporary buffer allocated by serialize() to avoid leaks
    delete[] buffer;
    return success;
}

/**
 * @brief Receives and reconstructs a NetworkPacket from the socket.
 *
 * First reads the fixed-size header to determine payload size, then
 * receives the remaining data and reconstructs the packet.
 *
 * @param clientSocket Socket used for communication
 * @param packet Reference to store the received packet
 * @return true if packet was received and deserialized successfully, false otherwise
 */
bool NetworkManager::receivePacket(SOCKET clientSocket, NetworkPacket& packet)
{
    // Our protocol constants as defined in the binary specification
    const int HEADER_SIZE = 10;
    const int TAIL_SIZE = 2;

    char headerBuffer[HEADER_SIZE];

    // 1. Receive the fixed-size header first to inspect the payload length
    if (!recvAll(clientSocket, headerBuffer, HEADER_SIZE))
    {
        return false;
    }

    // 2. Extract payload length from the header (located at offset 6)
    uint32_t payloadLength = 0;
    memcpy(&payloadLength, headerBuffer + 6, sizeof(payloadLength));

    // 3. Calculate full expected size: Header (10) + Body (N) + Checksum (2)
    uint32_t remainingSize = payloadLength + TAIL_SIZE;
    uint32_t fullPacketSize = HEADER_SIZE + remainingSize;

    // --- SECURITY & ANALYSIS GUARD ---
    // Max size set to 2MB to accommodate the 1.045MB snapshot images.
    if (fullPacketSize < (HEADER_SIZE + TAIL_SIZE) || fullPacketSize > 2000000)
    {
        return false;
    }

    // 4. Allocate and initialize the buffer for the complete packet
    char* fullBuffer = new char[fullPacketSize];

    // Explicit initialization satisfies static analyzers and prevents memory leaks
    memset(fullBuffer, 0, fullPacketSize);

    // Copy the header we already have into the beginning of the buffer
    memcpy(fullBuffer, headerBuffer, HEADER_SIZE);

    // 5. Receive the remaining parts (Payload + Checksum) directly into the buffer
    if (!recvAll(clientSocket, fullBuffer + HEADER_SIZE, (int)remainingSize))
    {
        delete[] fullBuffer;
        return false;
    }

    // 6. Reconstruct the NetworkPacket object and verify the checksum internally
    bool success = packet.deserialize(fullBuffer, fullPacketSize);

    delete[] fullBuffer;
    return success;
}