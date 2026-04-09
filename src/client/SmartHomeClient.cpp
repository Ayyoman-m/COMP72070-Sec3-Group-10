#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "SmartHomeClient.h"
#include "StyleManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>

// Crucial: These includes must be here so the compiler knows the Signal/Slot signatures
#include "pages/HomePage.h"
#include "pages/SignUpPage.h"
#include "pages/RoomDetailPage.h"

SmartHomeClient::SmartHomeClient(QWidget* parent)
    : QMainWindow(parent), isSidebarCollapsed(false), clientSocket(INVALID_SOCKET)
{
    // Requirement #7: Initialize local DB
    localUserDb.push_back({ "admin", "password", "admin@smarthome.pro" });

    // Networking Init
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    setupUi();
    this->setStyleSheet(StyleManager::getMainWindowStyle());
}

SmartHomeClient::~SmartHomeClient() {
    if (clientSocket != INVALID_SOCKET) closesocket(clientSocket);
    WSACleanup();
}

void SmartHomeClient::setupUi() {
    this->setWindowTitle("SmartHome Pro v2.0");
    this->resize(1200, 800);

    centralStack = new QStackedWidget(this);
    this->setCentralWidget(centralStack);

    // LOGIN SCREEN
    loginWidget = new QWidget();
    QVBoxLayout* lLayout = new QVBoxLayout(loginWidget);
    lLayout->setContentsMargins(350, 150, 350, 150);

    userEdit = new QLineEdit();
    userEdit->setPlaceholderText("Username");
    userEdit->setStyleSheet(StyleManager::getLoginInputStyle());

    passEdit = new QLineEdit();
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setStyleSheet(StyleManager::getLoginInputStyle());

    QPushButton* btnLogin = new QPushButton("LOGIN");
    btnLogin->setStyleSheet("background-color: #61AFEF; color: #12151A; font-weight: bold; padding: 12px; border-radius: 5px;");

    QPushButton* btnGoToSignUp = new QPushButton("Don't have an account? Sign Up");
    btnGoToSignUp->setStyleSheet("background: transparent; color: #ABB2BF; border: none; text-decoration: underline;");

    loginStatusLabel = new QLabel("");
    loginStatusLabel->setAlignment(Qt::AlignCenter);

    lLayout->addWidget(new QLabel("<h1 style='color:#61AFEF; text-align:center;'>SmartHome Access</h1>"));
    lLayout->addWidget(userEdit);
    lLayout->addWidget(passEdit);
    lLayout->addWidget(btnLogin);
    lLayout->addWidget(btnGoToSignUp);
    lLayout->addWidget(loginStatusLabel);
    lLayout->addStretch();

    // Fix for E0304: Use QObject::connect to avoid conflict with winsock connect()
    QObject::connect(btnLogin, &QPushButton::clicked, this, &SmartHomeClient::attemptLogin);
    QObject::connect(btnGoToSignUp, &QPushButton::clicked, this, &SmartHomeClient::showSignUpPage);

    centralStack->addWidget(loginWidget);

    // DASHBOARD SHELL
    dashboardWidget = new QWidget();
    QHBoxLayout* dashLayout = new QHBoxLayout(dashboardWidget);
    dashLayout->setSpacing(0); dashLayout->setContentsMargins(0, 0, 0, 0);

    setupSidebar();
    setupPages();

    dashLayout->addWidget(sidebar);
    dashLayout->addWidget(pageStack);
    centralStack->addWidget(dashboardWidget);

    centralStack->setCurrentWidget(loginWidget);
}

void SmartHomeClient::setupSidebar() {
    sidebar = new QWidget();
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet(StyleManager::getSidebarStyle());

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->addWidget(new QLabel("<h3 style='color:#61AFEF; padding: 15px;'>PRO HUB</h3>"));

    QPushButton* btnHome = new QPushButton(" ⌂  DASHBOARD");
    QPushButton* btnMap = new QPushButton(" 🗺️  FLOORPLAN");
    QPushButton* btnUser = new QPushButton(" 👤  PROFILE");
    QPushButton* btnSet = new QPushButton(" ⚙️  SETTINGS");

    layout->addWidget(btnHome);
    layout->addWidget(btnMap);
    layout->addWidget(btnUser);
    layout->addStretch();
    layout->addWidget(btnSet);

    QObject::connect(btnHome, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(homePage); });
    QObject::connect(btnMap, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(mapPage); });
    QObject::connect(btnUser, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(profilePage); });
    QObject::connect(btnSet, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(settingsPage); });
}

void SmartHomeClient::setupPages() {
    pageStack = new QStackedWidget();

    homePage = new HomePage();
    mapPage = new MapPage();
    profilePage = new ProfilePage();
    settingsPage = new SettingsPage();
    roomDetailPage = new RoomDetailPage();
    signUpPage = new SignUpPage();

    pageStack->addWidget(homePage);
    pageStack->addWidget(mapPage);
    pageStack->addWidget(profilePage);
    pageStack->addWidget(settingsPage);
    pageStack->addWidget(roomDetailPage);

    centralStack->addWidget(signUpPage);

    // Fix for E0304: Explicit QObject scope for all connections
    QObject::connect(homePage, &HomePage::roomClicked, this, &SmartHomeClient::onRoomSelected);
    QObject::connect(mapPage, &MapPage::roomRequested, this, &SmartHomeClient::onRoomSelected);
    QObject::connect(mapPage, &MapPage::deviceRequested, this, &SmartHomeClient::onDeviceSelected);

    QObject::connect(roomDetailPage, &RoomDetailPage::backButtonClicked, [this]() {
        pageStack->setCurrentWidget(homePage);
        });

    QObject::connect(signUpPage, &SignUpPage::registrationRequested, this, &SmartHomeClient::handleNewRegistration);
    QObject::connect(signUpPage, &SignUpPage::backToLoginRequested, [this]() {
        centralStack->setCurrentWidget(loginWidget);
        });
}

bool SmartHomeClient::connectToServer(const std::string& ip, int port) {
    if (clientSocket != INVALID_SOCKET) closesocket(clientSocket);
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) return false;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // This is the Winsock connect, NOT the Qt connect
    if (::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        return false;
    }
    return true;
}

void SmartHomeClient::attemptLogin() {
    if (!connectToServer("127.0.0.1", 8080)) {
        loginStatusLabel->setText("SYSTEM ERROR: Server Offline");
        loginStatusLabel->setStyleSheet("color: #E06C75; font-weight: bold;");
        return;
    }

    QString inputUser = userEdit->text();
    QString inputPass = passEdit->text();
    bool found = false;
    for (const auto& account : localUserDb) {
        if (account.username == inputUser && account.password == inputPass) {
            found = true;
            break;
        }
    }

    if (found) centralStack->setCurrentWidget(dashboardWidget);
    else {
        loginStatusLabel->setText("Access Denied: Invalid Credentials");
        loginStatusLabel->setStyleSheet("color: #E06C75;");
    }
}

void SmartHomeClient::handleNewRegistration(QString user, QString pass, QString email) {
    localUserDb.push_back({ user, pass, email });
    centralStack->setCurrentWidget(loginWidget);
    loginStatusLabel->setText("Registration Successful! Please Login.");
    loginStatusLabel->setStyleSheet("color: #98C379;");
}

void SmartHomeClient::showSignUpPage() {
    centralStack->setCurrentWidget(signUpPage);
}

void SmartHomeClient::onRoomSelected(const QString& roomName) {
    roomDetailPage->loadRoom(roomName);
    pageStack->setCurrentWidget(roomDetailPage);
}

void SmartHomeClient::onDeviceSelected(const QString& deviceId) {
    if (deviceId.contains("LR")) onRoomSelected("Living Room");
    else if (deviceId.contains("KITCHEN")) onRoomSelected("Kitchen");
    else onRoomSelected("Living Room");
}

void SmartHomeClient::toggleSidebar() {
    isSidebarCollapsed = !isSidebarCollapsed;
    sidebar->setFixedWidth(isSidebarCollapsed ? 60 : 200);
}