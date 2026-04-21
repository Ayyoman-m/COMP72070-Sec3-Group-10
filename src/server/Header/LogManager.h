#pragma once
#include <string>

/**
 * @class LogManager
 * @brief Handles logging of system events.
 *
 * Writes system events and messages to a log file
 * for monitoring and debugging purposes.
 */
class LogManager {
public:

    /**
     * @brief Logs an event message.
     *
     * @param message Event description to log
     */
    void logEvent(const std::string& message);
};