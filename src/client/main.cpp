/**
 * @file main.cpp
 * @brief Entry point for the Smart Home Client application.
 *
 * This file initializes the Windows networking library (Winsock)
 * and starts the Qt-based Smart Home Client interface.
 */
#include "SmartHomeClient.h"
#include <QtWidgets/QApplication>
#include <winsock2.h>


 /**
  * @brief Main function that launches the Smart Home Client.
  *
  * Initializes Winsock for network communication, starts the Qt application,
  * and displays the main client window.
  *
  * @param argc Argument count
  * @param argv Argument vector
  * @return int Exit status of the application
  */
int main(int argc, char* argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1; // Initialization failed
    }
    // Start Qt application
    QApplication a(argc, argv);
    // Create and display the Smart Home Client window
    SmartHomeClient clientWindow;
    clientWindow.show();

    return a.exec();

    // Execute application event loop
    int result = a.exec();

    // Clean up Winsock before exiting
    WSACleanup();
    return result;
}