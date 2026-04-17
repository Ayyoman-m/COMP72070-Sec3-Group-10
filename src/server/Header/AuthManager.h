#pragma once
#include <string>
#include "ClientSession.h"

/**
 * @class AuthManager
 * @brief Handles user authentication for the system.
 *
 * Validates user credentials and updates the client session
 * upon successful login.
 */
class AuthManager {
public:

    /**
     * @brief Authenticates a user using username and password.
     *
     * @param username User's username
     * @param password User's password
     * @param session Client session to update upon successful login
     * @return true if authentication is successful, false otherwise
     */
    bool login(const std::string& username, const std::string& password, ClientSession& session);
};