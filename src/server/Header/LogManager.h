#pragma once
#include <string>

// this class writes logs to file
class LogManager {
public:
    // writes one log line
    void logEvent(const std::string& message);
};