#include "Header/RequestHandler.h"
#include "Header/AuthManager.h"

RequestHandler::RequestHandler(StateMachine& sm) : stateMachine(sm) {}

std::string RequestHandler::handleRequest(const Packet& packet, ClientSession& session) {

    // checking login request
    if (packet.command == CommandID::LOGIN) {
        AuthManager auth;

        // expecting data in format: username,password
        size_t comma = packet.data.find(',');

        if (comma != std::string::npos) {
            std::string username = packet.data.substr(0, comma);
            std::string password = packet.data.substr(comma + 1);

            bool success = auth.login(username, password, session);

            // returning result based on login
            return success ? "LOGIN SUCCESS" : "LOGIN FAILED";
        }

        // if format is wrong
        return "ERROR: INVALID LOGIN FORMAT";
    }

    // checking status request
    if (packet.command == CommandID::GET_STATUS) {
        ServerState current = stateMachine.getState();

        if (current == ServerState::LOCKED) return "LOCKED";
        if (current == ServerState::HOME) return "HOME";
        if (current == ServerState::AWAY) return "AWAY";
        if (current == ServerState::MAINTENANCE) return "MAINTENANCE";
    }

    // checking mode change request
    if (packet.command == CommandID::SET_MODE) {

        // making sure user is logged in
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

    // default case if command not matched
    return "ERROR: INVALID COMMAND";
}