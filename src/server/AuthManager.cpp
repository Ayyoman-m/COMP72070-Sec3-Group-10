#include "Header/AuthManager.h"

/**
 * @brief Validates user credentials and updates session.
 *
 * Compares provided username and password against stored values.
 * If valid, the session is marked as authenticated.
 *
 * @param username User's username
 * @param password User's password
 * @param session Client session to update
 * @return true if login is successful, false otherwise
 */
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