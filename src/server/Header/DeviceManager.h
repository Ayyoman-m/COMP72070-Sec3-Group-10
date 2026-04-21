#pragma once
#include <string>
#include <unordered_map>

/**
 * @class DeviceManager
 * @brief Manages smart home device states.
 *
 * Stores and controls the ON/OFF status of devices and
 * provides methods to retrieve individual or overall status.
 */
class DeviceManager {
private:
    /// Stores device names and their ON/OFF state
    std::unordered_map<std::string, bool> devices;

public:

    /**
     * @brief Constructs the DeviceManager.
     */
    DeviceManager();

    /**
     * @brief Turns a device ON.
     * @param device Device name
     * @return true if operation is successful
     */
    bool turnOn(const std::string& device);

    /**
     * @brief Turns a device OFF.
     * @param device Device name
     * @return true if operation is successful
     */
    bool turnOff(const std::string& device);

    /**
     * @brief Gets the status of a specific device.
     * @param device Device name
     * @return Device status as string
     */
    std::string getStatus(const std::string& device);

    /**
     * @brief Gets the status of all devices.
     * @return Combined status string
     */
    std::string getAllStatus();
};