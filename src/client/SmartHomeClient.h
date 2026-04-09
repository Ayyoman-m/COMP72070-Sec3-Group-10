#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <QMainWindow>
#include <QStackedWidget>
#include <vector>
#include <QString>

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
    void onDeviceSelected(const QString& deviceId); // New: From Map
    void toggleSidebar();

private:
    void setupUi();
    void setupSidebar();
    void setupPages();

    // Networking Core (Requirement: Real Connection Check)
    bool connectToServer(const std::string& ip, int port);

    bool isSidebarCollapsed;

    QStackedWidget* centralStack;
    QStackedWidget* pageStack;

    QWidget* loginWidget;
    QWidget* dashboardWidget;
    QWidget* sidebar;

    HomePage* homePage;
    MapPage* mapPage;
    ProfilePage* profilePage;
    SettingsPage* settingsPage;
    RoomDetailPage* roomDetailPage;
    SignUpPage* signUpPage;

    QLineEdit* userEdit;
    QLineEdit* passEdit;
    QLabel* loginStatusLabel;

    SOCKET clientSocket;
    std::vector<UserAccount> localUserDb;
};