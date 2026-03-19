#pragma once
#include <string>

class ClientSession {
private:
    bool authenticated;
    std::string username;

public:
    ClientSession() : authenticated(false), username("") {}

    bool isAuthenticated() const {
        return authenticated;
    }

    void setAuthenticated(bool value) {
        authenticated = value;
    }

    std::string getUsername() const {
        return username;
    }

    void setUsername(const std::string& name) {
        username = name;
    }
};