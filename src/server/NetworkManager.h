#pragma once
#include "Packet.h"
#include <winsock2.h>

class NetworkManager
{
public:
    static bool sendAll(SOCKET clientSocket, const char* data, int size);
    static bool recvAll(SOCKET clientSocket, char* buffer, int size);

    static bool sendPacket(SOCKET clientSocket, const Packet& packet);
    static bool receivePacket(SOCKET clientSocket, Packet& packet);
};