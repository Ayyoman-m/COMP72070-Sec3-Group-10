#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "../src/server/Header/StateMachine.h"
#include "../src/server/Header/RequestHandler.h"
#include "../src/server/Header/AuthManager.h"
#include "../src/server/Header/DeviceManager.h"
#include "../src/server/Header/LogManager.h"
#include "../src/server/NetworkManager.h"
#include "../src/server/NetworkPacket.h"

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
            // Close the listener first so a blocking accept() unblocks and the thread can join.
            if (listener != INVALID_SOCKET)
            {
                closesocket(listener);
                listener = INVALID_SOCKET;
            }

            if (acceptThread.joinable())
            {
                acceptThread.join();
            }

            if (accepted != INVALID_SOCKET)
            {
                closesocket(accepted);
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


// STATE MACHINE TESTS

// this test checks if the server starts in LOCKED state
bool testInitialState() {
    StateMachine sm;

    if (sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: Initial state is LOCKED\n";
        return true;
    }
    else {
        std::cout << "FAIL: Initial state should be LOCKED\n";
        return false;
    }
}

// this test checks valid transition from LOCKED to HOME
bool testLockedToHome() {
    StateMachine sm;

    bool result = sm.setState(ServerState::HOME);

    if (result && sm.getState() == ServerState::HOME) {
        std::cout << "PASS: LOCKED -> HOME\n";
        return true;
    }
    else {
        std::cout << "FAIL: LOCKED -> HOME should be valid\n";
        return false;
    }
}

// this test checks invalid transition from LOCKED to AWAY
bool testLockedToAwayInvalid() {
    StateMachine sm;

    bool result = sm.setState(ServerState::AWAY);

    if (!result && sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: LOCKED -> AWAY rejected\n";
        return true;
    }
    else {
        std::cout << "FAIL: LOCKED -> AWAY should be invalid\n";
        return false;
    }
}

// this test checks valid transition from HOME to AWAY
bool testHomeToAway() {
    StateMachine sm;
    sm.setState(ServerState::HOME);

    bool result = sm.setState(ServerState::AWAY);

    if (result && sm.getState() == ServerState::AWAY) {
        std::cout << "PASS: HOME -> AWAY\n";
        return true;
    }
    else {
        std::cout << "FAIL: HOME -> AWAY should be valid\n";
        return false;
    }
}

// REQUEST HANDLER TESTS

// this test checks GET_STATUS returns LOCKED at start
bool testGetStatusInitiallyLocked() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    Packet packet{ CommandID::GET_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "LOCKED") {
        std::cout << "PASS: GET_STATUS returns LOCKED\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_STATUS should return LOCKED\n";
        return false;
    }
}

// this test checks GET_STATUS returns HOME after transition
bool testGetStatusHome() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    sm.setState(ServerState::HOME);

    Packet packet{ CommandID::GET_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "HOME") {
        std::cout << "PASS: GET_STATUS returns HOME\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_STATUS should return HOME\n";
        return false;
    }
}

// this test checks GET_STATUS returns AWAY after transition
bool testGetStatusAway() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    sm.setState(ServerState::HOME);
    sm.setState(ServerState::AWAY);

    Packet packet{ CommandID::GET_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "AWAY") {
        std::cout << "PASS: GET_STATUS returns AWAY\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_STATUS should return AWAY\n";
        return false;
    }
}

// this test checks GET_STATUS returns MAINTENANCE after transition
bool testGetStatusMaintenance() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    sm.setState(ServerState::HOME);
    sm.setState(ServerState::MAINTENANCE);

    Packet packet{ CommandID::GET_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "MAINTENANCE") {
        std::cout << "PASS: GET_STATUS returns MAINTENANCE\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_STATUS should return MAINTENANCE\n";
        return false;
    }
}

// this test checks SET_MODE fails without login
bool testSetModeWithoutAuthFails() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    Packet packet{ CommandID::SET_MODE, "HOME" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: NOT AUTHENTICATED") {
        std::cout << "PASS: SET_MODE blocked without auth\n";
        return true;
    }
    else {
        std::cout << "FAIL: SET_MODE should fail without auth\n";
        return false;
    }
}

// this test checks SET_MODE works after login
bool testSetModeWithAuthWorks() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::SET_MODE, "HOME" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "SUCCESS" && sm.getState() == ServerState::HOME) {
        std::cout << "PASS: SET_MODE HOME works after login\n";
        return true;
    }
    else {
        std::cout << "FAIL: SET_MODE HOME should work after login\n";
        return false;
    }
}

// this test checks SET_MODE MAINTENANCE works after login
bool testSetModeMaintenanceWithAuthWorks() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);

    Packet packet{ CommandID::SET_MODE, "MAINTENANCE" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "SUCCESS" && sm.getState() == ServerState::MAINTENANCE) {
        std::cout << "PASS: SET_MODE MAINTENANCE works after login\n";
        return true;
    }
    else {
        std::cout << "FAIL: SET_MODE MAINTENANCE should work after login\n";
        return false;
    }
}

// this test checks SET_MODE LOCKED works after login
bool testSetModeLockedWithAuthWorks() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);

    Packet packet{ CommandID::SET_MODE, "LOCKED" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "SUCCESS" && sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: SET_MODE LOCKED works after login\n";
        return true;
    }
    else {
        std::cout << "FAIL: SET_MODE LOCKED should work after login\n";
        return false;
    }
}

// this test checks SET_MODE returns INVALID TRANSITION for a disallowed transition
bool testSetModeInvalidTransitionReturnsError() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::SET_MODE, "AWAY" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: INVALID TRANSITION" && sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: SET_MODE invalid transition rejected\n";
        return true;
    }
    else {
        std::cout << "FAIL: SET_MODE invalid transition should be rejected\n";
        return false;
    }
}

// AUTH MANAGER TESTS

// this test checks valid login
bool testValidLogin() {
    AuthManager auth;
    ClientSession session;

    bool result = auth.login("admin", "1234", session);

    if (result && session.isAuthenticated()) {
        std::cout << "PASS: Valid login works\n";
        return true;
    }
    else {
        std::cout << "FAIL: Valid login should succeed\n";
        return false;
    }
}

// this test checks invalid login
bool testInvalidLogin() {
    AuthManager auth;
    ClientSession session;

    bool result = auth.login("admin", "wrong", session);

    if (!result && !session.isAuthenticated()) {
        std::cout << "PASS: Invalid login rejected\n";
        return true;
    }
    else {
        std::cout << "FAIL: Invalid login should fail\n";
        return false;
    }
}

// DEVICE MANAGER TESTS

// this test checks device turns ON
bool testTurnOnDevice() {
    DeviceManager dm;

    bool result = dm.turnOn("LIGHT");

    if (result && dm.getStatus("LIGHT") == "ON") {
        std::cout << "PASS: LIGHT turned ON\n";
        return true;
    }
    else {
        std::cout << "FAIL: LIGHT should turn ON\n";
        return false;
    }
}

// this test checks device turns OFF
bool testTurnOffDevice() {
    DeviceManager dm;

    dm.turnOn("FAN");
    bool result = dm.turnOff("FAN");

    if (result && dm.getStatus("FAN") == "OFF") {
        std::cout << "PASS: FAN turned OFF\n";
        return true;
    }
    else {
        std::cout << "FAIL: FAN should turn OFF\n";
        return false;
    }
}

// this test checks invalid device handling
bool testInvalidDevice() {
    DeviceManager dm;

    std::string result = dm.getStatus("TV");

    if (result == "DEVICE NOT FOUND") {
        std::cout << "PASS: Invalid device handled\n";
        return true;
    }
    else {
        std::cout << "FAIL: Invalid device should return error\n";
        return false;
    }
}

// DEVICE INTEGRATION TESTS

// this test checks turning on device through request handler
bool testTurnOnDeviceThroughHandler() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);

    Packet packet{ CommandID::TURN_ON_DEVICE, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "SUCCESS" && dm.getStatus("LIGHT") == "ON") {
        std::cout << "PASS: RequestHandler turns LIGHT ON\n";
        return true;
    }
    else {
        std::cout << "FAIL: RequestHandler should turn LIGHT ON\n";
        return false;
    }
}

// this test checks getting device status through request handler
bool testGetDeviceStatusThroughHandler() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);
    dm.turnOn("FAN");

    Packet packet{ CommandID::GET_DEVICE_STATUS, "FAN" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ON") {
        std::cout << "PASS: RequestHandler gets device status\n";
        return true;
    }
    else {
        std::cout << "FAIL: RequestHandler should return device status\n";
        return false;
    }
}

// this test checks device command is rejected in LOCKED
bool testDeviceCommandRejectedInLocked() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::TURN_ON_DEVICE, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: Device command rejected in LOCKED\n";
        return true;
    }
    else {
        std::cout << "FAIL: Device command should be rejected in LOCKED\n";
        return false;
    }
}

// this test checks device command is rejected in MAINTENANCE
bool testDeviceCommandRejectedInMaintenance() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);
    sm.setState(ServerState::MAINTENANCE);

    Packet packet{ CommandID::TURN_ON_DEVICE, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: Device command rejected in MAINTENANCE\n";
        return true;
    }
    else {
        std::cout << "FAIL: Device command should be rejected in MAINTENANCE\n";
        return false;
    }
}

// this test checks device command works in HOME
bool testDeviceCommandWorksInHome() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);

    Packet packet{ CommandID::TURN_ON_DEVICE, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "SUCCESS" && dm.getStatus("LIGHT") == "ON") {
        std::cout << "PASS: Device command works in HOME\n";
        return true;
    }
    else {
        std::cout << "FAIL: Device command should work in HOME\n";
        return false;
    }
}

// this test checks full device status list
bool testGetAllDeviceStatus() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);

    dm.turnOn("LIGHT");
    dm.turnOff("FAN");

    Packet packet{ CommandID::GET_ALL_DEVICE_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result.find("LIGHT=ON") != std::string::npos) {
        std::cout << "PASS: Full device status list returned\n";
        return true;
    }
    else {
        std::cout << "FAIL: Full device status list should include LIGHT=ON\n";
        return false;
    }
}

// LOG MANAGER TEST

// this test checks log file writing
bool testLogEvent() {
    LogManager lm;

    lm.logEvent("TEST LOG ENTRY");

    std::ifstream inFile("server_log.txt");
    std::string line;
    bool found = false;

    while (std::getline(inFile, line)) {
        if (line == "TEST LOG ENTRY") {
            found = true;
            break;
        }
    }

    if (found) {
        std::cout << "PASS: Log entry written\n";
        return true;
    }
    else {
        std::cout << "FAIL: Log entry should be written\n";
        return false;
    }
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

    std::cerr << "Unknown server test: " << selectedTest << std::endl;
    return 1;
}

// this test checks valid transition from HOME to MAINTENANCE
bool testHomeToMaintenance() {
    StateMachine sm;
    sm.setState(ServerState::HOME);

    bool result = sm.setState(ServerState::MAINTENANCE);

    if (result && sm.getState() == ServerState::MAINTENANCE) {
        std::cout << "PASS: HOME -> MAINTENANCE\n";
        return true;
    }
    else {
        std::cout << "FAIL: HOME -> MAINTENANCE should be valid\n";
        return false;
    }
}

// this test checks valid transition from AWAY to LOCKED
bool testAwayToLocked() {
    StateMachine sm;
    sm.setState(ServerState::HOME);
    sm.setState(ServerState::AWAY);

    bool result = sm.setState(ServerState::LOCKED);

    if (result && sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: AWAY -> LOCKED\n";
        return true;
    }
    else {
        std::cout << "FAIL: AWAY -> LOCKED should be valid\n";
        return false;
    }
}

// this test checks LOGIN packet with invalid format
bool testLoginInvalidFormat() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    Packet packet{ CommandID::LOGIN, "admin1234" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: INVALID LOGIN FORMAT" && !session.isAuthenticated()) {
        std::cout << "PASS: Invalid login format rejected\n";
        return true;
    }
    else {
        std::cout << "FAIL: Invalid login format should be rejected\n";
        return false;
    }
}

// this test checks SET_MODE rejects unknown mode
bool testSetModeUnknownMode() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::SET_MODE, "VACATION" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: UNKNOWN MODE") {
        std::cout << "PASS: Unknown mode rejected\n";
        return true;
    }
    else {
        std::cout << "FAIL: Unknown mode should be rejected\n";
        return false;
    }
}

// this test checks invalid command handling
bool testInvalidCommand() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::INVALID, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: INVALID COMMAND") {
        std::cout << "PASS: Invalid command rejected\n";
        return true;
    }
    else {
        std::cout << "FAIL: Invalid command should be rejected\n";
        return false;
    }
}
    // this test checks valid transition from HOME to LOCKED
    bool testHomeToLocked() {
        StateMachine sm;
        sm.setState(ServerState::HOME);

        bool result = sm.setState(ServerState::LOCKED);

        if (result && sm.getState() == ServerState::LOCKED) {
            std::cout << "PASS: HOME -> LOCKED\n";
            return true;
        }
        else {
            std::cout << "FAIL: HOME -> LOCKED should be valid\n";
            return false;
        }
    }

    // this test checks valid transition from AWAY to HOME
    bool testAwayToHome() {
        StateMachine sm;
        sm.setState(ServerState::HOME);
        sm.setState(ServerState::AWAY);

        bool result = sm.setState(ServerState::HOME);

        if (result && sm.getState() == ServerState::HOME) {
            std::cout << "PASS: AWAY -> HOME\n";
            return true;
        }
        else {
            std::cout << "FAIL: AWAY -> HOME should be valid\n";
            return false;
        }
    }

    // this test checks valid transition from MAINTENANCE to LOCKED
    bool testMaintenanceToLocked() {
        StateMachine sm;
        sm.setState(ServerState::HOME);
        sm.setState(ServerState::MAINTENANCE);

        bool result = sm.setState(ServerState::LOCKED);

        if (result && sm.getState() == ServerState::LOCKED) {
            std::cout << "PASS: MAINTENANCE -> LOCKED\n";
            return true;
        }
        else {
            std::cout << "FAIL: MAINTENANCE -> LOCKED should be valid\n";
            return false;
        }
    }

    // this test checks SET_MODE AWAY works after login
    bool testSetModeAwayWithAuthWorks() {
        StateMachine sm;
        DeviceManager dm;
        LogManager lm;
        ClientSession session;
        RequestHandler handler(sm, dm, lm);

        session.setAuthenticated(true);
        sm.setState(ServerState::HOME);

        Packet packet{ CommandID::SET_MODE, "AWAY" };
        std::string result = handler.handleRequest(packet, session);

        if (result == "SUCCESS" && sm.getState() == ServerState::AWAY) {
            std::cout << "PASS: SET_MODE AWAY works after login\n";
            return true;
        }
        else {
            std::cout << "FAIL: SET_MODE AWAY should work after login\n";
            return false;
        }
    }

    // this test checks turning off device through request handler
    bool testTurnOffDeviceThroughHandler() {
        StateMachine sm;
        DeviceManager dm;
        LogManager lm;
        ClientSession session;
        RequestHandler handler(sm, dm, lm);

        session.setAuthenticated(true);
        sm.setState(ServerState::HOME);
        dm.turnOn("FAN");

        Packet packet{ CommandID::TURN_OFF_DEVICE, "FAN" };
        std::string result = handler.handleRequest(packet, session);

        if (result == "SUCCESS" && dm.getStatus("FAN") == "OFF") {
            std::cout << "PASS: RequestHandler turns FAN OFF\n";
            return true;
        }
        else {
            std::cout << "FAIL: RequestHandler should turn FAN OFF\n";
            return false;
        }
    }

    // this test checks invalid device status through request handler
    bool testGetInvalidDeviceStatusThroughHandler() {
        StateMachine sm;
        DeviceManager dm;
        LogManager lm;
        ClientSession session;
        RequestHandler handler(sm, dm, lm);

        session.setAuthenticated(true);
        sm.setState(ServerState::HOME);

        Packet packet{ CommandID::GET_DEVICE_STATUS, "TV" };
        std::string result = handler.handleRequest(packet, session);

        if (result == "DEVICE NOT FOUND") {
            std::cout << "PASS: RequestHandler handles invalid device status\n";
            return true;
        }
        else {
            std::cout << "FAIL: RequestHandler should return DEVICE NOT FOUND\n";
            return false;
        }
    }
    // this test checks valid transition from MAINTENANCE to HOME
    bool testMaintenanceToHome() {
        StateMachine sm;
        sm.setState(ServerState::HOME);
        sm.setState(ServerState::MAINTENANCE);

        bool result = sm.setState(ServerState::HOME);

        if (result && sm.getState() == ServerState::HOME) {
            std::cout << "PASS: MAINTENANCE -> HOME\n";
            return true;
        }
        else {
            std::cout << "FAIL: MAINTENANCE -> HOME should be valid\n";
            return false;
        }
    }

    // this test checks invalid transition from AWAY to MAINTENANCE
    bool testAwayToMaintenanceInvalid() {
        StateMachine sm;
        sm.setState(ServerState::HOME);
        sm.setState(ServerState::AWAY);

        bool result = sm.setState(ServerState::MAINTENANCE);

        if (!result && sm.getState() == ServerState::AWAY) {
            std::cout << "PASS: AWAY -> MAINTENANCE rejected\n";
            return true;
        }
        else {
            std::cout << "FAIL: AWAY -> MAINTENANCE should be invalid\n";
            return false;
        }
    }

    // this test checks invalid TURN_ON_DEVICE through request handler
    bool testTurnOnInvalidDeviceThroughHandler() {
        StateMachine sm;
        DeviceManager dm;
        LogManager lm;
        ClientSession session;
        RequestHandler handler(sm, dm, lm);

        session.setAuthenticated(true);
        sm.setState(ServerState::HOME);

        Packet packet{ CommandID::TURN_ON_DEVICE, "TV" };
        std::string result = handler.handleRequest(packet, session);

        if (result == "FAILURE") {
            std::cout << "PASS: Invalid TURN_ON_DEVICE rejected\n";
            return true;
        }
        else {
            std::cout << "FAIL: Invalid TURN_ON_DEVICE should return FAILURE\n";
            return false;
        }
    }

    // this test checks invalid TURN_OFF_DEVICE through request handler
bool testTurnOffInvalidDeviceThroughHandler() {
        StateMachine sm;
        DeviceManager dm;
        LogManager lm;
        ClientSession session;
        RequestHandler handler(sm, dm, lm);

        session.setAuthenticated(true);
        sm.setState(ServerState::HOME);

        Packet packet{ CommandID::TURN_OFF_DEVICE, "TV" };
        std::string result = handler.handleRequest(packet, session);

        if (result == "FAILURE") {
            std::cout << "PASS: Invalid TURN_OFF_DEVICE rejected\n";
            return true;
        }
        else {
            std::cout << "FAIL: Invalid TURN_OFF_DEVICE should return FAILURE\n";
            return false;
    }
}

// this test checks AuthManager stores the username on successful login
bool testValidLoginSetsUsername() {
    AuthManager auth;
    ClientSession session;

    const bool result = auth.login("admin", "1234", session);

    if (result && session.isAuthenticated() && session.getUsername() == "admin") {
        std::cout << "PASS: Valid login sets username\n";
        return true;
    }
    else {
        std::cout << "FAIL: Valid login should set username\n";
        return false;
    }
}

// this test checks the numeric-command bridge maps and routes commands correctly
bool testHandleNetworkPacketMappingAndUnknownCommand() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    // Login via numeric ID = 1
    std::string loginResult = handler.handleNetworkPacket(1, "admin,1234", session);

    // Unknown numeric ID should map to INVALID and return INVALID COMMAND when authenticated
    std::string unknownResult = handler.handleNetworkPacket(999, "anything", session);

    if (loginResult == "LOGIN SUCCESS" &&
        session.isAuthenticated() &&
        unknownResult == "ERROR: INVALID COMMAND") {
        std::cout << "PASS: NetworkPacket mapping handles known and unknown command IDs\n";
        return true;
    }
    else {
        std::cout << "FAIL: NetworkPacket mapping did not behave as expected\n";
        return false;
    }
}

// this test checks NetworkPacket serialize/deserialize round-trip in the Test_Server module
bool testNetworkPacketSerializeDeserializeSmoke() {
    NetworkPacket packet(3, "HOME");

    uint32_t size = 0;
    char* buffer = packet.serialize(size);

    NetworkPacket parsed;
    const bool ok = parsed.deserialize(buffer, size);

    delete[] buffer;

    const std::string parsedPayload = (parsed.getPayload() && parsed.getPayloadLength() > 0)
        ? std::string(parsed.getPayload(), parsed.getPayload() + parsed.getPayloadLength())
        : std::string();

    if (ok &&
        parsed.getCommandId() == packet.getCommandId() &&
        parsedPayload == "HOME") {
        std::cout << "PASS: NetworkPacket serialize/deserialize smoke\n";
        return true;
    }
    else {
        std::cout << "FAIL: NetworkPacket serialize/deserialize smoke failed\n";
        return false;
    }
}

// this test checks NetworkPacket rejects a bad checksum and restores original state
bool testNetworkPacketDeserializeRejectsInvalidChecksumAndRestores() {
    NetworkPacket baseline(2, "LOCKED");
    const uint16_t originalCmd = baseline.getCommandId();
    const uint32_t originalLen = baseline.getPayloadLength();
    const uint16_t originalChecksum = baseline.getChecksum();

    NetworkPacket good(2, "HOME");
    uint32_t size = 0;
    char* buffer = good.serialize(size);

    // Corrupt one payload byte (this should invalidate checksum).
    if (size > 12) {
        buffer[10] = static_cast<char>(buffer[10] ^ 0xFF);
    }

    const bool ok = baseline.deserialize(buffer, size);
    delete[] buffer;

    if (!ok &&
        baseline.getCommandId() == originalCmd &&
        baseline.getPayloadLength() == originalLen &&
        baseline.getChecksum() == originalChecksum) {
        std::cout << "PASS: NetworkPacket rejects invalid checksum and restores\n";
        return true;
    }
    else {
        std::cout << "FAIL: NetworkPacket should reject invalid checksum and restore\n";
        return false;
    }
}

// this test checks NetworkManager::sendAll fails cleanly on an invalid socket
bool testNetworkManagerSendAllFailsOnInvalidSocket() {
    const char data[2] = { 'x', 'y' };
    const bool ok = NetworkManager::sendAll(INVALID_SOCKET, data, 2);

    if (!ok) {
        std::cout << "PASS: sendAll fails on invalid socket\n";
        return true;
    }
    else {
        std::cout << "FAIL: sendAll should fail on invalid socket\n";
        return false;
    }
}

// this test checks NetworkManager::recvAll fails cleanly on an invalid socket
bool testNetworkManagerRecvAllFailsOnInvalidSocket() {
    char buffer[4] = {};
    const bool ok = NetworkManager::recvAll(INVALID_SOCKET, buffer, 4);

    if (!ok) {
        std::cout << "PASS: recvAll fails on invalid socket\n";
        return true;
    }
    else {
        std::cout << "FAIL: recvAll should fail on invalid socket\n";
        return false;
    }
}

// this test checks NetworkManager can send and receive a NetworkPacket over loopback sockets
bool testNetworkManagerSendReceivePacketLoopback() {
    LoopbackServer server;

    SOCKET clientSocket = INVALID_SOCKET;
    if (!connectSocketToServer(clientSocket, server.port()))
    {
        std::cout << "FAIL: Could not connect to loopback server\n";
        return false;
    }

    SOCKET serverSocket = server.socketHandle();
    if (serverSocket == INVALID_SOCKET)
    {
        closesocket(clientSocket);
        std::cout << "FAIL: Server did not accept connection\n";
        return false;
    }

    NetworkPacket outgoing(4, "LIGHT");
    const bool sent = NetworkManager::sendPacket(clientSocket, outgoing);

    NetworkPacket incoming;
    const bool received = NetworkManager::receivePacket(serverSocket, incoming);

    closesocket(clientSocket);

    const std::string payload = (incoming.getPayload() && incoming.getPayloadLength() > 0)
        ? std::string(incoming.getPayload(), incoming.getPayload() + incoming.getPayloadLength())
        : std::string();

    if (sent && received && incoming.getCommandId() == outgoing.getCommandId() && payload == "LIGHT") {
        std::cout << "PASS: NetworkManager send/receive packet loopback\n";
        return true;
    }
    else {
        std::cout << "FAIL: NetworkManager send/receive packet loopback failed\n";
        return false;
    }
}

// this test checks NetworkManager::sendPacket fails cleanly on an invalid socket
bool testNetworkManagerSendPacketFailsOnInvalidSocket() {
    NetworkPacket packet(4, "LIGHT");
    const bool ok = NetworkManager::sendPacket(INVALID_SOCKET, packet);

    if (!ok) {
        std::cout << "PASS: sendPacket fails on invalid socket\n";
        return true;
    }
    else {
        std::cout << "FAIL: sendPacket should fail on invalid socket\n";
        return false;
    }
}

// this test checks NetworkManager::receivePacket fails cleanly on an invalid socket
bool testNetworkManagerReceivePacketFailsOnInvalidSocket() {
    NetworkPacket packet;
    const bool ok = NetworkManager::receivePacket(INVALID_SOCKET, packet);

    if (!ok) {
        std::cout << "PASS: receivePacket fails on invalid socket\n";
        return true;
    }
    else {
        std::cout << "FAIL: receivePacket should fail on invalid socket\n";
        return false;
    }
}

// this test checks DeviceManager rejects turning ON an unknown device
bool testDeviceManagerTurnOnInvalidFails() {
    DeviceManager dm;
    const bool ok = dm.turnOn("TV");

    if (!ok) {
        std::cout << "PASS: DeviceManager rejects turning ON an unknown device\n";
        return true;
    }
    else {
        std::cout << "FAIL: DeviceManager should reject turning ON an unknown device\n";
        return false;
    }
}

// this test checks DeviceManager rejects turning OFF an unknown device
bool testDeviceManagerTurnOffInvalidFails() {
    DeviceManager dm;
    const bool ok = dm.turnOff("TV");

    if (!ok) {
        std::cout << "PASS: DeviceManager rejects turning OFF an unknown device\n";
        return true;
    }
    else {
        std::cout << "FAIL: DeviceManager should reject turning OFF an unknown device\n";
        return false;
    }
}

// these tests cover each device-command rejection branch in LOCKED
bool testDeviceCommandTurnOffRejectedInLocked() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::TURN_OFF_DEVICE, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: TURN_OFF rejected in LOCKED\n";
        return true;
    }
    else {
        std::cout << "FAIL: TURN_OFF should be rejected in LOCKED\n";
        return false;
    }
}

bool testDeviceCommandGetDeviceStatusRejectedInLocked() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::GET_DEVICE_STATUS, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: GET_DEVICE_STATUS rejected in LOCKED\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_DEVICE_STATUS should be rejected in LOCKED\n";
        return false;
    }
}

bool testDeviceCommandGetAllStatusRejectedInLocked() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::GET_ALL_DEVICE_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: GET_ALL_DEVICE_STATUS rejected in LOCKED\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_ALL_DEVICE_STATUS should be rejected in LOCKED\n";
        return false;
    }
}

// these tests cover each device-command rejection branch in MAINTENANCE
bool testDeviceCommandTurnOffRejectedInMaintenance() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);
    sm.setState(ServerState::MAINTENANCE);

    Packet packet{ CommandID::TURN_OFF_DEVICE, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: TURN_OFF rejected in MAINTENANCE\n";
        return true;
    }
    else {
        std::cout << "FAIL: TURN_OFF should be rejected in MAINTENANCE\n";
        return false;
    }
}

bool testDeviceCommandGetDeviceStatusRejectedInMaintenance() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);
    sm.setState(ServerState::MAINTENANCE);

    Packet packet{ CommandID::GET_DEVICE_STATUS, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: GET_DEVICE_STATUS rejected in MAINTENANCE\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_DEVICE_STATUS should be rejected in MAINTENANCE\n";
        return false;
    }
}

bool testDeviceCommandGetAllStatusRejectedInMaintenance() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    session.setAuthenticated(true);
    sm.setState(ServerState::HOME);
    sm.setState(ServerState::MAINTENANCE);

    Packet packet{ CommandID::GET_ALL_DEVICE_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE") {
        std::cout << "PASS: GET_ALL_DEVICE_STATUS rejected in MAINTENANCE\n";
        return true;
    }
    else {
        std::cout << "FAIL: GET_ALL_DEVICE_STATUS should be rejected in MAINTENANCE\n";
        return false;
    }
}

// MAIN

int main(int argc, char** argv)
{
    WinsockSession winsock;

    const TestCase tests[] = {
        {"initial_state", testInitialState},
        {"locked_to_home", testLockedToHome},
        {"locked_to_away_invalid", testLockedToAwayInvalid},
        {"home_to_away", testHomeToAway},
        {"get_status_initially_locked", testGetStatusInitiallyLocked},
        {"get_status_home", testGetStatusHome},
        {"get_status_away", testGetStatusAway},
        {"get_status_maintenance", testGetStatusMaintenance},
        {"set_mode_without_auth_fails", testSetModeWithoutAuthFails},
        {"set_mode_with_auth_works", testSetModeWithAuthWorks},
        {"set_mode_maintenance_with_auth_works", testSetModeMaintenanceWithAuthWorks},
        {"set_mode_locked_with_auth_works", testSetModeLockedWithAuthWorks},
        {"set_mode_invalid_transition_rejected", testSetModeInvalidTransitionReturnsError},
        {"valid_login", testValidLogin},
        {"valid_login_sets_username", testValidLoginSetsUsername},
        {"invalid_login", testInvalidLogin},
        {"turn_on_device", testTurnOnDevice},
        {"turn_off_device", testTurnOffDevice},
        {"invalid_device", testInvalidDevice},
        {"turn_on_device_through_handler", testTurnOnDeviceThroughHandler},
        {"get_device_status_through_handler", testGetDeviceStatusThroughHandler},
        {"device_command_rejected_in_locked", testDeviceCommandRejectedInLocked},
        {"device_command_rejected_in_maintenance", testDeviceCommandRejectedInMaintenance},
        {"device_command_works_in_home", testDeviceCommandWorksInHome},
        {"get_all_device_status", testGetAllDeviceStatus},
        {"log_event", testLogEvent},
        {"home_to_maintenance", testHomeToMaintenance},
        {"away_to_locked", testAwayToLocked},
        {"login_invalid_format", testLoginInvalidFormat},
        {"set_mode_unknown_mode", testSetModeUnknownMode},
        {"invalid_command", testInvalidCommand},
        {"handle_network_packet_mapping_and_unknown_command", testHandleNetworkPacketMappingAndUnknownCommand},
        {"network_packet_serialize_deserialize_smoke", testNetworkPacketSerializeDeserializeSmoke},
        {"network_packet_deserialize_rejects_invalid_checksum_and_restores", testNetworkPacketDeserializeRejectsInvalidChecksumAndRestores},
        {"network_manager_send_all_fails_on_invalid_socket", testNetworkManagerSendAllFailsOnInvalidSocket},
        {"network_manager_recv_all_fails_on_invalid_socket", testNetworkManagerRecvAllFailsOnInvalidSocket},
        {"network_manager_send_receive_packet_loopback", testNetworkManagerSendReceivePacketLoopback},
        {"network_manager_send_packet_fails_on_invalid_socket", testNetworkManagerSendPacketFailsOnInvalidSocket},
        {"network_manager_receive_packet_fails_on_invalid_socket", testNetworkManagerReceivePacketFailsOnInvalidSocket},
        {"device_manager_turn_on_invalid_fails", testDeviceManagerTurnOnInvalidFails},
        {"device_manager_turn_off_invalid_fails", testDeviceManagerTurnOffInvalidFails},
        {"device_command_turn_off_rejected_in_locked", testDeviceCommandTurnOffRejectedInLocked},
        {"device_command_get_device_status_rejected_in_locked", testDeviceCommandGetDeviceStatusRejectedInLocked},
        {"device_command_get_all_status_rejected_in_locked", testDeviceCommandGetAllStatusRejectedInLocked},
        {"device_command_turn_off_rejected_in_maintenance", testDeviceCommandTurnOffRejectedInMaintenance},
        {"device_command_get_device_status_rejected_in_maintenance", testDeviceCommandGetDeviceStatusRejectedInMaintenance},
        {"device_command_get_all_status_rejected_in_maintenance", testDeviceCommandGetAllStatusRejectedInMaintenance},
        {"home_to_locked", testHomeToLocked},
        {"away_to_home", testAwayToHome},
        {"maintenance_to_locked", testMaintenanceToLocked},
        {"set_mode_away_with_auth_works", testSetModeAwayWithAuthWorks},
        {"turn_off_device_through_handler", testTurnOffDeviceThroughHandler},
        {"get_invalid_device_status_through_handler", testGetInvalidDeviceStatusThroughHandler},
        {"maintenance_to_home", testMaintenanceToHome},
        {"away_to_maintenance_invalid", testAwayToMaintenanceInvalid},
        {"turn_on_invalid_device_through_handler", testTurnOnInvalidDeviceThroughHandler},
        {"turn_off_invalid_device_through_handler", testTurnOffInvalidDeviceThroughHandler},
    };

    return runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));
}
