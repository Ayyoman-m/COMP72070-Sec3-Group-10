#include "Header/DeviceManager.h"

// constructor - adding default devices
DeviceManager::DeviceManager() {
    devices["LIGHT"] = false;
    devices["FAN"] = false;
    devices["AC"] = false;
    devices["DOOR"] = false;
}

// turning ON a device
bool DeviceManager::turnOn(const std::string& device) {
    if (devices.find(device) != devices.end()) {
        devices[device] = true;
        return true;
    }

    return false;
}

// turning OFF a device
bool DeviceManager::turnOff(const std::string& device) {
    if (devices.find(device) != devices.end()) {
        devices[device] = false;
        return true;
    }

    return false;
}

// getting one device status
std::string DeviceManager::getStatus(const std::string& device) {
    if (devices.find(device) != devices.end()) {
        return devices[device] ? "ON" : "OFF";
    }

    return "DEVICE NOT FOUND";
}

// getting all device status together
std::string DeviceManager::getAllStatus() {
    std::string result = "";

    for (const auto& item : devices) {
        result += item.first + "=" + (item.second ? "ON" : "OFF") + "; ";
    }

    return result;
}