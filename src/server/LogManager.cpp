#include "Header/LogManager.h"
#include <fstream>

// writing log message into file
void LogManager::logEvent(const std::string& message) {
    std::ofstream file("server_log.txt", std::ios::app);

    if (file.is_open()) {
        file << message << "\n";
        file.close();
    }
}