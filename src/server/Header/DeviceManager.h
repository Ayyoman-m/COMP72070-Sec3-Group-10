#pragma once
#include <string>
#include <unordered_map>

// this class manages smart home devices
class DeviceManager {
private:
    // storing device name and its state
    std::unordered_map<std::string, bool> devices;

public:
    DeviceManager();

    // turn device ON
    bool turnOn(const std::string& device);

    // turn device OFF
    bool turnOff(const std::string& device);

    // get one device status
    std::string getStatus(const std::string& device);

    // get all device status in one string
    std::string getAllStatus();
};