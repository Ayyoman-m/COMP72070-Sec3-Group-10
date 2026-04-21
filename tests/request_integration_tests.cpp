#include "../src/server/Header/ClientSession.h"
#include "../src/server/Header/DeviceManager.h"
#include "../src/server/Header/LogManager.h"
#include "../src/server/Header/RequestHandler.h"
#include "../src/server/Header/StateMachine.h"

#include <iostream>
#include <string>

namespace
{
    struct TestCase
    {
        const char* name;
        bool (*function)();
    };

    struct TestContext
    {
        StateMachine stateMachine;
        DeviceManager deviceManager;
        LogManager logManager;
        ClientSession session;
        RequestHandler handler;

        TestContext()
            : handler(stateMachine, deviceManager, logManager)
        {
        }
    };

    bool expect(bool condition, const char* passMessage, const char* failMessage)
    {
        if (condition)
        {
            std::cout << "PASS: " << passMessage << "\n";
            return true;
        }

        std::cout << "FAIL: " << failMessage << "\n";
        return false;
    }
}

bool testNetworkLoginSuccess()
{
    TestContext ctx;

    const std::string result = ctx.handler.handleNetworkPacket(1, "admin,1234", ctx.session);

    return expect(
        result == "LOGIN SUCCESS" && ctx.session.isAuthenticated() && ctx.session.getUsername() == "admin",
        "Network LOGIN authenticates valid credentials",
        "Network LOGIN should authenticate valid credentials");
}

bool testNetworkLoginInvalidCredentials()
{
    TestContext ctx;

    const std::string result = ctx.handler.handleNetworkPacket(1, "admin,wrong", ctx.session);

    return expect(
        result == "LOGIN FAILED" && !ctx.session.isAuthenticated(),
        "Network LOGIN rejects invalid credentials",
        "Network LOGIN should reject invalid credentials");
}

bool testNetworkLoginInvalidFormat()
{
    TestContext ctx;

    const std::string result = ctx.handler.handleNetworkPacket(1, "admin-only", ctx.session);

    return expect(
        result == "ERROR: INVALID LOGIN FORMAT" && !ctx.session.isAuthenticated(),
        "Network LOGIN rejects invalid payload format",
        "Network LOGIN should reject invalid payload format");
}

bool testNetworkGetStatusInitiallyLocked()
{
    TestContext ctx;

    const std::string result = ctx.handler.handleNetworkPacket(2, "", ctx.session);

    return expect(
        result == "LOCKED",
        "Network GET_STATUS returns LOCKED initially",
        "Network GET_STATUS should return LOCKED initially");
}

bool testNetworkGetStatusAfterAwayTransition()
{
    TestContext ctx;

    const std::string loginResult = ctx.handler.handleNetworkPacket(1, "admin,1234", ctx.session);
    const std::string homeResult = ctx.handler.handleNetworkPacket(3, "HOME", ctx.session);
    const std::string awayResult = ctx.handler.handleNetworkPacket(3, "AWAY", ctx.session);
    const std::string statusResult = ctx.handler.handleNetworkPacket(2, "", ctx.session);

    return expect(
        loginResult == "LOGIN SUCCESS" && homeResult == "SUCCESS" &&
        awayResult == "SUCCESS" && statusResult == "AWAY",
        "Network GET_STATUS returns AWAY after a valid AWAY transition",
        "Network GET_STATUS should return AWAY after a valid AWAY transition");
}

bool testNetworkGetStatusAfterMaintenanceTransition()
{
    TestContext ctx;

    const std::string loginResult = ctx.handler.handleNetworkPacket(1, "admin,1234", ctx.session);
    const std::string homeResult = ctx.handler.handleNetworkPacket(3, "HOME", ctx.session);
    const std::string maintenanceResult = ctx.handler.handleNetworkPacket(3, "MAINTENANCE", ctx.session);
    const std::string statusResult = ctx.handler.handleNetworkPacket(2, "", ctx.session);

    return expect(
        loginResult == "LOGIN SUCCESS" && homeResult == "SUCCESS" &&
        maintenanceResult == "SUCCESS" && statusResult == "MAINTENANCE",
        "Network GET_STATUS returns MAINTENANCE after a valid maintenance transition",
        "Network GET_STATUS should return MAINTENANCE after a valid maintenance transition");
}

bool testNetworkAuthenticatedModeChangeSequence()
{
    TestContext ctx;

    const std::string loginResult = ctx.handler.handleNetworkPacket(1, "admin,1234", ctx.session);
    const std::string setModeResult = ctx.handler.handleNetworkPacket(3, "HOME", ctx.session);
    const std::string statusResult = ctx.handler.handleNetworkPacket(2, "", ctx.session);

    return expect(
        loginResult == "LOGIN SUCCESS" && setModeResult == "SUCCESS" && statusResult == "HOME",
        "Authenticated network mode change updates server state",
        "Authenticated network mode change should update server state");
}

bool testNetworkSetModeUnknownMode()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);

    const std::string result = ctx.handler.handleNetworkPacket(3, "VACATION", ctx.session);

    return expect(
        result == "ERROR: UNKNOWN MODE",
        "Network SET_MODE rejects unknown mode values",
        "Network SET_MODE should reject unknown mode values");
}

bool testNetworkReturnToLockedFromAway()
{
    TestContext ctx;

    const std::string loginResult = ctx.handler.handleNetworkPacket(1, "admin,1234", ctx.session);
    const std::string homeResult = ctx.handler.handleNetworkPacket(3, "HOME", ctx.session);
    const std::string awayResult = ctx.handler.handleNetworkPacket(3, "AWAY", ctx.session);
    const std::string lockedResult = ctx.handler.handleNetworkPacket(3, "LOCKED", ctx.session);
    const std::string statusResult = ctx.handler.handleNetworkPacket(2, "", ctx.session);

    return expect(
        loginResult == "LOGIN SUCCESS" && homeResult == "SUCCESS" &&
        awayResult == "SUCCESS" && lockedResult == "SUCCESS" &&
        statusResult == "LOCKED",
        "Network SET_MODE returns from AWAY to LOCKED",
        "Network SET_MODE should allow AWAY to LOCKED");
}

bool testNetworkReturnToLockedFromMaintenance()
{
    TestContext ctx;

    const std::string loginResult = ctx.handler.handleNetworkPacket(1, "admin,1234", ctx.session);
    const std::string homeResult = ctx.handler.handleNetworkPacket(3, "HOME", ctx.session);
    const std::string maintenanceResult = ctx.handler.handleNetworkPacket(3, "MAINTENANCE", ctx.session);
    const std::string lockedResult = ctx.handler.handleNetworkPacket(3, "LOCKED", ctx.session);
    const std::string statusResult = ctx.handler.handleNetworkPacket(2, "", ctx.session);

    return expect(
        loginResult == "LOGIN SUCCESS" && homeResult == "SUCCESS" &&
        maintenanceResult == "SUCCESS" && lockedResult == "SUCCESS" &&
        statusResult == "LOCKED",
        "Network SET_MODE returns from MAINTENANCE to LOCKED",
        "Network SET_MODE should allow MAINTENANCE to LOCKED");
}

bool testNetworkUnauthenticatedDeviceCommandRejected()
{
    TestContext ctx;

    const std::string result = ctx.handler.handleNetworkPacket(4, "LIGHT", ctx.session);

    return expect(
        result == "ERROR: NOT AUTHENTICATED",
        "Unauthenticated network device command is rejected",
        "Unauthenticated network device command should be rejected");
}

bool testNetworkTurnOnAndGetDeviceStatus()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);
    ctx.stateMachine.setState(ServerState::HOME);

    const std::string turnOnResult = ctx.handler.handleNetworkPacket(4, "LIGHT", ctx.session);
    const std::string statusResult = ctx.handler.handleNetworkPacket(6, "LIGHT", ctx.session);

    return expect(
        turnOnResult == "SUCCESS" && statusResult == "ON",
        "Network TURN_ON_DEVICE updates device status",
        "Network TURN_ON_DEVICE should update device status");
}

bool testNetworkTurnOffDevice()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);
    ctx.stateMachine.setState(ServerState::HOME);
    ctx.deviceManager.turnOn("FAN");

    const std::string turnOffResult = ctx.handler.handleNetworkPacket(5, "FAN", ctx.session);
    const std::string statusResult = ctx.handler.handleNetworkPacket(6, "FAN", ctx.session);

    return expect(
        turnOffResult == "SUCCESS" && statusResult == "OFF",
        "Network TURN_OFF_DEVICE updates device status",
        "Network TURN_OFF_DEVICE should update device status");
}

bool testNetworkTurnOnInvalidDeviceReturnsFailure()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);
    ctx.stateMachine.setState(ServerState::HOME);

    const std::string result = ctx.handler.handleNetworkPacket(4, "TV", ctx.session);

    return expect(
        result == "FAILURE",
        "Network TURN_ON_DEVICE returns FAILURE for unknown devices",
        "Network TURN_ON_DEVICE should return FAILURE for unknown devices");
}

bool testNetworkTurnOffInvalidDeviceReturnsFailure()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);
    ctx.stateMachine.setState(ServerState::HOME);

    const std::string result = ctx.handler.handleNetworkPacket(5, "TV", ctx.session);

    return expect(
        result == "FAILURE",
        "Network TURN_OFF_DEVICE returns FAILURE for unknown devices",
        "Network TURN_OFF_DEVICE should return FAILURE for unknown devices");
}

bool testNetworkGetAllDeviceStatus()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);
    ctx.stateMachine.setState(ServerState::HOME);
    ctx.deviceManager.turnOn("LIGHT");
    ctx.deviceManager.turnOff("FAN");

    const std::string result = ctx.handler.handleNetworkPacket(7, "", ctx.session);

    return expect(
        result.find("LIGHT=ON") != std::string::npos && result.find("FAN=OFF") != std::string::npos,
        "Network GET_ALL_DEVICE_STATUS returns combined status list",
        "Network GET_ALL_DEVICE_STATUS should return combined status list");
}

bool testNetworkMaintenanceRejectsDeviceCommand()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);
    ctx.stateMachine.setState(ServerState::HOME);
    ctx.stateMachine.setState(ServerState::MAINTENANCE);

    const std::string result = ctx.handler.handleNetworkPacket(4, "LIGHT", ctx.session);

    return expect(
        result == "ERROR: DEVICE COMMAND NOT ALLOWED IN CURRENT STATE",
        "Maintenance state rejects network device commands",
        "Maintenance state should reject network device commands");
}

bool testNetworkInvalidCommandId()
{
    TestContext ctx;
    ctx.session.setAuthenticated(true);

    const std::string result = ctx.handler.handleNetworkPacket(999, "", ctx.session);

    return expect(
        result == "ERROR: INVALID COMMAND",
        "Unknown network command IDs are rejected",
        "Unknown network command IDs should be rejected");
}

int runSelectedTests(int argc, char** argv, const TestCase* tests, int testCount)
{
    if (argc <= 1)
    {
        bool allPassed = true;
        for (int i = 0; i < testCount; ++i)
        {
            if (!tests[i].function())
            {
                allPassed = false;
            }
        }
        return allPassed ? 0 : 1;
    }

    const std::string selectedTest = argv[1];
    for (int i = 0; i < testCount; ++i)
    {
        if (selectedTest == tests[i].name)
        {
            return tests[i].function() ? 0 : 1;
        }
    }

    std::cerr << "Unknown request integration test: " << selectedTest << std::endl;
    return 1;
}

int main(int argc, char** argv)
{
    const TestCase tests[] = {
        {"network_login_success", testNetworkLoginSuccess},
        {"network_login_invalid_credentials", testNetworkLoginInvalidCredentials},
        {"network_login_invalid_format", testNetworkLoginInvalidFormat},
        {"network_get_status_initially_locked", testNetworkGetStatusInitiallyLocked},
        {"network_get_status_after_away_transition", testNetworkGetStatusAfterAwayTransition},
        {"network_get_status_after_maintenance_transition", testNetworkGetStatusAfterMaintenanceTransition},
        {"network_authenticated_mode_change_sequence", testNetworkAuthenticatedModeChangeSequence},
        {"network_set_mode_unknown_mode", testNetworkSetModeUnknownMode},
        {"network_return_to_locked_from_away", testNetworkReturnToLockedFromAway},
        {"network_return_to_locked_from_maintenance", testNetworkReturnToLockedFromMaintenance},
        {"network_unauthenticated_device_command_rejected", testNetworkUnauthenticatedDeviceCommandRejected},
        {"network_turn_on_and_get_device_status", testNetworkTurnOnAndGetDeviceStatus},
        {"network_turn_off_device", testNetworkTurnOffDevice},
        {"network_turn_on_invalid_device_returns_failure", testNetworkTurnOnInvalidDeviceReturnsFailure},
        {"network_turn_off_invalid_device_returns_failure", testNetworkTurnOffInvalidDeviceReturnsFailure},
        {"network_get_all_device_status", testNetworkGetAllDeviceStatus},
        {"network_maintenance_rejects_device_command", testNetworkMaintenanceRejectsDeviceCommand},
        {"network_invalid_command_id", testNetworkInvalidCommandId},
    };

    return runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));
}
