#pragma once
#include <string>

// command types used in requests
enum class CommandID {
    LOGIN,
    SET_MODE,
    GET_STATUS,
    TURN_ON_DEVICE,
    TURN_OFF_DEVICE,
    GET_DEVICE_STATUS,
    INVALID
};

// simple packet used for testing
struct Packet {
    CommandID command;
    std::string data;
};