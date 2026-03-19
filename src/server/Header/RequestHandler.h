#pragma once

#include "Packet.h"
#pragma once

#include "Packet.h"
#include "ClientSession.h"
#include "StateMachine.h"
#include "DeviceManager.h"
#include <string>

// this class handles incoming requests
class RequestHandler {
private:
    StateMachine& stateMachine;
    DeviceManager& deviceManager;

public:
    // constructor
    RequestHandler(StateMachine& sm, DeviceManager& dm);

    // handles request and returns response
    std::string handleRequest(const Packet& packet, ClientSession& session);
};