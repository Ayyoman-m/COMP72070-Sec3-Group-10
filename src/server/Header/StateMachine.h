#pragma once

#include "ServerState.h"

class StateMachine {
private:
    ServerState currentState;

public:
    StateMachine();
    ServerState getState() const;
    bool validateTransition(ServerState newState) const;
    bool setState(ServerState newState);
};