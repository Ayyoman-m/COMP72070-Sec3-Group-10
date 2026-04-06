#include "SmartHomeServer.h"
#include <QDateTime>

SmartHomeServer::SmartHomeServer(QWidget* parent) : QMainWindow(parent), isRunning(false) {
    setupUi();
    logMessage("Server Initialized. Ready to bind sockets.");
}

void SmartHomeServer::setupUi() {
    this->setWindowTitle("Smart Home Server Admin Console");
    this->resize(600, 400);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // Header / Controls
    statusLabel = new QLabel("<b>Status:</b> OFFLINE", this);
    statusLabel->setStyleSheet("color: red; font-size: 14px;");

    startStopBtn = new QPushButton("Start Server", this);
    startStopBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 5px;");

    connect(startStopBtn, &QPushButton::clicked, this, &SmartHomeServer::toggleServer);

    // The Log Viewer
    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true); // Don't let users type in the log
    logConsole->setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: monospace;");

    layout->addWidget(statusLabel);
    layout->addWidget(startStopBtn);
    layout->addWidget(new QLabel("System Logs:", this));
    layout->addWidget(logConsole);

    this->setCentralWidget(centralWidget);
}

void SmartHomeServer::toggleServer() {
    isRunning = !isRunning;

    if (isRunning) {
        startStopBtn->setText("Stop Server");
        startStopBtn->setStyleSheet("background-color: #f44336; color: white; font-weight: bold; padding: 5px;");
        statusLabel->setText("<b>Status:</b> ONLINE (Listening on Port...)");
        statusLabel->setStyleSheet("color: green; font-size: 14px;");

        // TODO: Call your POSIX socket bind/listen functions here
        logMessage("Server STARTED. Listening for incoming client connections.");
    }
    else {
        startStopBtn->setText("Start Server");
        startStopBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 5px;");
        statusLabel->setText("<b>Status:</b> OFFLINE");
        statusLabel->setStyleSheet("color: red; font-size: 14px;");

        // TODO: Call your POSIX socket close functions here
        logMessage("Server STOPPED. Dropped all client connections.");
    }
}

void SmartHomeServer::logMessage(const QString& msg) {
    // Adds a nice timestamp to every log entry
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    logConsole->append(QString("[%1] %2").arg(timestamp).arg(msg));
}