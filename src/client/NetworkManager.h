#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include "NetworkPacket.h"

class NetworkManager {
public:
    // Reliable Raw Data Helpers
    static bool sendAll(SOCKET sock, const char* data, int size);
    static bool recvAll(SOCKET sock, char* buffer, int size);

    // High-Level Packet Helpers
    static bool sendPacket(SOCKET sock, const NetworkPacket& packet);
    static bool receivePacket(SOCKET sock, NetworkPacket& packet);
};