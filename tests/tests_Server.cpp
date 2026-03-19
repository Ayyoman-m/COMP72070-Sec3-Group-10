#include <iostream>
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/StateMachine.h"
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/RequestHandler.h"
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/AuthManager.h"

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
    ClientSession session;
    RequestHandler handler(sm);

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
    ClientSession session;
    RequestHandler handler(sm);

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
    ClientSession session;
    RequestHandler handler(sm);

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

    return 0;
}