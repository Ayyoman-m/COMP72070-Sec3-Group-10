#include "Header/RequestHandler.h"

RequestHandler::RequestHandler(StateMachine& sm) : stateMachine(sm) {}

std::string RequestHandler::handleRequest(const Packet& packet, ClientSession& session) {
    if (packet.command == CommandID::GET_STATUS) {
        ServerState current = stateMachine.getState();

        if (current == ServerState::LOCKED) return "LOCKED";
        if (current == ServerState::HOME) return "HOME";
        if (current == ServerState::AWAY) return "AWAY";
        if (current == ServerState::MAINTENANCE) return "MAINTENANCE";
    }

    if (packet.command == CommandID::SET_MODE) {
        if (!session.isAuthenticated()) {
            return "ERROR: NOT AUTHENTICATED";
        }

        if (packet.data == "HOME") {
            return stateMachine.setState(ServerState::HOME) ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }
        if (packet.data == "AWAY") {
            return stateMachine.setState(ServerState::AWAY) ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }
        if (packet.data == "MAINTENANCE") {
            return stateMachine.setState(ServerState::MAINTENANCE) ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }
        if (packet.data == "LOCKED") {
            return stateMachine.setState(ServerState::LOCKED) ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }

        return "ERROR: UNKNOWN MODE";
    }

    return "ERROR: INVALID COMMAND";
}