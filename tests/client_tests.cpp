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
    static QLabel* cameraMonitor(RoomDetailPage& page) { return page.cameraMonitor; }
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

    bool labelHasPixmap(const QLabel* label)
    {
        if (!label) return false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPixmap pix = label->pixmap();
        return !pix.isNull();
#else
        const QPixmap* pix = label->pixmap();
        return pix && !pix->isNull();
#endif
    }
}

// LOGIN TESTS

// this test checks login success opens dashboard and persists last user
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

// this test checks invalid credentials stay on login and show an error
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

// SIGN-UP TESTS

// this test checks navigating from login to sign-up
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

// this test checks registration stores the new account and returns to login
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

// this test checks a newly registered user can log in successfully
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

// NAVIGATION TESTS

// this test checks selecting Garage routes to the security-enabled room detail page
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

// this test checks Kitchen device identifiers route to the Kitchen room
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

// this test checks unknown device identifiers default to the Living Room
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

// UI BEHAVIOR TESTS

// this test checks sidebar toggle collapses and expands
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

// this test checks logout closes socket, clears password, and returns to login
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

// NETWORKING TESTS

// this test checks handleModeChange sends the expected packet to the server
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

// TEST RUNNER

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

// EDGE CASE TESTS

// this test checks offline login stays on login and shows server offline message
bool testLoginWhenServerOfflineShowsSystemError()
{
    clearClientSettings();
    SmartHomeClient client;
    SmartHomeClientTestAccessor::serverPort(client) = 6553; // assume unused port
    SmartHomeClientTestAccessor::userEdit(client)->setText("admin");
    SmartHomeClientTestAccessor::passEdit(client)->setText("password");

    const bool invoked = QMetaObject::invokeMethod(&client, "attemptLogin");

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::centralStack(client)->currentWidget() == SmartHomeClientTestAccessor::loginWidget(client) &&
        SmartHomeClientTestAccessor::loginStatusLabel(client)->text() == "SYSTEM ERROR: Server Offline",
        "Offline login keeps the user on login and shows server offline",
        "Offline login should keep the user on login and show server offline");
}

// this test checks saved last user is restored on startup
bool testSavedLastUserLoadsOnStartup()
{
    clearClientSettings();
    {
        QSettings settings("SmartHomeProject", "ClientApp");
        settings.setValue("lastUser", "remembered_user");
        settings.sync();
    }

    SmartHomeClient client;

    const bool passed = expect(
        SmartHomeClientTestAccessor::userEdit(client)->text() == "remembered_user",
        "Saved username is restored on client startup",
        "Saved username should be restored on client startup");

    clearClientSettings();
    return passed;
}

// this test checks Living Room device identifiers route correctly
bool testLivingRoomDeviceSelectionRoutesCorrectly()
{
    SmartHomeClient client;
    const bool invoked = QMetaObject::invokeMethod(
        &client,
        "onDeviceSelected",
        Q_ARG(QString, QString("LR_LIGHT_01")));

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::pageStack(client)->currentWidget() == SmartHomeClientTestAccessor::roomDetailPage(client) &&
        SmartHomeClientTestAccessor::roomTitleLabel(*SmartHomeClientTestAccessor::roomDetailPage(client))->text() == "LIVING ROOM",
        "Living room device identifiers route to the Living Room",
        "Living room device identifiers should route to the Living Room");
}

// this test checks mode change without connection returns safely
bool testModeChangeWithoutConnectionStillInvokesSafely()
{
    SmartHomeClient client;
    SmartHomeClientTestAccessor::clientSocket(client) = INVALID_SOCKET;

    const bool invoked = QMetaObject::invokeMethod(&client, "handleModeChange", Q_ARG(int, 1));

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::clientSocket(client) == INVALID_SOCKET,
        "Mode change without a connection still returns safely",
        "Mode change without a connection should not modify the socket or fail");
}

// this test checks security image request without connection is handled safely
bool testRequestSecurityImageWithoutConnectionIsHandled()
{
    SmartHomeClient client;
    SmartHomeClientTestAccessor::clientSocket(client) = INVALID_SOCKET;

    const bool invoked = QMetaObject::invokeMethod(&client, "requestSecurityImage");

    return expect(
        invoked &&
        SmartHomeClientTestAccessor::clientSocket(client) == INVALID_SOCKET,
        "Security image request without a connection is handled safely",
        "Security image request without a connection should be handled safely");
}

// this test checks requestSecurityImage sends a request and updates the camera monitor on a valid image response
bool testRequestSecurityImageValidImageUpdatesMonitor()
{
    LoopbackServer server;
    SmartHomeClient client;

    const bool connected = connectSocketToServer(SmartHomeClientTestAccessor::clientSocket(client), server.port());
    if (!connected)
    {
        return expect(false, "", "Client should connect to the loopback server before requesting an image");
    }

    // Ensure the Garage page is loaded, which creates the camera monitor label.
    const bool navigated = QMetaObject::invokeMethod(&client, "onRoomSelected", Q_ARG(QString, QString("Garage")));
    if (!navigated)
    {
        return expect(false, "", "Client should navigate to Garage before requesting an image");
    }

    QLabel* monitor = SmartHomeClientTestAccessor::cameraMonitor(*SmartHomeClientTestAccessor::roomDetailPage(client));
    if (!monitor)
    {
        return expect(false, "", "Garage page should have a camera monitor label");
    }

    // Minimal 1x1 PNG. QPixmap auto-detects the image format from bytes.
    static const unsigned char kTinyPng[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
        0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
        0x08,0x06,0x00,0x00,0x00,0x1F,0x15,0xC4,0x89,
        0x00,0x00,0x00,0x0A,0x49,0x44,0x41,0x54,
        0x78,0x9C,0x63,0x00,0x01,0x00,0x00,0x05,0x00,0x01,0x0D,0x0A,0x2D,0xB4,
        0x00,0x00,0x00,0x00,0x49,0x45,0x4E,0x44,0xAE,0x42,0x60,0x82
    };

    std::thread responder([&]() {
        SOCKET s = server.socketHandle();
        NetworkPacket request;
        const bool received = NetworkManager::receivePacket(s, request);
        assert(received);
        assert(request.getCommandId() == 5);

        const uint32_t imageSize = static_cast<uint32_t>(sizeof(kTinyPng));
        const bool sentSize = NetworkManager::sendAll(s, reinterpret_cast<const char*>(&imageSize), 4);
        assert(sentSize);
        const bool sentData = NetworkManager::sendAll(s, reinterpret_cast<const char*>(kTinyPng), static_cast<int>(imageSize));
        assert(sentData);
        });

    const bool invoked = QMetaObject::invokeMethod(&client, "requestSecurityImage");
    responder.join();

    return expect(
        invoked &&
        monitor->text().isEmpty() &&
        labelHasPixmap(monitor),
        "Valid security image updates the camera monitor",
        "Valid security image should update the camera monitor");
}

// this test checks requestSecurityImage rejects a zero-size header without updating the monitor
bool testRequestSecurityImageZeroSizeDoesNotUpdateMonitor()
{
    LoopbackServer server;
    SmartHomeClient client;

    const bool connected = connectSocketToServer(SmartHomeClientTestAccessor::clientSocket(client), server.port());
    if (!connected)
    {
        return expect(false, "", "Client should connect to the loopback server before requesting an image");
    }

    QMetaObject::invokeMethod(&client, "onRoomSelected", Q_ARG(QString, QString("Garage")));
    QLabel* monitor = SmartHomeClientTestAccessor::cameraMonitor(*SmartHomeClientTestAccessor::roomDetailPage(client));
    if (!monitor)
    {
        return expect(false, "", "Garage page should have a camera monitor label");
    }

    const QString initialText = monitor->text();

    std::thread responder([&]() {
        SOCKET s = server.socketHandle();
        NetworkPacket request;
        const bool received = NetworkManager::receivePacket(s, request);
        assert(received);
        assert(request.getCommandId() == 5);

        const uint32_t imageSize = 0;
        const bool sentSize = NetworkManager::sendAll(s, reinterpret_cast<const char*>(&imageSize), 4);
        assert(sentSize);
        });

    const bool invoked = QMetaObject::invokeMethod(&client, "requestSecurityImage");
    responder.join();

    return expect(
        invoked &&
        monitor->text() == initialText &&
        !labelHasPixmap(monitor),
        "Zero-size image response is rejected without updating monitor",
        "Zero-size image response should be rejected without updating monitor");
}

// this test checks requestSecurityImage rejects oversized headers without updating the monitor
bool testRequestSecurityImageOversizedDoesNotUpdateMonitor()
{
    LoopbackServer server;
    SmartHomeClient client;

    const bool connected = connectSocketToServer(SmartHomeClientTestAccessor::clientSocket(client), server.port());
    if (!connected)
    {
        return expect(false, "", "Client should connect to the loopback server before requesting an image");
    }

    QMetaObject::invokeMethod(&client, "onRoomSelected", Q_ARG(QString, QString("Garage")));
    QLabel* monitor = SmartHomeClientTestAccessor::cameraMonitor(*SmartHomeClientTestAccessor::roomDetailPage(client));
    if (!monitor)
    {
        return expect(false, "", "Garage page should have a camera monitor label");
    }

    const QString initialText = monitor->text();

    std::thread responder([&]() {
        SOCKET s = server.socketHandle();
        NetworkPacket request;
        const bool received = NetworkManager::receivePacket(s, request);
        assert(received);
        assert(request.getCommandId() == 5);

        const uint32_t imageSize = 6000000; // > 5,000,000 safety limit in SmartHomeClient
        const bool sentSize = NetworkManager::sendAll(s, reinterpret_cast<const char*>(&imageSize), 4);
        assert(sentSize);
        });

    const bool invoked = QMetaObject::invokeMethod(&client, "requestSecurityImage");
    responder.join();

    return expect(
        invoked &&
        monitor->text() == initialText &&
        !labelHasPixmap(monitor),
        "Oversized image response is rejected without updating monitor",
        "Oversized image response should be rejected without updating monitor");
}

// this test checks requestSecurityImage does not update the monitor on corrupt image payloads
bool testRequestSecurityImageCorruptPayloadDoesNotUpdateMonitor()
{
    LoopbackServer server;
    SmartHomeClient client;

    const bool connected = connectSocketToServer(SmartHomeClientTestAccessor::clientSocket(client), server.port());
    if (!connected)
    {
        return expect(false, "", "Client should connect to the loopback server before requesting an image");
    }

    QMetaObject::invokeMethod(&client, "onRoomSelected", Q_ARG(QString, QString("Garage")));
    QLabel* monitor = SmartHomeClientTestAccessor::cameraMonitor(*SmartHomeClientTestAccessor::roomDetailPage(client));
    if (!monitor)
    {
        return expect(false, "", "Garage page should have a camera monitor label");
    }

    const QString initialText = monitor->text();

    std::thread responder([&]() {
        SOCKET s = server.socketHandle();
        NetworkPacket request;
        const bool received = NetworkManager::receivePacket(s, request);
        assert(received);
        assert(request.getCommandId() == 5);

        const uint32_t imageSize = 128;
        const bool sentSize = NetworkManager::sendAll(s, reinterpret_cast<const char*>(&imageSize), 4);
        assert(sentSize);

        std::vector<char> garbage(imageSize, 0);
        const bool sentData = NetworkManager::sendAll(s, garbage.data(), static_cast<int>(garbage.size()));
        assert(sentData);
        });

    const bool invoked = QMetaObject::invokeMethod(&client, "requestSecurityImage");
    responder.join();

    return expect(
        invoked &&
        monitor->text() == initialText &&
        !labelHasPixmap(monitor),
        "Corrupt image payload does not update the camera monitor",
        "Corrupt image payload should not update the camera monitor");
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
        {"login_when_server_offline_shows_system_error", testLoginWhenServerOfflineShowsSystemError},
        {"saved_last_user_loads_on_startup", testSavedLastUserLoadsOnStartup},
        {"living_room_device_selection_routes_correctly", testLivingRoomDeviceSelectionRoutesCorrectly},
        {"mode_change_without_connection_still_invokes_safely", testModeChangeWithoutConnectionStillInvokesSafely},
        {"request_security_image_without_connection_is_handled", testRequestSecurityImageWithoutConnectionIsHandled},
        {"request_security_image_valid_image_updates_monitor", testRequestSecurityImageValidImageUpdatesMonitor},
        {"request_security_image_zero_size_does_not_update_monitor", testRequestSecurityImageZeroSizeDoesNotUpdateMonitor},
        {"request_security_image_oversized_does_not_update_monitor", testRequestSecurityImageOversizedDoesNotUpdateMonitor},
        {"request_security_image_corrupt_payload_does_not_update_monitor", testRequestSecurityImageCorruptPayloadDoesNotUpdateMonitor},
    };

    return runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));
}
