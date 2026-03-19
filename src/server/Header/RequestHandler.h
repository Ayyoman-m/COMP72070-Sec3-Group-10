#pragma once

#include "Packet.h"
#include "ClientSession.h"
#include "StateMachine.h"
#include <string>

class RequestHandler {
private:
    StateMachine& stateMachine;

public:
    // constructor
    RequestHandler(StateMachine& sm);

    // handles incoming request
    std::string handleRequest(const Packet& packet, ClientSession& session);
};