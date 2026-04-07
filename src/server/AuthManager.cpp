#include "Header/AuthManager.h"

// this function checks login credentials
bool AuthManager::login(const std::string& username, const std::string& password, ClientSession& session) {

    // using hardcoded username and password
    if (username == "admin" && password == "1234") {

        // setting session as logged in
        session.setAuthenticated(true);
        session.setUsername(username);

        return true;
    }

    // login failed
    return false;
}