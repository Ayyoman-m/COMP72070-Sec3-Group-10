#pragma once
#include <string>
#include "ClientSession.h"

//handles login functionality
class AuthManager {
public:
    //checks username and password and updates session
    bool login(const std::string& username, const std::string& password, ClientSession& session);
};