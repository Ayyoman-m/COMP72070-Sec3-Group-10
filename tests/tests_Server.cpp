#include <iostream>
#include "C:\Users\RASIK\OneDrive\Desktop\Sem 4\Mobile and Network Environment\src\server\Header/StateMachine.h"

void test_initial_state() {
    StateMachine sm;

    if (sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: Initial state is LOCKED\n";
    }
    else {
        std::cout << "FAIL: Initial state is not LOCKED\n";
    }
}

void test_locked_to_home() {
    StateMachine sm;

    bool result = sm.setState(ServerState::HOME);

    if (result && sm.getState() == ServerState::HOME) {
        std::cout << "PASS: LOCKED -> HOME\n";
    }
    else {
        std::cout << "FAIL: LOCKED -> HOME\n";
    }
}

void test_locked_to_away_invalid() {
    StateMachine sm;

    bool result = sm.setState(ServerState::AWAY);

    if (!result && sm.getState() == ServerState::LOCKED) {
        std::cout << "PASS: LOCKED -> AWAY rejected\n";
    }
    else {
        std::cout << "FAIL: LOCKED -> AWAY should be rejected\n";
    }
}

int main() {
    test_initial_state();
    test_locked_to_home();
    test_locked_to_away_invalid();
    return 0;
}