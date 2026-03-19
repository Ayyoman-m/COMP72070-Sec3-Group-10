#include "Header/DeviceManager.h"

// constructor - initializing some default devices
DeviceManager::DeviceManager() {
    devices["LIGHT"] = false;
    devices["FAN"] = false;
    devices["AC"] = false;
}

// turning ON device
bool DeviceManager::turnOn(const std::string& device) {

    // checking if device exists
    if (devices.find(device) != devices.end()) {
        devices[device] = true;
        return true;
    }

    return false;
}

// turning OFF device
bool DeviceManager::turnOff(const std::string& device) {

    // checking if device exists
    if (devices.find(device) != devices.end()) {
        devices[device] = false;
        return true;
    }

    return false;
}

// getting device status
std::string DeviceManager::getStatus(const std::string& device) {

    if (devices.find(device) != devices.end()) {
        return devices[device] ? "ON" : "OFF";
    }

    return "DEVICE NOT FOUND";
}