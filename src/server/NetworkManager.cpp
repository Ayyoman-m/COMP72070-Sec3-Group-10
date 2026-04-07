#include "NetworkManager.h"
#include "NetworkPacket.h"
#include <cstring>

/**
 * Reliable transmission of raw data over a TCP socket.
 * TCP may fragment data; this loop ensures we don't return until every
 * single byte requested has been successfully sent.
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
 * Reliable reception of raw data from a TCP socket.
 * This blocks until the specific 'size' of data requested is fully received,
 * preventing us from processing incomplete headers or payloads.
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
 * High-level helper to transmit a NetworkPacket object.
 * This handles the internal serialization to bytes before calling sendAll.
 */
bool NetworkManager::sendPacket(SOCKET clientSocket, const NetworkPacket& packet)
{
    unsigned int packetSize = 0;

    // Convert the object into its binary wire format
    char* buffer = packet.serialize(packetSize);

    // Transmit the complete serialized blob
    bool success = sendAll(clientSocket, buffer, (int)packetSize);

    // Always free the temporary buffer allocated by serialize() to avoid leaks
    delete[] buffer;
    return success;
}

/**
 * High-level helper to reconstruct a NetworkPacket from the wire.
 * This reads the fixed 10-byte header first to determine the payload size.
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
    unsigned int payloadLength = 0;
    memcpy(&payloadLength, headerBuffer + 6, sizeof(payloadLength));

    // 3. Calculate full expected size: Header (10) + Body (N) + Checksum (2)
    unsigned int remainingSize = payloadLength + TAIL_SIZE;
    unsigned int fullPacketSize = HEADER_SIZE + remainingSize;

    // --- SECURITY & ANALYSIS GUARD (Resolves C6386) ---
    // Validate the size before allocation to prevent heap overflows or 
    // large-allocation denial of service. Max size set to 1MB.
    if (fullPacketSize < (HEADER_SIZE + TAIL_SIZE) || fullPacketSize > 1000000)
    {
        return false;
    }

    // 4. Allocate and initialize the buffer for the complete packet
    char* fullBuffer = new char[fullPacketSize];

    // Explicit initialization satisfies the static analyzer and prevents 
    // reading uninitialized memory if recvAll fails.
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