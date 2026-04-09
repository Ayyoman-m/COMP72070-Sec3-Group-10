#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "SmartHomeServer.h"
#include "NetworkManager.h"
#include "NetworkPacket.h"

#include <QDateTime>
#include <WS2tcpip.h>
#include <QVBoxLayout>

SmartHomeServer::SmartHomeServer(QWidget* parent)
    : QMainWindow(parent), isRunning(false), serverSocket(INVALID_SOCKET), listenerThread(nullptr)
{
    setupUi();
    logMessage("Server Initialized. Ready to bind sockets.");
}

SmartHomeServer::~SmartHomeServer() {
    stopNetworking();
}

void SmartHomeServer::setupUi() {
    this->setWindowTitle("Smart Home Server Admin Console");
    this->resize(600, 400);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    statusLabel = new QLabel("<b>Status:</b> OFFLINE", this);
    statusLabel->setStyleSheet("color: red; font-size: 14px;");

    startStopBtn = new QPushButton("Start Server", this);
    startStopBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 5px;");

    connect(startStopBtn, &QPushButton::clicked, this, &SmartHomeServer::toggleServer);

    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    logConsole->setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: monospace;");

    layout->addWidget(statusLabel);
    layout->addWidget(startStopBtn);
    layout->addWidget(new QLabel("System Logs:", this));
    layout->addWidget(logConsole);

    this->setCentralWidget(centralWidget);
}

void SmartHomeServer::toggleServer() {
    if (!isRunning) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            logMessage("CRITICAL: Winsock initialization failed.");
            return;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            logMessage("CRITICAL: Socket creation failed.");
            WSACleanup();
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8080);

        if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            logMessage("CRITICAL: Bind failed. Port 8080 might be in use.");
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            logMessage("CRITICAL: Listen failed.");
            return;
        }

        isRunning = true;
        startStopBtn->setText("Stop Server");
        startStopBtn->setStyleSheet("background-color: #f44336; color: white; font-weight: bold; padding: 5px;");
        statusLabel->setText("<b>Status:</b> ONLINE (Listening on Port 8080)");
        statusLabel->setStyleSheet("color: green; font-size: 14px;");

        listenerThread = new std::thread(&SmartHomeServer::startListening, this);
        listenerThread->detach();

        logMessage("Server STARTED. Port 8080 is now open.");
    }
    else {
        stopNetworking();
        isRunning = false;
        startStopBtn->setText("Start Server");
        startStopBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 5px;");
        statusLabel->setText("<b>Status:</b> OFFLINE");
        statusLabel->setStyleSheet("color: red; font-size: 14px;");
        logMessage("Server STOPPED.");
    }
}

void SmartHomeServer::startListening() {
    while (isRunning) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = ::accept(serverSocket, (SOCKADDR*)&clientAddr, &clientSize);

        if (clientSocket != INVALID_SOCKET) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);

            QMetaObject::invokeMethod(this, [this, ip]() {
                logMessage(QString("--- New Session: %1 ---").arg(ip));
                });

            NetworkPacket packet;
            while (isRunning && NetworkManager::receivePacket(clientSocket, packet)) {
                processPacket(packet);
            }

            closesocket(clientSocket);
            QMetaObject::invokeMethod(this, [this, ip]() {
                logMessage(QString("--- Session Ended: %1 ---").arg(ip));
                });
        }
    }
}

void SmartHomeServer::processPacket(const NetworkPacket& packet) {
    // 1. Get the Command ID (Note the lowercase 'd')
    uint16_t cmd = packet.getCommandId();

    // 2. Extract the payload string using 'getPayloadLength' instead of 'getPayloadSize'
    std::string payloadStr(packet.getPayload(), packet.getPayloadLength());

    QString logEntry;
    switch (cmd) {
    case 4: // Toggle Device
        logEntry = QString("ACTION: Toggle Request -> [%1]").arg(QString::fromStdString(payloadStr));
        break;
    default:
        logEntry = QString("DATA: Cmd %1 | Payload: %2").arg(cmd).arg(QString::fromStdString(payloadStr));
        break;
    }

    // Safely log to UI
    QMetaObject::invokeMethod(this, [this, logEntry]() {
        logMessage(logEntry);
        });
}


void SmartHomeServer::stopNetworking() {
    isRunning = false;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

void SmartHomeServer::logMessage(const QString& msg) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    logConsole->append(QString("[%1] %2").arg(timestamp).arg(msg));
}