#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "SmartHomeServer.h"
#include "NetworkManager.h"
#include "NetworkPacket.h"
#include <QDateTime>
#include <WS2tcpip.h>
#include <QVBoxLayout>
#include <fstream> 

/**
 * @brief Constructs the SmartHomeServer UI and initializes state.
 *
 * Sets initial server state and prepares UI components.
 */
enum SystemState { LOCKED = 0, HOME = 1, AWAY = 2, MAINTENANCE = 3 };
SystemState currentState = HOME;

SmartHomeServer::SmartHomeServer(QWidget* parent)
    : QMainWindow(parent), isRunning(false), serverSocket(INVALID_SOCKET), listenerThread(nullptr)
{
    /**
 * @brief Sets up the server UI.
 *
 * Creates status display, start/stop controls, and log console.
 */
    setupUi();
    logMessage("Server Initialized. System State: [HOME]");
}

SmartHomeServer::~SmartHomeServer() {
    stopNetworking();
}

void SmartHomeServer::setupUi() {
    this->setWindowTitle("SmartHome Pro - Server Console");
    this->resize(600, 450);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    statusLabel = new QLabel("<b>Status:</b> <font color='red'>OFFLINE</font>", this);

    startStopBtn = new QPushButton("START SERVER", this);
    startStopBtn->setStyleSheet("background-color: #98C379; color: #12151A; font-weight: bold; padding: 8px;");

    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    logConsole->setStyleSheet("background-color: #1e1e1e; color: #abb2bf; font-family: 'Consolas';");

    layout->addWidget(statusLabel);
    layout->addWidget(startStopBtn);
    layout->addWidget(new QLabel("<b>Live Transaction Logs:</b>"));
    layout->addWidget(logConsole);

    connect(startStopBtn, &QPushButton::clicked, this, &SmartHomeServer::toggleServer);
    this->setCentralWidget(centralWidget);
}

/**
 * @brief Logs network transactions to file.
 *
 * Records command ID and data size for monitoring.
 *
 * Implements REQ-SVR-080 (logging requirement).
 */
void logTransaction(const QString& type, int cmd, size_t bytes) {
    std::ofstream logFile("server_log.txt", std::ios::app);
    if (logFile.is_open()) {
        QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        logFile << "[" << ts.toStdString() << "] " << type.toStdString()
            << " | CMD: " << cmd << " | LEN: " << bytes << " bytes" << std::endl;
    }
}

/**
 * @brief Starts or stops the server.
 *
 * Initializes socket, binds to port, and starts listener thread.
 */
void SmartHomeServer::toggleServer() {
    if (!isRunning) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(8080);

        if (::bind(serverSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            logMessage("ERROR: Bind failed. Is port 8080 busy?");
            return;
        }

        listen(serverSocket, SOMAXCONN);
        isRunning = true;

        startStopBtn->setText("STOP SERVER");
        startStopBtn->setStyleSheet("background-color: #E06C75; color: white; font-weight: bold;");
        statusLabel->setText("<b>Status:</b> <font color='green'>ONLINE (Port 8080)</font>");

        listenerThread = new std::thread(&SmartHomeServer::startListening, this);
        listenerThread->detach();
        logMessage("Network engine started.");
    }
    else {
        stopNetworking();
        isRunning = false;
        startStopBtn->setText("START SERVER");
        startStopBtn->setStyleSheet("background-color: #98C379; color: #12151A;");
        statusLabel->setText("<b>Status:</b> <font color='red'>OFFLINE</font>");
    }
}

/**
 * @brief Listens for incoming client connections.
 *
 * Accepts connections and continuously receives packets.
 */
void SmartHomeServer::startListening() {
    while (isRunning) {
        sockaddr_in cAddr;
        int cSize = sizeof(cAddr);
        SOCKET cSocket = ::accept(serverSocket, (SOCKADDR*)&cAddr, &cSize);

        if (cSocket != INVALID_SOCKET) {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cAddr.sin_addr, ipStr, INET_ADDRSTRLEN);

            QMetaObject::invokeMethod(this, [this, ipStr]() {
                logMessage(QString("Connection from: %1").arg(ipStr));
                });

            NetworkPacket packet;
            while (isRunning && NetworkManager::receivePacket(cSocket, packet)) {
                logTransaction("RECEIVE", packet.getCommandId(), packet.getPayloadLength());
                processPacket(packet, cSocket);
            }
            closesocket(cSocket);
        }
    }
}

/**
 * @brief Processes incoming network packets.
 *
 * Handles system mode updates and image transfer requests.
 *
 * Implements:
 * - REQ-SVR-040 (state machine update)
 * - REQ-SVR-070 (image transfer)
 */
void SmartHomeServer::processPacket(const NetworkPacket& packet, SOCKET clientSocket) {
    uint16_t cmd = packet.getCommandId();
    std::string payload(packet.getPayload(), packet.getPayloadLength());
    QString uiLog;

    switch (cmd) {
    case 4: // REQ-SVR-040: State Machine Update
    {
        int mode = std::stoi(payload);
        currentState = static_cast<SystemState>(mode);
        uiLog = QString("SYSTEM: Mode set to [%1]").arg(mode);

        // Send ACK (REQ-CLT-040)
        NetworkPacket response(4, "ACK_OK");
        NetworkManager::sendPacket(clientSocket, response);
        logTransaction("SEND_ACK", 4, 6);
        break;
    }
    case 5: // REQ-SVR-070: 1MB Image Transfer
    {
        uiLog = "IMAGE: Streaming 1MB snapshot...";

        std::ifstream file("C:\\Users\\DELL\\Documents\\Project IV\\build_root\\src\\server\\Debug\\snapshot.jpg", std::ios::binary | std::ios::ate);
        uint32_t size = 0;
        std::vector<char> imgData;

        if (file.is_open()) {
            size = (uint32_t)file.tellg();
            file.seekg(0, std::ios::beg);
            imgData.resize(size);
            file.read(imgData.data(), size);
        }
        else {
            size = 1024 * 1024; // Fallback 1MB
            imgData.assign(size, 0x00);
        }

        // Send 4-byte size header then the data
        send(clientSocket, (char*)&size, 4, 0);
        NetworkManager::sendAll(clientSocket, imgData.data(), (int)size);

        logTransaction("SEND_IMG", 5, size);
        uiLog += " Success.";
        break;
    }
    default:
        uiLog = QString("NET: Cmd %1 received.").arg(cmd);
        break;
    }

    QMetaObject::invokeMethod(this, [this, uiLog]() {
        logMessage(uiLog);
        });
}

/**
 * @brief Stops server networking.
 *
 * Closes sockets and cleans up Winsock resources.
 */
void SmartHomeServer::stopNetworking() {
    isRunning = false;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

/**
 * @brief Logs messages to UI console.
 *
 * Displays timestamped messages in the server interface.
 */
void SmartHomeServer::logMessage(const QString& msg) {
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    logConsole->append(QString("[%1] %2").arg(ts).arg(msg));
}