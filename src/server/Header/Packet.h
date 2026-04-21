#pragma once
#include <string>

/**
 * @enum CommandID
 * @brief Identifies commands sent between client and server.
 *
 * Used by the RequestHandler to determine which operation
 * should be performed on the server.
 */
enum class CommandID {
    LOGIN,                  ///< Authenticate user
    GET_STATUS,             ///< Retrieve system status
    SET_MODE,               ///< Change system mode
    TURN_ON_DEVICE,         ///< Turn a device ON
    TURN_OFF_DEVICE,        ///< Turn a device OFF
    GET_DEVICE_STATUS,      ///< Get status of a specific device
    GET_ALL_DEVICE_STATUS,  ///< Get status of all devices
    INVALID                 ///< Invalid or unknown command
};

/**
 * @struct Packet
 * @brief Represents a logical request packet.
 *
 * Used internally by the server to process commands and associated data.
 */
struct Packet {
    CommandID command;  ///< Command type
    std::string data;   ///< Payload data associated with the command
};