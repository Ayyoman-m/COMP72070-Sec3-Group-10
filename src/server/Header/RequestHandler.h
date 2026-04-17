#pragma once

#include "Packet.h"
#include "ClientSession.h"
#include "StateMachine.h"
#include "DeviceManager.h"
#include "LogManager.h"
#include <string>

/**
 * @class RequestHandler
 * @brief Processes incoming client requests on the server.
 *
 * Coordinates authentication, state management, device control,
 * and logging by interacting with system components.
 */
class RequestHandler {
private:
    StateMachine& stateMachine;   ///< Manages system state transitions
    DeviceManager& deviceManager; ///< Controls device operations
    LogManager& logManager;       ///< Handles logging of events

public:

    /**
     * @brief Constructs the RequestHandler.
     *
     * @param sm Reference to StateMachine
     * @param dm Reference to DeviceManager
     * @param lm Reference to LogManager
     */
    RequestHandler(StateMachine& sm, DeviceManager& dm, LogManager& lm);

    /**
     * @brief Handles a logical packet request.
     *
     * Processes commands based on Packet structure and updates session.
     *
     * @param packet Incoming request packet
     * @param session Client session
     * @return Response string
     */
    std::string handleRequest(const Packet& packet, ClientSession& session);

    /**
     * @brief Handles a network-level packet.
     *
     * Processes binary packet data received over the network.
     *
     * @param commandId Command identifier
     * @param data Payload data
     * @param session Client session
     * @return Response string
     */
    std::string handleNetworkPacket(unsigned short commandId,
        const std::string& data,
        ClientSession& session);
};