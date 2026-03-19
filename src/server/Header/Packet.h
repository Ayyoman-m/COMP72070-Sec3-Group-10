#pragma once
#include <string>

enum class CommandID {
    SET_MODE,
    GET_STATUS,
    LOGIN,
    INVALID
};

struct Packet {
    CommandID command;
    std::string data;
};