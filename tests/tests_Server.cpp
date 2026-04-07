#include <iostream>
#include<fstream>
#include "../server/Header/StateMachine.h"
#include "../server/Header/RequestHandler.h"
#include "../server/Header/AuthManager.h"
#include "../server/Header/DeviceManager.h"
#include "../server/Header/LogManager.h"


// STATE MACHINE TESTS

// this test checks if the server starts in LOCKED state
void testInitialState() {
    StateMachine sm;

    if (sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: Initial state is LOCKED\n";
    }
    else {
        std::cout << "FAIL: Initial state should be LOCKED\n";
    }
}

// this test checks valid transition from LOCKED to HOME
void testLockedToHome() {
    StateMachine sm;

    bool result = sm.setState(ServerState::HOME);

    if (result && sm.getState() == ServerState::HOME) {
        std::cout << "PASS: LOCKED -> HOME\n";
    }
    else {
        std::cout << "FAIL: LOCKED -> HOME should be valid\n";
    }
}

// this test checks invalid transition from LOCKED to AWAY
void testLockedToAwayInvalid() {
    StateMachine sm;

    bool result = sm.setState(ServerState::AWAY);

    if (!result && sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: LOCKED -> AWAY rejected\n";
    }
    else {
        std::cout << "FAIL: LOCKED -> AWAY should be invalid\n";
    }
}

// this test checks valid transition from HOME to AWAY
void testHomeToAway() {
    StateMachine sm;
    sm.setState(ServerState::HOME);

    bool result = sm.setState(ServerState::AWAY);

    if (result && sm.getState() == ServerState::AWAY) {
        std::cout << "PASS: HOME -> AWAY\n";
    }
    else {
        std::cout << "FAIL: HOME -> AWAY should be valid\n";
    }
}

// REQUEST HANDLER TESTS

// this test checks GET_STATUS returns LOCKED at start
void testGetStatusInitiallyLocked() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    Packet packet{ CommandID::GET_STATUS, "" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "LOCKED") {
        std::cout << "PASS: GET_STATUS returns LOCKED\n";
    }
    else {
        std::cout << "FAIL: GET_STATUS should return LOCKED\n";
    }
}

// this test checks SET_MODE fails without login
void testSetModeWithoutAuthFails() {
    StateMachine sm;
    DeviceManager dm;
    LogManager lm;
    ClientSession session;
    RequestHandler handler(sm, dm, lm);

    Packet packet{ CommandID::SET_MODE, "HOME" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: NOT AUTHENTICATED") {
        std::cout << "PASS: SET_MODE blocked without auth\n";
    }
    else {
        std::cout << "FAIL: SET_MODE should fail without auth\n";
    }
}

// this test checks SET_MODE works after login
void testSetModeWithAuthWorks() {
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
    }
    else {
        std::cout << "FAIL: SET_MODE HOME should work after login\n";
    }
}

// AUTH MANAGER TESTS

// this test checks valid login
void testValidLogin() {
    AuthManager auth;
    ClientSession session;

    bool result = auth.login("admin", "1234", session);

    if (result && session.isAuthenticated()) {
        std::cout << "PASS: Valid login works\n";
    }
    else {
        std::cout << "FAIL: Valid login should succeed\n";
    }
}

// this test checks invalid login
void testInvalidLogin() {
    AuthManager auth;
    ClientSession session;

    bool result = auth.login("admin", "wrong", session);

    if (!result && !session.isAuthenticated()) {
        std::cout << "PASS: Invalid login rejected\n";
    }
    else {
        std::cout << "FAIL: Invalid login should fail\n";
    }
}

// DEVICE MANAGER TESTS

// this test checks device turns ON
void testTurnOnDevice() {
    DeviceManager dm;

    bool result = dm.turnOn("LIGHT");

    if (result && dm.getStatus("LIGHT") == "ON") {
        std::cout << "PASS: LIGHT turned ON\n";
    }
    else {
        std::cout << "FAIL: LIGHT should turn ON\n";
    }
}

// this test checks device turns OFF
void testTurnOffDevice() {
    DeviceManager dm;

    dm.turnOn("FAN");
    bool result = dm.turnOff("FAN");

    if (result && dm.getStatus("FAN") == "OFF") {
        std::cout << "PASS: FAN turned OFF\n";
    }
    else {
        std::cout << "FAIL: FAN should turn OFF\n";
    }
}

// this test checks invalid device handling
void testInvalidDevice() {
    DeviceManager dm;

    std::string result = dm.getStatus("TV");

    if (result == "DEVICE NOT FOUND") {
        std::cout << "PASS: Invalid device handled\n";
    }
    else {
        std::cout << "FAIL: Invalid device should return error\n";
    }
}

// DEVICE INTEGRATION TESTS

// this test checks turning on device through request handler
void testTurnOnDeviceThroughHandler() {
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
    }
    else {
        std::cout << "FAIL: RequestHandler should turn LIGHT ON\n";
    }
}

// this test checks getting device status through request handler
void testGetDeviceStatusThroughHandler() {
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
    }
    else {
        std::cout << "FAIL: RequestHandler should return device status\n";
    }
}

// this test checks device command is rejected in LOCKED
void testDeviceCommandRejectedInLocked() {
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
    }
    else {
        std::cout << "FAIL: Device command should be rejected in LOCKED\n";
    }
}

// this test checks device command is rejected in MAINTENANCE
void testDeviceCommandRejectedInMaintenance() {
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
    }
    else {
        std::cout << "FAIL: Device command should be rejected in MAINTENANCE\n";
    }
}

// this test checks device command works in HOME
void testDeviceCommandWorksInHome() {
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
    }
    else {
        std::cout << "FAIL: Device command should work in HOME\n";
    }
}

// this test checks full device status list
void testGetAllDeviceStatus() {
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
    }
    else {
        std::cout << "FAIL: Full device status list should include LIGHT=ON\n";
    }
}

// LOG MANAGER TEST

// this test checks log file writing
void testLogEvent() {
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
    }
    else {
        std::cout << "FAIL: Log entry should be written\n";
    }
}

// MAIN

int main() {
    // running all tests one by one

    std::cout << "---- Running StateMachine Tests ----\n";
    testInitialState();
    testLockedToHome();
    testLockedToAwayInvalid();
    testHomeToAway();

    std::cout << "\n---- Running RequestHandler Tests ----\n";
    testGetStatusInitiallyLocked();
    testSetModeWithoutAuthFails();
    testSetModeWithAuthWorks();

    std::cout << "\n---- Running AuthManager Tests ----\n";
    testValidLogin();
    testInvalidLogin();

    std::cout << "\n---- Running DeviceManager Tests ----\n";
    testTurnOnDevice();
    testTurnOffDevice();
    testInvalidDevice();

    std::cout << "\n---- Running Device Integration Tests ----\n";
    testTurnOnDeviceThroughHandler();
    testGetDeviceStatusThroughHandler();
    testDeviceCommandRejectedInLocked();
    testDeviceCommandRejectedInMaintenance();
    testDeviceCommandWorksInHome();
    testGetAllDeviceStatus();

    std::cout << "\n---- Running LogManager Tests ----\n";
    testLogEvent();

    return 0;
}