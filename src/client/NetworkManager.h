#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include "NetworkPacket.h"


/**
 * @class NetworkManager
 * @brief Provides helper functions for sending and receiving data over sockets.
 *
 * This class handles low-level socket communication as well as high-level
 * packet transmission for the Smart Home system. It ensures reliable
 * transmission of raw data and structured packets between client and server.
 */
class NetworkManager {
public:

    /**
     * @brief Sends all bytes of data over a socket.
     *
     * Ensures that the entire buffer is transmitted, even if multiple send
     * operations are required.
     *
     * @param sock Socket used for communication
     * @param data Pointer to data buffer to send
     * @param size Number of bytes to send
     * @return true if all data was sent successfully, false otherwise
     */
    static bool sendAll(SOCKET sock, const char* data, int size);

    /**
     * @brief Receives all bytes of data from a socket.
     *
     * Ensures that the requested number of bytes are received, even if
     * multiple receive operations are required.
     *
     * @param sock Socket used for communication
     * @param buffer Buffer to store received data
     * @param size Number of bytes to receive
     * @return true if all data was received successfully, false otherwise
     */
    static bool recvAll(SOCKET sock, char* buffer, int size);

    /**
     * @brief Sends a structured network packet over a socket.
     *
     * Serializes the NetworkPacket object and transmits it using sendAll().
     *
     * @param sock Socket used for communication
     * @param packet Packet to be sent
     * @return true if packet was sent successfully, false otherwise
     */

    static bool sendPacket(SOCKET sock, const NetworkPacket& packet);

    /**
     * @brief Receives a structured network packet from a socket.
     *
     * Reads incoming data and reconstructs it into a NetworkPacket object.
     *
     * @param sock Socket used for communication
     * @param packet Reference to store the received packet
     * @return true if packet was received successfully, false otherwise
     */
    static bool receivePacket(SOCKET sock, NetworkPacket& packet);
};