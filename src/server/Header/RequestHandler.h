#pragma once

#include "Packet.h"
#include "ClientSession.h"
#include "StateMachine.h"
#include <string>

class RequestHandler {
private:
    StateMachine& stateMachine;

public:
    RequestHandler(StateMachine& sm);

    std::string handleRequest(const Packet& packet, ClientSession& session);
};