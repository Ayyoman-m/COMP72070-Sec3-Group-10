#include "Header/StateMachine.h"

/**
 * @brief Constructs the StateMachine.
 *
 * Initializes system state to LOCKED.
 */
StateMachine::StateMachine() : currentState(ServerState::LOCKED) {}

/**
 * @brief Returns current system state.
 *
 * @return Current ServerState
 */
ServerState StateMachine::getState() const {
    return currentState;
}

/**
 * @brief Validates if a state transition is allowed.
 *
 * Defines valid transitions between system states.
 *
 * @param newState Desired new state
 * @return true if transition is valid, false otherwise
 */
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

/**
 * @brief Updates system state if transition is valid.
 *
 * @param newState Desired new state
 * @return true if state updated successfully, false otherwise
 */
bool StateMachine::setState(ServerState newState) {
    if (validateTransition(newState)) {
        currentState = newState;
        return true;
    }

    return false;
}