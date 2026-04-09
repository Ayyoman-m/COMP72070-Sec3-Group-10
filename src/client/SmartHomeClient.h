#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit> // Added for userEdit/passEdit
#include <QLabel>    // Added for loginStatusLabel
#include <vector>
#include <string>    // Added for std::string in connectToServer
#include <QString>

// Page Includes
#include "pages/HomePage.h"
#include "pages/MapPage.h"
#include "pages/ProfilePage.h"
#include "pages/SettingsPage.h"
#include "pages/RoomDetailPage.h"
#include "pages/SignUpPage.h"

class SmartHomeClient : public QMainWindow {
    Q_OBJECT

public:
    struct UserAccount {
        QString username;
        QString password;
        QString email;
    };

    explicit SmartHomeClient(QWidget* parent = nullptr);
    ~SmartHomeClient();

private slots:
    // Authentication & Navigation
    void attemptLogin();
    void showSignUpPage();
    void handleNewRegistration(QString user, QString pass, QString email);

    // Page Routing
    void onRoomSelected(const QString& roomName);
    void onDeviceSelected(const QString& deviceId);
    void toggleSidebar();

    // Mode & Session Management
    void handleModeChange(int modeIndex);
    void logout();

    // REQ-SVR-070: Image Request Slot
    void requestSecurityImage();

private:
    void setupUi();
    void setupSidebar();
    void setupPages();
    void showToast(const QString& message, bool isError = false);

    // Networking Core
    bool connectToServer(const std::string& ip, int port);

    bool isSidebarCollapsed;

    // UI Stack Components
    QStackedWidget* centralStack;
    QStackedWidget* pageStack;

    QWidget* loginWidget;
    QWidget* dashboardWidget;
    QWidget* sidebar;

    // Page Pointers
    HomePage* homePage;
    MapPage* mapPage;
    ProfilePage* profilePage;
    SettingsPage* settingsPage;
    RoomDetailPage* roomDetailPage;
    SignUpPage* signUpPage;

    // Inputs & Status
    QLineEdit* userEdit;
    QLineEdit* passEdit;
    QLabel* loginStatusLabel;

    // Networking Data
    SOCKET clientSocket;
    std::vector<UserAccount> localUserDb;
};