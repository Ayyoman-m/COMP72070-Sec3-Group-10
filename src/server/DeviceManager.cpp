#include "Header/DeviceManager.h"

/**
 * @brief Constructs the DeviceManager and initializes default devices.
 *
 * Adds predefined devices with initial OFF state.
 */
DeviceManager::DeviceManager() {
    devices["LIGHT"] = false;
    devices["FAN"] = false;
    devices["AC"] = false;
    devices["DOOR"] = false;
}

/**
 * @brief Turns a device ON.
 *
 * @param device Device name
 * @return true if device exists and was updated, false otherwise
 */
bool DeviceManager::turnOn(const std::string& device) {
    if (devices.find(device) != devices.end()) {
        devices[device] = true;
        return true;
    }
    return false;
}

/**
 * @brief Turns a device OFF.
 *
 * @param device Device name
 * @return true if device exists and was updated, false otherwise
 */
bool DeviceManager::turnOff(const std::string& device) {
    if (devices.find(device) != devices.end()) {
        devices[device] = false;
        return true;
    }
    return false;
}

/**
 * @brief Gets the status of a specific device.
 *
 * @param device Device name
 * @return "ON", "OFF", or "DEVICE NOT FOUND"
 */
std::string DeviceManager::getStatus(const std::string& device) {
    if (devices.find(device) != devices.end()) {
        return devices[device] ? "ON" : "OFF";
    }
    return "DEVICE NOT FOUND";
}

/**
 * @brief Gets the status of all devices.
 *
 * @return Combined status string for all devices
 */
std::string DeviceManager::getAllStatus() {
    std::string result = "";

    for (const auto& item : devices) {
        result += item.first + "=" + (item.second ? "ON" : "OFF") + "; ";
    }

    return result;
}