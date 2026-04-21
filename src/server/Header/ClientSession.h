#pragma once
#include <string>

/**
 * @class ClientSession
 * @brief Represents a client's session state.
 *
 * Stores authentication status and associated username
 * for a connected client.
 */
class ClientSession {
private:
    bool authenticated;       ///< Indicates if the user is authenticated
    std::string username;     ///< Username of the connected client

public:

    /**
     * @brief Constructs a default session.
     *
     * Initializes session as unauthenticated.
     */
    ClientSession() : authenticated(false), username("") {}

    /**
     * @brief Checks if the client is authenticated.
     * @return true if authenticated, false otherwise
     */
    bool isAuthenticated() const {
        return authenticated;
    }

    /**
     * @brief Sets authentication status.
     * @param value Authentication state
     */
    void setAuthenticated(bool value) {
        authenticated = value;
    }

    /**
     * @brief Gets the username of the session.
     * @return Username string
     */
    std::string getUsername() const {
        return username;
    }

    /**
     * @brief Sets the username for the session.
     * @param name Username
     */
    void setUsername(const std::string& name) {
        username = name;
    }
};