#include "NetworkPacket.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

namespace
{
    /**
     * Simple structure to map test names to their implementation functions.
     * Used by the command-line test runner in main().
     */
    struct TestCase
    {
        const char* name;
        void (*function)();
    };

    /**
     * Helper function to verify the payload contents of a NetworkPacket.
     */
    void assertPayloadEquals(const NetworkPacket& packet, const char* expected, unsigned int length)
    {
        assert(packet.getPayloadLength() == length);

        if (length == 0)
        {
            assert(packet.getPayload() == nullptr);
            return;
        }

        assert(packet.getPayload() != nullptr);
        assert(std::memcmp(packet.getPayload(), expected, length) == 0);
    }

    // --- TEST CASES ---

    void test_default_constructor()
    {
        // Verifies that a new packet starts with the correct protocol defaults.
        // Magic 0x7E and Version 1 are required by our binary protocol.
        NetworkPacket packet;

        assert(packet.getMagicNumber() == 0x7E);
        assert(packet.getVersion() == 1);
        assert(packet.getCommandId() == 0);
        assert(packet.getStatusCode() == 0);
        assertPayloadEquals(packet, nullptr, 0);
        assert(packet.isValid());
    }

    void test_parameterized_constructor_and_setters()
    {
        // Checks if we can initialize a packet with specific values and update 
        // them using setter methods.
        NetworkPacket packet(10, 20);

        assert(packet.getCommandId() == 10);
        assert(packet.getStatusCode() == 20);

        const char payload[] = "LIGHT_ON";
        packet.setCommandId(30);
        packet.setStatusCode(40);
        packet.setPayload(payload, sizeof(payload) - 1);

        assert(packet.getCommandId() == 30);
        assert(packet.getStatusCode() == 40);
        assertPayloadEquals(packet, payload, sizeof(payload) - 1);
        assert(packet.isValid());
    }

    void test_copy_constructor_deep_copy()
    {
        // Ensures that copying a packet creates a unique memory allocation for the payload.
        // Changing the original should NOT affect the copy.
        const char payload[] = "admin,1234";
        NetworkPacket original(1, 200);
        original.setPayload(payload, sizeof(payload) - 1);

        NetworkPacket copy(original);

        // Verify data matches
        assert(copy.getChecksum() == original.getChecksum());
        assertPayloadEquals(copy, payload, sizeof(payload) - 1);

        // Verify memory is unique (Deep Copy)
        assert(copy.getPayload() != original.getPayload());

        // Modifying original should not touch the copy
        original.setPayload("changed", 7);
        assertPayloadEquals(copy, payload, sizeof(payload) - 1);
    }

    void test_assignment_operator_and_self_assignment()
    {
        // Tests the '=' operator, including the critical self-assignment guard (a = a).
        NetworkPacket source(9, 500);
        source.setPayload("JSON", 4);

        NetworkPacket destination(7, 404);
        destination = source;

        assert(destination.getCommandId() == source.getCommandId());
        assertPayloadEquals(destination, "JSON", 4);

        // Self-assignment should do nothing and remain stable
        const char* payloadBefore = destination.getPayload();
        destination = destination;
        assert(destination.getPayload() == payloadBefore);
    }

    void test_serialize_and_deserialize_round_trip()
    {
        // This is the most important test: it verifies that an object can be 
        // turned into bytes and reconstructed perfectly on the other side.
        const char payload[] = { 'J', 'P', 'E', 'G', '\0', 'B', 'Y', 'T', 'E', 'S' };

        NetworkPacket original(55, 201);
        original.setPayload(payload, sizeof(payload));

        unsigned int size = 0;
        char* buffer = original.serialize(size);

        // Protocol size: 10 (Header) + Payload + 2 (Checksum)
        assert(size == 10 + sizeof(payload) + 2);

        NetworkPacket parsed;
        const bool success = parsed.deserialize(buffer, size);

        assert(success);
        assert(parsed.getCommandId() == original.getCommandId());
        assert(parsed.getChecksum() == original.getChecksum());
        assertPayloadEquals(parsed, payload, sizeof(payload));

        delete[] buffer;
    }

    void test_deserialize_rejects_invalid_inputs()
    {
        // Security check: The parser should reject data with bad magic numbers, 
        // wrong versions, or corrupted checksums without crashing.
        NetworkPacket packet(88, 777);
        packet.setPayload("SAFE", 4);

        unsigned int validSize = 0;
        char* validBuffer = packet.serialize(validSize);

        // Test 1: Wrong size
        assert(!packet.deserialize(validBuffer, 5));

        // Test 2: Bad Magic Number
        char* badMagic = new char[validSize];
        std::memcpy(badMagic, validBuffer, validSize);
        badMagic[0] = 0x00; // Protocol requires 0x7E
        assert(!packet.deserialize(badMagic, validSize));

        // Test 3: Corrupted Checksum
        char* badChecksum = new char[validSize];
        std::memcpy(badChecksum, validBuffer, validSize);
        badChecksum[validSize - 1] ^= 0xFF;
        assert(!packet.deserialize(badChecksum, validSize));

        delete[] validBuffer;
        delete[] badMagic;
        delete[] badChecksum;
    }

    // --- TEST RUNNER ---

    int runSelectedTests(int argc, char** argv, const TestCase* tests, int testCount)
    {
        if (argc <= 1)
        {
            for (int i = 0; i < testCount; ++i) tests[i].function();
            return 0;
        }

        const std::string selectedTest = argv[1];
        for (int i = 0; i < testCount; ++i)
        {
            if (selectedTest == tests[i].name)
            {
                tests[i].function();
                return 0;
            }
        }

        std::cerr << "Unknown NetworkPacket test: " << selectedTest << std::endl;
        return 1;
    }
}

int main(int argc, char** argv)
{
    std::cout << "--- Starting NetworkPacket Tests ---" << std::endl;

    const TestCase tests[] = {
        {"default_constructor", test_default_constructor},
        {"parameterized_constructor_and_setters", test_parameterized_constructor_and_setters},
        {"copy_constructor_deep_copy", test_copy_constructor_deep_copy},
        {"assignment_operator_and_self_assignment", test_assignment_operator_and_self_assignment},
        {"serialize_and_deserialize_round_trip", test_serialize_and_deserialize_round_trip},
        {"deserialize_rejects_invalid_inputs", test_deserialize_rejects_invalid_inputs},
    };

    const int result = runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));

    if (result == 0) std::cout << "--- NetworkPacket Tests Passed ---" << std::endl;

    return result;
}