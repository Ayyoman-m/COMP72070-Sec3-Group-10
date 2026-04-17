#pragma once

#include "ServerState.h"

/**
 * @class StateMachine
 * @brief Manages system state and transitions.
 *
 * Controls the current state of the smart home system and
 * ensures only valid transitions between states are allowed.
 */
class StateMachine {
private:
    ServerState currentState; ///< Current system state

public:

    /**
     * @brief Constructs the StateMachine.
     *
     * Initializes the system in a default state.
     */
    StateMachine();

    /**
     * @brief Returns the current system state.
     * @return Current ServerState
     */
    ServerState getState() const;

    /**
     * @brief Validates if a state transition is allowed.
     *
     * @param newState Desired new state
     * @return true if transition is valid, false otherwise
     */
    bool validateTransition(ServerState newState) const;

    /**
     * @brief Updates the system state.
     *
     * Applies the new state if the transition is valid.
     *
     * @param newState Desired new state
     * @return true if state was updated successfully, false otherwise
     */
    bool setState(ServerState newState);
};