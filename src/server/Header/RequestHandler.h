#pragma once

#include "Packet.h"
#include "ClientSession.h"
#include "StateMachine.h"
#include "DeviceManager.h"
#include "LogManager.h"
#include <string>

class RequestHandler {
private:
    StateMachine& stateMachine;
    DeviceManager& deviceManager;
    LogManager& logManager;

public:
    RequestHandler(StateMachine& sm, DeviceManager& dm, LogManager& lm);

    // handles server-side string-based packet
    std::string handleRequest(const Packet& packet, ClientSession& session);

    // NEW: handles real network binary packet from networking branch
    std::string handleNetworkPacket(unsigned short commandId,
        const std::string& data,
        ClientSession& session);
};