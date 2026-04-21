#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <thread> 
#include <string>
#include <vector>

#include "NetworkPacket.h"

// Qt Includes
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

/**
 * @class SmartHomeServer
 * @brief Main server application handling networking and UI.
 *
 * Provides a GUI-based server that listens for client connections,
 * processes network packets, and logs system activity.
 */
class SmartHomeServer : public QMainWindow {
    Q_OBJECT

public:

    /**
     * @brief Constructs the SmartHomeServer.
     * @param parent Parent widget
     */
    explicit SmartHomeServer(QWidget* parent = nullptr);

    /**
     * @brief Destructor for SmartHomeServer.
     *
     * Ensures networking resources are properly released.
     */
    ~SmartHomeServer();

private slots:

    /**
     * @brief Starts or stops the server.
     */
    void toggleServer();

private:

    /**
     * @brief Initializes UI components.
     */
    void setupUi();

    /**
     * @brief Logs message to UI console.
     */
    void logMessage(const QString& msg);

    // Networking Logic

    /**
     * @brief Listens for incoming client connections.
     */
    void startListening();

    /**
     * @brief Stops networking operations.
     */
    void stopNetworking();

    /**
     * @brief Processes incoming network packets.
     *
     * @param packet Received packet
     * @param clientSocket Client socket
     */
    void processPacket(const NetworkPacket& packet, SOCKET clientSocket);

    SOCKET serverSocket;          ///< Server socket
    bool isRunning;               ///< Server running state
    std::thread* listenerThread;  ///< Listener thread

    // UI Elements
    QLabel* statusLabel;
    QPushButton* startStopBtn;
    QTextEdit* logConsole;
};