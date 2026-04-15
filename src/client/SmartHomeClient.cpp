#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "SmartHomeClient.h"
#include "StyleManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QSettings>
#include <QDebug>
#include <QTimer> 
#include <QCoreApplication> 
#include "NetworkPacket.h"   
#include "NetworkManager.h"

// Page Includes
#include "pages/HomePage.h"
#include "pages/SignUpPage.h"
#include "pages/RoomDetailPage.h"
#include "pages/MapPage.h"
#include "pages/ProfilePage.h"
#include "pages/SettingsPage.h"

SmartHomeClient::SmartHomeClient(QWidget* parent)
    : QMainWindow(parent),
    isSidebarCollapsed(false),
    clientSocket(INVALID_SOCKET),
    serverIp("127.0.0.1"),
    serverPort(8080)
{
    localUserDb.push_back({ "admin", "password", "admin@smarthome.pro" });

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    setupUi();

    QSettings settings("SmartHomeProject", "ClientApp");
    userEdit->setText(settings.value("lastUser", "").toString());

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

    QPushButton* btnLogout = new QPushButton(" ⏻  LOGOUT");
    btnLogout->setStyleSheet("color: #E06C75; font-weight: bold;");

    layout->addWidget(btnHome);
    layout->addWidget(btnMap);
    layout->addWidget(btnUser);
    layout->addStretch();
    layout->addWidget(btnSet);
    layout->addWidget(btnLogout);

    QObject::connect(btnHome, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(homePage); });
    QObject::connect(btnMap, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(mapPage); });
    QObject::connect(btnUser, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(profilePage); });
    QObject::connect(btnSet, &QPushButton::clicked, [this]() { pageStack->setCurrentWidget(settingsPage); });
    QObject::connect(btnLogout, &QPushButton::clicked, this, &SmartHomeClient::logout);
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

    // Core Navigation
    QObject::connect(homePage, &HomePage::roomClicked, this, &SmartHomeClient::onRoomSelected);
    QObject::connect(mapPage, &MapPage::roomRequested, this, &SmartHomeClient::onRoomSelected);
    QObject::connect(mapPage, &MapPage::deviceRequested, this, &SmartHomeClient::onDeviceSelected);

    // REQ-SVR-040: State Machine
    QObject::connect(homePage, &HomePage::modeChangeRequested, this, &SmartHomeClient::handleModeChange);

    // REQ-SVR-070: Image Request
    QObject::connect(roomDetailPage, &RoomDetailPage::imageRequestTriggered, this, &SmartHomeClient::requestSecurityImage);

    QObject::connect(roomDetailPage, &RoomDetailPage::backButtonClicked, [this]() {
        pageStack->setCurrentWidget(homePage);
        });

    QObject::connect(signUpPage, &SignUpPage::registrationRequested, this, &SmartHomeClient::handleNewRegistration);
    QObject::connect(signUpPage, &SignUpPage::backToLoginRequested, [this]() {
        centralStack->setCurrentWidget(loginWidget);
        });
}

void SmartHomeClient::showToast(const QString& message, bool isError) {
    QLabel* toast = new QLabel(message, this);
    toast->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    toast->setAttribute(Qt::WA_TranslucentBackground);
    toast->setAlignment(Qt::AlignCenter);

    QString color = isError ? "#E06C75" : "#61AFEF";
    toast->setStyleSheet(QString(
        "background-color: rgba(30, 34, 42, 230); color: %1; "
        "border: 1px solid %1; border-radius: 10px; padding: 12px 20px; font-weight: bold;"
    ).arg(color));

    toast->adjustSize();
    int x = (this->width() - toast->width()) / 2;
    int y = this->height() - 100;
    toast->move(this->pos().x() + x, this->pos().y() + y);
    toast->show();

    QTimer::singleShot(2500, toast, &QLabel::deleteLater);
}

// REQ-SVR-070: High-Res Image Transfer (Fixed for Reliable Reception)
void SmartHomeClient::requestSecurityImage() {
    if (clientSocket == INVALID_SOCKET) {
        showToast("Error: No Connection", true);
        return;
    }

    showToast("Requesting Security Snapshot...");

    // Send Command 5 (Image Request)
    NetworkPacket req(5, "GET_IMG");
    if (!NetworkManager::sendPacket(clientSocket, req)) {
        showToast("Error: Command Failed", true);
        return;
    }

    // 1. Receive the 4-byte size header RELIABLY
    uint32_t imageSize = 0;
    if (!NetworkManager::recvAll(clientSocket, reinterpret_cast<char*>(&imageSize), 4)) {
        showToast("Error: Failed to receive image size", true);
        return;
    }

    // Safety check for size
    if (imageSize == 0 || imageSize > 5000000) {
        showToast("Error: Invalid Image Size", true);
        return;
    }

    // 2. Prepare buffer for exact file size
    std::vector<char> buffer(imageSize);
    int receivedSoFar = 0;

    // Use loop to gather the stream while keeping UI responsive
    while (receivedSoFar < (int)imageSize) {
        // We receive in chunks to ensure QCoreApplication::processEvents() runs
        int remaining = imageSize - receivedSoFar;
        int chunkSize = (remaining > 4096) ? 4096 : remaining;

        int n = recv(clientSocket, buffer.data() + receivedSoFar, chunkSize, 0);
        if (n <= 0) break;
        receivedSoFar += n;

        QCoreApplication::processEvents();
    }

    // 3. Render Image
    QPixmap pix;
    // Auto-detect format without hardcoding "JPG" hint
    if (receivedSoFar == (int)imageSize && pix.loadFromData(reinterpret_cast<uchar*>(buffer.data()), imageSize)) {
        roomDetailPage->updateCameraDisplay(pix);
        showToast(QString("Success: %1 KB Snapshot Received").arg(imageSize / 1024));
    }
    else {
        showToast("Error: Image Data Corrupt or Incomplete", true);
    }
}

bool SmartHomeClient::connectToServer(const std::string& ip, int port) {
    if (clientSocket != INVALID_SOCKET) closesocket(clientSocket);
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) return false;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        return false;
    }
    return true;
}

void SmartHomeClient::attemptLogin() {
    if (!connectToServer(serverIp, serverPort)) {
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

    if (found) {
        QSettings settings("SmartHomeProject", "ClientApp");
        settings.setValue("lastUser", inputUser);
        centralStack->setCurrentWidget(dashboardWidget);
        showToast("Welcome back, " + inputUser);
    }
    else {
        loginStatusLabel->setText("The username you entered does not exist. Please try again.");
        loginStatusLabel->setStyleSheet("color: #E06C75;");
    }
}

void SmartHomeClient::handleModeChange(int modeIndex) {
    if (clientSocket != INVALID_SOCKET) {
        NetworkPacket p(4, std::to_string(modeIndex));
        NetworkManager::sendPacket(clientSocket, p);
    }

    QStringList modes = { "LOCKED", "HOME", "AWAY", "MAINTENANCE" };
    showToast("Mode Request: " + modes[modeIndex]);
}

void SmartHomeClient::logout() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    passEdit->clear();
    centralStack->setCurrentWidget(loginWidget);
    showToast("Logged Out Safely");
}

void SmartHomeClient::handleNewRegistration(QString user, QString pass, QString email) {
    localUserDb.push_back({ user, pass, email });
    centralStack->setCurrentWidget(loginWidget);
    loginStatusLabel->setText("Registration Successful!");
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
