#include "Header/RequestHandler.h"
#include "Header/AuthManager.h"

// constructor
RequestHandler::RequestHandler(StateMachine& sm, DeviceManager& dm)
    : stateMachine(sm), deviceManager(dm) {}

// handles incoming request
std::string RequestHandler::handleRequest(const Packet& packet, ClientSession& session) {

    // checking login request
    if (packet.command == CommandID::LOGIN) {
        AuthManager auth;

        // expecting username,password
        size_t comma = packet.data.find(',');

        if (comma != std::string::npos) {
            std::string username = packet.data.substr(0, comma);
            std::string password = packet.data.substr(comma + 1);

            bool success = auth.login(username, password, session);

            return success ? "LOGIN SUCCESS" : "LOGIN FAILED";
        }

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

        // user must be logged in first
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

    // checking device ON request
    if (packet.command == CommandID::TURN_ON_DEVICE) {
        if (!session.isAuthenticated()) {
            return "ERROR: NOT AUTHENTICATED";
        }

        return deviceManager.turnOn(packet.data) ? "DEVICE TURNED ON" : "ERROR: DEVICE NOT FOUND";
    }

    // checking device OFF request
    if (packet.command == CommandID::TURN_OFF_DEVICE) {
        if (!session.isAuthenticated()) {
            return "ERROR: NOT AUTHENTICATED";
        }

        return deviceManager.turnOff(packet.data) ? "DEVICE TURNED OFF" : "ERROR: DEVICE NOT FOUND";
    }

    // checking device status request
    if (packet.command == CommandID::GET_DEVICE_STATUS) {
        if (!session.isAuthenticated()) {
            return "ERROR: NOT AUTHENTICATED";
        }

        return deviceManager.getStatus(packet.data);
    }

    return "ERROR: INVALID COMMAND";
}