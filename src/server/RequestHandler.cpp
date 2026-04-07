#include "Header/RequestHandler.h"
#include "Header/AuthManager.h"

// constructor
RequestHandler::RequestHandler(StateMachine& sm, DeviceManager& dm, LogManager& lm)
    : stateMachine(sm), deviceManager(dm), logManager(lm) {}

// handles incoming request
std::string RequestHandler::handleRequest(const Packet& packet, ClientSession& session) {

    // logging received command
    logManager.logEvent("RX request received");

    // checking login request
    if (packet.command == CommandID::LOGIN) {
        AuthManager auth;

        // expecting username,password
        size_t comma = packet.data.find(',');

        if (comma != std::string::npos) {
            std::string username = packet.data.substr(0, comma);
            std::string password = packet.data.substr(comma + 1);

            bool success = auth.login(username, password, session);

            if (success) {
                logManager.logEvent("LOGIN SUCCESS");
                return "LOGIN SUCCESS";
            }

            logManager.logEvent("LOGIN FAILED");
            return "LOGIN FAILED";
        }

        logManager.logEvent("LOGIN FORMAT ERROR");
        return "ERROR: INVALID LOGIN FORMAT";
    }

    // checking current server status
    if (packet.command == CommandID::GET_STATUS) {
        ServerState current = stateMachine.getState();

        if (current == ServerState::LOCKED) return "LOCKED";
        if (current == ServerState::HOME) return "HOME";
        if (current == ServerState::AWAY) return "AWAY";
        if (current == ServerState::MAINTENANCE) return "MAINTENANCE";
    }

    // all other commands need login first
    if (!session.isAuthenticated()) {
        logManager.logEvent("COMMAND REJECTED - NOT AUTHENTICATED");
        return "ERROR: NOT AUTHENTICATED";
    }

    // checking mode change request
    if (packet.command == CommandID::SET_MODE) {
        if (packet.data == "HOME") {
            bool result = stateMachine.setState(ServerState::HOME);
            logManager.logEvent(result ? "SET_MODE HOME SUCCESS" : "SET_MODE HOME FAILED");
            return result ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }

        if (packet.data == "AWAY") {
            bool result = stateMachine.setState(ServerState::AWAY);
            logManager.logEvent(result ? "SET_MODE AWAY SUCCESS" : "SET_MODE AWAY FAILED");
            return result ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }

        if (packet.data == "MAINTENANCE") {
            bool result = stateMachine.setState(ServerState::MAINTENANCE);
            logManager.logEvent(result ? "SET_MODE MAINTENANCE SUCCESS" : "SET_MODE MAINTENANCE FAILED");
            return result ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }

        if (packet.data == "LOCKED") {
            bool result = stateMachine.setState(ServerState::LOCKED);
            logManager.logEvent(result ? "SET_MODE LOCKED SUCCESS" : "SET_MODE LOCKED FAILED");
            return result ? "SUCCESS" : "ERROR: INVALID TRANSITION";
        }

        logManager.logEvent("SET_MODE UNKNOWN MODE");
        return "ERROR: UNKNOWN MODE";
    }

    // rejecting device commands in LOCKED or MAINTENANCE
    if (stateMachine.getState() == ServerState::LOCKED ||
        stateMachine.getState() == ServerState::MAINTENANCE) {

        if (packet.command == CommandID::TURN_ON_DEVICE ||
            packet.command == CommandID::TURN_OFF_DEVICE ||
            packet.command == CommandID::GET_DEVICE_STATUS ||
            packet.command == CommandID::GET_ALL_DEVICE_STATUS) {

            logManager.logEvent("DEVICE COMMAND REJECTED - INVALID SERVER STATE");
            return "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE";
        }
    }

    // turning on device
    if (packet.command == CommandID::TURN_ON_DEVICE) {
        bool result = deviceManager.turnOn(packet.data);
        logManager.logEvent(result ? "DEVICE TURN ON SUCCESS" : "DEVICE TURN ON FAILED");
        return result ? "SUCCESS" : "FAILURE";
    }

    // turning off device
    if (packet.command == CommandID::TURN_OFF_DEVICE) {
        bool result = deviceManager.turnOff(packet.data);
        logManager.logEvent(result ? "DEVICE TURN OFF SUCCESS" : "DEVICE TURN OFF FAILED");
        return result ? "SUCCESS" : "FAILURE";
    }

    // getting one device status
    if (packet.command == CommandID::GET_DEVICE_STATUS) {
        std::string status = deviceManager.getStatus(packet.data);
        logManager.logEvent("GET_DEVICE_STATUS");
        return status;
    }

    // getting all device status
    if (packet.command == CommandID::GET_ALL_DEVICE_STATUS) {
        std::string statusList = deviceManager.getAllStatus();
        logManager.logEvent("GET_ALL_DEVICE_STATUS");
        return statusList;
    }

    logManager.logEvent("INVALID COMMAND");
    return "ERROR: INVALID COMMAND";
}

// This bridges the networking binary Packet to the server string-based logic
std::string RequestHandler::handleNetworkPacket(unsigned short commandId,
    const std::string& data,
    ClientSession& session)
{
    // convert numeric commandId to server's CommandID enum
    Packet packet;
    packet.data = data;

    switch (commandId)
    {
    case 1:  packet.command = CommandID::LOGIN;                 break;
    case 2:  packet.command = CommandID::GET_STATUS;            break;
    case 3:  packet.command = CommandID::SET_MODE;              break;
    case 4:  packet.command = CommandID::TURN_ON_DEVICE;        break;
    case 5:  packet.command = CommandID::TURN_OFF_DEVICE;       break;
    case 6:  packet.command = CommandID::GET_DEVICE_STATUS;     break;
    case 7:  packet.command = CommandID::GET_ALL_DEVICE_STATUS; break;
    default: packet.command = CommandID::INVALID;               break;
    }

    return handleRequest(packet, session);
}