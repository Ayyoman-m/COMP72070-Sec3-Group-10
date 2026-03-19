#include <iostream>
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/StateMachine.h"
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/RequestHandler.h"
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/AuthManager.h"
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/DeviceManager.h"

// STATE MACHINE TESTS

// This test checks if the server starts in LOCKED mode
void testInitialState() {
    StateMachine sm;

    if (sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: Initial state is LOCKED\n";
    }
    else {
        std::cout << "FAIL: Initial state should be LOCKED\n";
    }
}

// This test checks a valid transition from LOCKED -> HOME
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

// This test checks that invalid transition is rejected
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

// This test checks another valid transition HOME -> AWAY
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

// This test checks if GET_STATUS returns LOCKED initially
void testGetStatusInitiallyLocked() {
    StateMachine sm;
    DeviceManager dm;
    ClientSession session;
    RequestHandler handler(sm,dm);

    Packet packet{ CommandID::GET_STATUS, "" };

    std::string result = handler.handleRequest(packet, session);

    if (result == "LOCKED") {
        std::cout << "PASS: GET_STATUS returns LOCKED\n";
    }
    else {
        std::cout << "FAIL: GET_STATUS should return LOCKED\n";
    }
}

// This test checks that SET_MODE fails if user is not logged in
void testSetModeWithoutAuthFails() {
    StateMachine sm;
    DeviceManager dm;
    ClientSession session;
    RequestHandler handler(sm,dm);

    Packet packet{ CommandID::SET_MODE, "HOME" };

    std::string result = handler.handleRequest(packet, session);

    if (result == "ERROR: NOT AUTHENTICATED") {
        std::cout << "PASS: SET_MODE blocked without auth\n";
    }
    else {
        std::cout << "FAIL: SET_MODE should fail without auth\n";
    }
}

// This test checks SET_MODE works after login
void testSetModeWithAuthWorks() {
    StateMachine sm;
    DeviceManager dm;
    ClientSession session;
    RequestHandler handler(sm,dm);

    // manually setting authentication (simulating login success)
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

// test for valid login
void testValidLogin() {
    AuthManager auth;
    ClientSession session;

    bool result = auth.login("admin", "1234", session);

    // checking if login worked
    if (result && session.isAuthenticated()) {
        std::cout << "PASS: Valid login works\n";
    }
    else {
        std::cout << "FAIL: Valid login should succeed\n";
    }
}

// test for invalid login
void testInvalidLogin() {
    AuthManager auth;
    ClientSession session;

    bool result = auth.login("admin", "wrong",session);

    // checking if login fails correctly
    if (!result && !session.isAuthenticated()) {
        std::cout << "PASS: Invalid login rejected\n";
    }
    else {
        std::cout << "FAIL: Invalid login should fail\n";
    }
}

// test turning device ON
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

// test turning device OFF
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

// test invalid device
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

// test turning on device through request handler
void testTurnOnDeviceThroughHandler() {
    StateMachine sm;
    DeviceManager dm;
    ClientSession session;
    RequestHandler handler(sm, dm);

    session.setAuthenticated(true);

    Packet packet{ CommandID::TURN_ON_DEVICE, "LIGHT" };
    std::string result = handler.handleRequest(packet, session);

    if (result == "DEVICE TURNED ON" && dm.getStatus("LIGHT") == "ON") {
        std::cout << "PASS: RequestHandler turns LIGHT ON\n";
    }
    else {
        std::cout << "FAIL: RequestHandler should turn LIGHT ON\n";
    }
}

// test getting device status through request handler
void testGetDeviceStatusThroughHandler() {
    StateMachine sm;
    DeviceManager dm;
    ClientSession session;
    RequestHandler handler(sm, dm);

    session.setAuthenticated(true);
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


    return 0;
}