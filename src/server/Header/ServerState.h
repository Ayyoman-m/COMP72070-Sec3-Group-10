#pragma once

/**
 * @enum ServerState
 * @brief Represents the operational state of the smart home system.
 *
 * Used by the StateMachine to control system behavior
 * and enforce valid transitions between modes.
 */
enum class ServerState {
    LOCKED,        ///< System is locked (no access)
    HOME,          ///< User is home
    AWAY,          ///< User is away
    MAINTENANCE    ///< System is under maintenance
};