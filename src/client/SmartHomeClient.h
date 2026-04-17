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

/**
 * @class SmartHomeClient
 * @brief Main client application window for the Smart Home system.
 *
 * This class manages the graphical user interface, user authentication,
 * navigation between pages, and communication with the server.
 * It acts as the central controller for the client-side application.
 */
class SmartHomeClient : public QMainWindow {
    Q_OBJECT
    friend struct SmartHomeClientTestAccessor;

public:
    /**
     * @struct UserAccount
     * @brief Stores user account information for authentication.
     */
    struct UserAccount {
        QString username;
        QString password;
        QString email;
    };

    /**
     * @brief Constructs the Smart Home Client window.
     * @param parent Parent widget
     */
    explicit SmartHomeClient(QWidget* parent = nullptr);

    /**
    * @brief Destructor for SmartHomeClient.
    */
    ~SmartHomeClient();

private slots:

    /**
     * @brief Attempts to log in the user.
     */
    void attemptLogin();

    /**
     * @brief Displays the sign-up page.
     */
    void showSignUpPage();

    /**
    * @brief Handles new user registration.
    * @param user Username
    * @param pass Password
    * @param email Email address
    */
    void handleNewRegistration(QString user, QString pass, QString email);

    /**
    * @brief Triggered when a room is selected.
    * @param roomName Name of the selected room
    */
    void onRoomSelected(const QString& roomName);

    /**
   * @brief Triggered when a device is selected.
   * @param deviceId ID of the selected device
   */
    void onDeviceSelected(const QString& deviceId);

    /**
     * @brief Toggles the sidebar visibility.
     */

    void toggleSidebar();

    /**
     * @brief Handles system mode changes.
     * @param modeIndex Index of selected mode
     */
    void handleModeChange(int modeIndex);
    void logout();
    /**
    * @brief Requests a security image from the server.
    *
    * Implements REQ-SVR-070 for image transfer.
    */

    void requestSecurityImage();

private:
    /**
     * @brief Initializes the UI components.
     */
    void setupUi();

    /**
    * @brief Sets up the sidebar layout and buttons.
    */
    void setupSidebar();

    /**
     * @brief Initializes application pages.
     */
    void setupPages();

    /**
     * @brief Displays a temporary notification message.
     * @param message Text to display
     * @param isError Indicates if message is an error
     */
    void showToast(const QString& message, bool isError = false);

    /**
     * @brief Connects the client to the server.
     * @param ip Server IP address
     * @param port Server port number
     * @return true if connection is successful, false otherwise
     */
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
    std::string serverIp;
    int serverPort;
};
