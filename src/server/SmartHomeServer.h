#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <thread> 
#include <string>
#include <vector>

// Include your packet logic
#include "NetworkPacket.h"

// Qt Includes
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

class SmartHomeServer : public QMainWindow {
    Q_OBJECT

public:
    explicit SmartHomeServer(QWidget* parent = nullptr);
    ~SmartHomeServer();

private slots:
    void toggleServer();

private:
    void setupUi();
    void logMessage(const QString& msg);

    // Networking Logic
    void startListening();
    void stopNetworking();

    // UPDATED: Must match the .cpp signature to handle responses/images
    void processPacket(const NetworkPacket& packet, SOCKET clientSocket);

    SOCKET serverSocket;
    bool isRunning;
    std::thread* listenerThread;

    // UI Elements
    QLabel* statusLabel;
    QPushButton* startStopBtn;
    QTextEdit* logConsole;
};