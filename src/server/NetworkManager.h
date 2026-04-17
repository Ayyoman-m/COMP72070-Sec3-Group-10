#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include "NetworkPacket.h"

/**
 * @class NetworkManager
 * @brief Handles socket communication and packet transmission.
 *
 * Provides helper functions for reliable sending and receiving
 * of raw data and structured network packets.
 */
class NetworkManager {
public:

    /**
     * @brief Sends all bytes over a socket.
     *
     * @param sock Socket used for communication
     * @param data Data buffer to send
     * @param size Number of bytes to send
     * @return true if successful, false otherwise
     */
    static bool sendAll(SOCKET sock, const char* data, int size);

    /**
     * @brief Receives all bytes from a socket.
     *
     * @param sock Socket used for communication
     * @param buffer Buffer to store received data
     * @param size Number of bytes to receive
     * @return true if successful, false otherwise
     */
    static bool recvAll(SOCKET sock, char* buffer, int size);

    /**
     * @brief Sends a network packet.
     *
     * @param sock Socket used for communication
     * @param packet Packet to send
     * @return true if successful, false otherwise
     */
    static bool sendPacket(SOCKET sock, const NetworkPacket& packet);

    /**
     * @brief Receives a network packet.
     *
     * @param sock Socket used for communication
     * @param packet Packet to populate
     * @return true if successful, false otherwise
     */
    static bool receivePacket(SOCKET sock, NetworkPacket& packet);
};