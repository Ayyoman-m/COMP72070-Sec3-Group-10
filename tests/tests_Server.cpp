#include <iostream>
#include <string>
#include<fstream>
#include "../server/Header/StateMachine.h"
#include "../server/Header/RequestHandler.h"
#include "../server/Header/AuthManager.h"
#include "../server/Header/DeviceManager.h"
#include "../server/Header/LogManager.h"

namespace
{
    struct TestCase
    {
        const char* name;
        bool (*function)();
    };
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

// MAIN

int main(int argc, char** argv) {
    const TestCase tests[] = {
        {"initial_state", testInitialState},
        {"locked_to_home", testLockedToHome},
        {"locked_to_away_invalid", testLockedToAwayInvalid},
        {"home_to_away", testHomeToAway},
        {"get_status_initially_locked", testGetStatusInitiallyLocked},
        {"set_mode_without_auth_fails", testSetModeWithoutAuthFails},
        {"set_mode_with_auth_works", testSetModeWithAuthWorks},
        {"valid_login", testValidLogin},
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
    };

    return runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));
}
