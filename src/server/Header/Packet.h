#pragma once
#include <string>

/**
 * CommandID Enum: Used by the RequestHandler to identify
 * which action the server needs to perform.
 */
enum class CommandID {
    LOGIN,
    GET_STATUS,
    SET_MODE,
    TURN_ON_DEVICE,
    TURN_OFF_DEVICE,
    GET_DEVICE_STATUS,
    GET_ALL_DEVICE_STATUS,
    INVALID
};

/**
 * Logic Packet: A simple structure used for internal server processing.
 * This member 'data' must be a std::string so that your logic can
 * use .find(), .substr(), and comparison operators.
 */
struct Packet {
    CommandID command;
    std::string data;
};