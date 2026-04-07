#include "Header/StateMachine.h"

StateMachine::StateMachine() : currentState(ServerState::LOCKED) {}

ServerState StateMachine::getState() const {
    return currentState;
}

bool StateMachine::validateTransition(ServerState newState) const {
    if (currentState == ServerState::LOCKED && newState == ServerState::HOME) {
        return true;
    }

    if (currentState == ServerState::HOME &&
        (newState == ServerState::AWAY ||
            newState == ServerState::MAINTENANCE ||
            newState == ServerState::LOCKED)) {
        return true;
    }

    if (currentState == ServerState::AWAY &&
        (newState == ServerState::HOME ||
            newState == ServerState::LOCKED)) {
        return true;
    }

    if (currentState == ServerState::MAINTENANCE &&
        (newState == ServerState::HOME ||
            newState == ServerState::LOCKED)) {
        return true;
    }

    return false;
}

bool StateMachine::setState(ServerState newState) {
    if (validateTransition(newState)) {
        currentState = newState;
        return true;
    }

    return false;
}