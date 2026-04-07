#pragma once
#include <string>

// command types - now with numbers to match networking branch
enum class CommandID : unsigned short {
    LOGIN = 1,
    GET_STATUS = 2,
    SET_MODE = 3,
    TURN_ON_DEVICE = 4,
    TURN_OFF_DEVICE = 5,
    GET_DEVICE_STATUS = 6,
    GET_ALL_DEVICE_STATUS = 7,
    INVALID = 0
};

// simple packet used for server logic
struct Packet {
    CommandID command;
    std::string data;
};