#include "SmartHomeClient.h"

SmartHomeClient::SmartHomeClient(QWidget* parent) : QMainWindow(parent), isCommandPending(false) {
    // Initialize the 5-second rule timer
    commandTimer = new QTimer(this);
    commandTimer->setSingleShot(true);
    connect(commandTimer, &QTimer::timeout, this, &SmartHomeClient::onCommandTimeout);

    setupUi();
}

void SmartHomeClient::setupUi() {
    this->setWindowTitle("Smart Home Control Panel");
    this->resize(800, 600);

    centralStack = new QStackedWidget(this);
    this->setCentralWidget(centralStack);

    setupLoginScreen();
    setupDashboard();

    // Start on the login screen (Index 0)
    centralStack->setCurrentWidget(loginWidget);
}

void SmartHomeClient::setupLoginScreen() {
    loginWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(loginWidget);

    QLabel* title = new QLabel("<h2>Smart Home Login</h2>", loginWidget);
    title->setAlignment(Qt::AlignCenter);

    userEdit = new QLineEdit(loginWidget);
    userEdit->setPlaceholderText("Username");

    passEdit = new QLineEdit(loginWidget);
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);

    QPushButton* loginBtn = new QPushButton("Login", loginWidget);
    loginStatusLabel = new QLabel("", loginWidget);
    loginStatusLabel->setStyleSheet("color: red;");

    connect(loginBtn, &QPushButton::clicked, this, &SmartHomeClient::attemptLogin);

    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(userEdit);
    layout->addWidget(passEdit);
    layout->addWidget(loginBtn);
    layout->addWidget(loginStatusLabel);
    layout->addStretch();

    centralStack->addWidget(loginWidget);
}

void SmartHomeClient::setupDashboard() {
    dashboardWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(dashboardWidget);

    // --- Mode Display ---
    QGroupBox* statusGroup = new QGroupBox("System Status", dashboardWidget);
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    modeLabel = new QLabel("Current Mode: <b>HOME</b>", statusGroup);
    statusLayout->addWidget(modeLabel);

    // --- Interactive Floorplan ---
    floorplanScene = new QGraphicsScene(this);
    floorplanView = new QGraphicsView(floorplanScene, dashboardWidget);
    floorplanView->setMinimumHeight(300);

    // Add a mockup appliance (a red square representing a light or heater)
    testAppliance = floorplanScene->addRect(0, 0, 50, 50, QPen(Qt::black), QBrush(Qt::red));
    testAppliance->setFlag(QGraphicsItem::ItemIsSelectable);

    QLabel* floorplanInstructions = new QLabel("Floorplan View: Select an appliance above.", dashboardWidget);

    // --- Controls & Feedback ---
    sendCommandBtn = new QPushButton("Send Test Command", dashboardWidget);
    feedbackLabel = new QLabel("System Ready.", dashboardWidget);
    feedbackLabel->setStyleSheet("color: blue;");

    connect(sendCommandBtn, &QPushButton::clicked, this, &SmartHomeClient::sendCommand);

    layout->addWidget(statusGroup);
    layout->addWidget(floorplanView);
    layout->addWidget(floorplanInstructions);
    layout->addWidget(sendCommandBtn);
    layout->addWidget(feedbackLabel);

    centralStack->addWidget(dashboardWidget);
}

// --- Logic Implementation ---

void SmartHomeClient::attemptLogin() {
    if (userEdit->text() == "admin" && passEdit->text() == "password") {
        centralStack->setCurrentWidget(dashboardWidget);
    }
    else {
        loginStatusLabel->setText("Invalid Credentials. Try admin/password");
    }
}

void SmartHomeClient::sendCommand() {
    if (isCommandPending) return;

    isCommandPending = true;
    sendCommandBtn->setEnabled(false);
    feedbackLabel->setText("Sending command... Waiting for server response.");
    feedbackLabel->setStyleSheet("color: orange;");

    // Enforce the 5-second timeout rule
    commandTimer->start(5000);

    // TODO: Birendra's packet logic goes here. Send the struct over the socket.

    // Mocking a successful server response after 2 seconds
    QTimer::singleShot(2000, this, &SmartHomeClient::simulateServerResponse);
}

void SmartHomeClient::onCommandTimeout() {
    isCommandPending = false;
    sendCommandBtn->setEnabled(true);
    feedbackLabel->setText("Error: Command failed. Server timeout (5 seconds exceeded).");
    feedbackLabel->setStyleSheet("color: red;");
}

void SmartHomeClient::simulateServerResponse() {
    if (!isCommandPending) return; // Ignore if the 5 seconds already passed

    commandTimer->stop();
    isCommandPending = false;
    sendCommandBtn->setEnabled(true);

    feedbackLabel->setText("Success: Appliance state updated.");
    feedbackLabel->setStyleSheet("color: green;");

    // Visually update the interactive floorplan appliance
    testAppliance->setBrush(QBrush(Qt::green));
}