#pragma once
#include "NetworkPacket.h" // Ensure this points to the renamed header
#include <winsock2.h>

class NetworkManager
{
public:
    static bool sendAll(SOCKET clientSocket, const char* data, int size);
    static bool recvAll(SOCKET clientSocket, char* buffer, int size);

    // These MUST use 'NetworkPacket' to match your .cpp implementation
    static bool sendPacket(SOCKET clientSocket, const NetworkPacket& packet);
    static bool receivePacket(SOCKET clientSocket, NetworkPacket& packet);
};