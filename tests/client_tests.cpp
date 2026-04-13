#include <winsock2.h>
#include <ws2tcpip.h>

#include <QSettings>
#include <QMetaObject>
#include <QtWidgets/QApplication>

#include <cassert>
#include <iostream>
#include <string>
#include <thread>

struct SmartHomeClientTestAccessor;

#include "../src/client/SmartHomeClient.h"

#include "../src/client/NetworkManager.h"
#include "../src/server/NetworkPacket.h"

struct SmartHomeClientTestAccessor
{
    static QStackedWidget* centralStack(SmartHomeClient& client) { return client.centralStack; }
    static QStackedWidget* pageStack(SmartHomeClient& client) { return client.pageStack; }
    static QWidget* loginWidget(SmartHomeClient& client) { return client.loginWidget; }
    static QWidget* dashboardWidget(SmartHomeClient& client) { return client.dashboardWidget; }
    static QWidget* sidebar(SmartHomeClient& client) { return client.sidebar; }
    static SignUpPage* signUpPage(SmartHomeClient& client) { return client.signUpPage; }
    static RoomDetailPage* roomDetailPage(SmartHomeClient& client) { return client.roomDetailPage; }
    static QLineEdit* userEdit(SmartHomeClient& client) { return client.userEdit; }
    static QLineEdit* passEdit(SmartHomeClient& client) { return client.passEdit; }
    static QLabel* loginStatusLabel(SmartHomeClient& client) { return client.loginStatusLabel; }
    static SOCKET& clientSocket(SmartHomeClient& client) { return client.clientSocket; }
    static std::vector<SmartHomeClient::UserAccount>& localUserDb(SmartHomeClient& client) { return client.localUserDb; }
    static std::string& serverIp(SmartHomeClient& client) { return client.serverIp; }
    static int& serverPort(SmartHomeClient& client) { return client.serverPort; }

    static QLabel* roomTitleLabel(RoomDetailPage& page) { return page.roomTitleLabel; }
    static QPushButton* requestImageButton(RoomDetailPage& page) { return page.btnRequestImage; }
};

namespace
{
    struct TestCase
    {
        const char* name;
        bool (*function)();
    };

    class WinsockSession
    {
    public:
        WinsockSession()
        {
            const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            assert(result == 0);
        }

        ~WinsockSession()
        {
            WSACleanup();
        }

    private:
        WSADATA wsaData{};
    };

    class LoopbackServer
    {
    public:
        explicit LoopbackServer(unsigned short requestedPort = 0)
        {
            listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            assert(listener != INVALID_SOCKET);

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = htons(requestedPort);

            assert(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
            assert(listen(listener, 1) == 0);

            int addressLength = sizeof(address);
            assert(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &addressLength) == 0);
            portValue = ntohs(address.sin_port);

            acceptThread = std::thread([this]() {
                accepted = accept(listener, nullptr, nullptr);
                });
        }

        ~LoopbackServer()
        {
            if (acceptThread.joinable())
            {
                acceptThread.join();
            }

            if (accepted != INVALID_SOCKET)
            {
                closesocket(accepted);
            }

            if (listener != INVALID_SOCKET)
            {
                closesocket(listener);
            }
        }

        unsigned short port() const
        {
            return portValue;
        }

        SOCKET socketHandle()
        {
            if (acceptThread.joinable())
            {
                acceptThread.join();
            }
            return accepted;
        }

    private:
        SOCKET listener{ INVALID_SOCKET };
        SOCKET accepted{ INVALID_SOCKET };
        unsigned short portValue{ 0 };
        std::thread acceptThread;
    };

    bool expect(bool condition, const char* passMessage, const char* failMessage)
    {
        if (condition)
        {
            std::cout << "PASS: " << passMessage << "\n";
            return true;
        }

        std::cout << "FAIL: " << failMessage << "\n";
        return false;
    }

    void clearClientSettings()
    {
        QSettings settings("SmartHomeProject", "ClientApp");
        settings.clear();
        settings.sync();
    }

    bool connectSocketToServer(SOCKET& socketHandle, unsigned short port)
    {
        socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socketHandle == INVALID_SOCKET)
        {
            return false;
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);

        if (connect(socketHandle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
        {
            closesocket(socketHandle);
            socketHandle = INVALID_SOCKET;
            return false;
        }

        return true;
    }
}

bool testLoginSuccessShowsDashboardAndPersistsUser()
{
    clearClientSettings();
    LoopbackServer server;
    SmartHomeClient client;
    SmartHomeClientTestAccessor::serverPort(client) = static_cast<int>(server.port());
    SmartHomeClientTestAccessor::userEdit(client)->setText("admin");
    SmartHomeClientTestAccessor::passEdit(client)->setText("password");

    const bool invoked = QMetaObject::invokeMethod(&client, "attemptLogin");

    QSettings settings("SmartHomeProject", "ClientApp");
    const bool passed = expect(
        invoked &&
        SmartHomeClientTestAccessor::centralStack(client)->currentWidget() == SmartHomeClientTestAccessor::dashboardWidget(client) &&
        settings.value("lastUser").toString() == "admin",
        "Client login opens dashboard and persists last user",
        "Client login should open dashboard and persist last user");
    clearClientSettings();
    return passed;
}

bool testInvalidCredentialsShowError()
{
    clearClientSettings();
    LoopbackServer server;
    SmartHomeClient client;
    SmartHomeClientTestAccessor::serverPort(client) = static_cast<int>(server.port());
    SmartHomeClientTestAccessor::userEdit(client)->setText("admin");
    SmartHomeClientTestAccessor::passEdit(client)->setText("wrong");

    const bool invoked = QMetaObject::invokeMethod(&client, "attemptLogin");

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::centralStack(client)->currentWidget() == SmartHomeClientTestAccessor::loginWidget(client) &&
        SmartHomeClientTestAccessor::loginStatusLabel(client)->text() == "Access Denied: Invalid Credentials",
        "Client invalid credentials stay on login and show an error",
        "Client invalid credentials should stay on login and show an error");
}

bool testShowSignUpPageNavigatesToRegistration()
{
    SmartHomeClient client;
    const bool invoked = QMetaObject::invokeMethod(&client, "showSignUpPage");

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::centralStack(client)->currentWidget() == SmartHomeClientTestAccessor::signUpPage(client),
        "Client can navigate from login to sign-up",
        "Client should navigate from login to sign-up");
}

bool testRegistrationAddsUserAndReturnsToLogin()
{
    SmartHomeClient client;
    const std::size_t initialCount = SmartHomeClientTestAccessor::localUserDb(client).size();

    const bool invoked = QMetaObject::invokeMethod(
        &client,
        "handleNewRegistration",
        Q_ARG(QString, QString("tester")),
        Q_ARG(QString, QString("secret")),
        Q_ARG(QString, QString("tester@example.com")));

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::localUserDb(client).size() == initialCount + 1 &&
        SmartHomeClientTestAccessor::localUserDb(client).back().username == "tester" &&
        SmartHomeClientTestAccessor::centralStack(client)->currentWidget() == SmartHomeClientTestAccessor::loginWidget(client) &&
        SmartHomeClientTestAccessor::loginStatusLabel(client)->text() == "Registration Successful!",
        "Client registration stores the account and returns to login",
        "Client registration should store the account and return to login");
}

bool testRegisteredUserCanLogin()
{
    clearClientSettings();
    LoopbackServer server;
    SmartHomeClient client;
    SmartHomeClientTestAccessor::serverPort(client) = static_cast<int>(server.port());
    const bool registrationInvoked = QMetaObject::invokeMethod(
        &client,
        "handleNewRegistration",
        Q_ARG(QString, QString("newuser")),
        Q_ARG(QString, QString("newpass")),
        Q_ARG(QString, QString("newuser@example.com")));
    SmartHomeClientTestAccessor::userEdit(client)->setText("newuser");
    SmartHomeClientTestAccessor::passEdit(client)->setText("newpass");

    const bool loginInvoked = QMetaObject::invokeMethod(&client, "attemptLogin");

    const bool passed = expect(
        registrationInvoked &&
        loginInvoked &&
        SmartHomeClientTestAccessor::centralStack(client)->currentWidget() == SmartHomeClientTestAccessor::dashboardWidget(client),
        "Newly registered client user can log in",
        "Newly registered client user should be able to log in");
    clearClientSettings();
    return passed;
}

bool testRoomSelectionLoadsGarageDetailPage()
{
    SmartHomeClient client;
    const bool invoked = QMetaObject::invokeMethod(
        &client,
        "onRoomSelected",
        Q_ARG(QString, QString("Garage")));

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::pageStack(client)->currentWidget() == SmartHomeClientTestAccessor::roomDetailPage(client) &&
        SmartHomeClientTestAccessor::roomTitleLabel(*SmartHomeClientTestAccessor::roomDetailPage(client))->text() == "GARAGE" &&
        SmartHomeClientTestAccessor::requestImageButton(*SmartHomeClientTestAccessor::roomDetailPage(client)) != nullptr,
        "Selecting Garage opens the security-enabled room detail page",
        "Selecting Garage should open the security-enabled room detail page");
}

bool testDeviceSelectionRoutesKitchenDevices()
{
    SmartHomeClient client;
    const bool invoked = QMetaObject::invokeMethod(
        &client,
        "onDeviceSelected",
        Q_ARG(QString, QString("KITCHEN_SENSOR_01")));

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::pageStack(client)->currentWidget() == SmartHomeClientTestAccessor::roomDetailPage(client) &&
        SmartHomeClientTestAccessor::roomTitleLabel(*SmartHomeClientTestAccessor::roomDetailPage(client))->text() == "KITCHEN",
        "Kitchen device identifiers route to the Kitchen room",
        "Kitchen device identifiers should route to the Kitchen room");
}

bool testDeviceSelectionDefaultsToLivingRoom()
{
    SmartHomeClient client;
    const bool invoked = QMetaObject::invokeMethod(
        &client,
        "onDeviceSelected",
        Q_ARG(QString, QString("UNKNOWN_DEVICE")));

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::pageStack(client)->currentWidget() == SmartHomeClientTestAccessor::roomDetailPage(client) &&
        SmartHomeClientTestAccessor::roomTitleLabel(*SmartHomeClientTestAccessor::roomDetailPage(client))->text() == "LIVING ROOM",
        "Unknown device identifiers default to the Living Room",
        "Unknown device identifiers should default to the Living Room");
}

bool testToggleSidebarCollapsesAndExpands()
{
    SmartHomeClient client;

    const bool firstInvoke = QMetaObject::invokeMethod(&client, "toggleSidebar");
    const bool collapsed = SmartHomeClientTestAccessor::sidebar(client)->minimumWidth() == 60 &&
        SmartHomeClientTestAccessor::sidebar(client)->maximumWidth() == 60;

    const bool secondInvoke = QMetaObject::invokeMethod(&client, "toggleSidebar");
    const bool expanded = SmartHomeClientTestAccessor::sidebar(client)->minimumWidth() == 200 &&
        SmartHomeClientTestAccessor::sidebar(client)->maximumWidth() == 200;

    return expect(
        firstInvoke && secondInvoke && collapsed && expanded,
        "Sidebar toggle collapses and expands the navigation rail",
        "Sidebar toggle should collapse and expand the navigation rail");
}

bool testLogoutClearsPasswordAndReturnsToLogin()
{
    SmartHomeClient client;
    SmartHomeClientTestAccessor::clientSocket(client) = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SmartHomeClientTestAccessor::passEdit(client)->setText("password");
    SmartHomeClientTestAccessor::centralStack(client)->setCurrentWidget(SmartHomeClientTestAccessor::dashboardWidget(client));

    const bool invoked = QMetaObject::invokeMethod(&client, "logout");

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::clientSocket(client) == INVALID_SOCKET &&
        SmartHomeClientTestAccessor::passEdit(client)->text().isEmpty() &&
        SmartHomeClientTestAccessor::centralStack(client)->currentWidget() == SmartHomeClientTestAccessor::loginWidget(client),
        "Logout closes the socket, clears the password, and returns to login",
        "Logout should close the socket, clear the password, and return to login");
}

bool testModeChangeSendsExpectedPacket()
{
    LoopbackServer server;
    SmartHomeClient client;
    const bool connected = connectSocketToServer(SmartHomeClientTestAccessor::clientSocket(client), server.port());
    if (!connected)
    {
        return expect(false, "", "Client should connect to the loopback server before sending a mode change");
    }

    const bool invoked = QMetaObject::invokeMethod(&client, "handleModeChange", Q_ARG(int, 2));

    NetworkPacket packet;
    const bool received = NetworkManager::receivePacket(server.socketHandle(), packet);
    const std::string payload = (packet.getPayload() && packet.getPayloadLength() > 0)
        ? std::string(packet.getPayload(), packet.getPayload() + packet.getPayloadLength())
        : std::string();

    return expect(
        invoked &&
        received &&
        packet.getCommandId() == 4 &&
        payload == "2",
        "Client mode changes send the expected network packet",
        "Client mode changes should send the expected network packet");
}

int runSelectedTests(int argc, char** argv, const TestCase* tests, int testCount)
{
    if (argc <= 1)
    {
        bool allPassed = true;
        for (int i = 0; i < testCount; ++i)
        {
            if (!tests[i].function())
            {
                allPassed = false;
            }
        }
        return allPassed ? 0 : 1;
    }

    const std::string selectedTest = argv[1];
    for (int i = 0; i < testCount; ++i)
    {
        if (selectedTest == tests[i].name)
        {
            return tests[i].function() ? 0 : 1;
        }
    }

    std::cerr << "Unknown client test: " << selectedTest << std::endl;
    return 1;
}

int main(int argc, char** argv)
{
    WinsockSession winsock;
    QApplication app(argc, argv);

    const TestCase tests[] = {
        {"login_success_shows_dashboard_and_persists_user", testLoginSuccessShowsDashboardAndPersistsUser},
        {"invalid_credentials_show_error", testInvalidCredentialsShowError},
        {"show_sign_up_page_navigates_to_registration", testShowSignUpPageNavigatesToRegistration},
        {"registration_adds_user_and_returns_to_login", testRegistrationAddsUserAndReturnsToLogin},
        {"registered_user_can_login", testRegisteredUserCanLogin},
        {"room_selection_loads_garage_detail_page", testRoomSelectionLoadsGarageDetailPage},
        {"device_selection_routes_kitchen_devices", testDeviceSelectionRoutesKitchenDevices},
        {"device_selection_defaults_to_living_room", testDeviceSelectionDefaultsToLivingRoom},
        {"toggle_sidebar_collapses_and_expands", testToggleSidebarCollapsesAndExpands},
        {"logout_clears_password_and_returns_to_login", testLogoutClearsPasswordAndReturnsToLogin},
        {"mode_change_sends_expected_packet", testModeChangeSendsExpectedPacket},
    };

    return runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));
}
