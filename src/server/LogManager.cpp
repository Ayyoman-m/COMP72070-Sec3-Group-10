#include "Header/LogManager.h"
#include <fstream>

/**
 * @brief Writes a log message to a file.
 *
 * Appends the given message to "server_log.txt" for
 * monitoring and debugging purposes.
 *
 * @param message Log message to record
 */
void LogManager::logEvent(const std::string& message) {
    std::ofstream file("server_log.txt", std::ios::app);

    if (file.is_open()) {
        file << message << "\n";
        file.close();
    }
}