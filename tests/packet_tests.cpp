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

    void assertPacketState(const NetworkPacket& packet, uint16_t commandId, uint16_t statusCode, const char* payload, unsigned int length)
    {
        assert(packet.getMagicNumber() == 0x7E);
        assert(packet.getVersion() == 1);
        assert(packet.getCommandId() == commandId);
        assert(packet.getStatusCode() == statusCode);
        assertPayloadEquals(packet, payload, length);
        assert(packet.isValid());
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

    void test_empty_payload_paths()
    {
        NetworkPacket packet(12, 204);
        assertPacketState(packet, 12, 204, nullptr, 0);

        packet.setPayload("ON", 2);
        assertPacketState(packet, 12, 204, "ON", 2);

        packet.setPayload(nullptr, 0);
        assertPacketState(packet, 12, 204, nullptr, 0);

        unsigned int size = 0;
        char* buffer = packet.serialize(size);
        assert(size == 12);

        NetworkPacket parsed(99, 500);
        parsed.setPayload("junk", 4);
        const bool success = parsed.deserialize(buffer, size);

        assert(success);
        assertPacketState(parsed, 12, 204, nullptr, 0);

        NetworkPacket emptyStringPacket(7, std::string());
        assertPacketState(emptyStringPacket, 7, 0, nullptr, 0);

        delete[] buffer;
    }

    void test_string_payload_constructor_sets_payload()
    {
        // Verifies the convenience constructor sets the payload and keeps status code defaulted to 0.
        NetworkPacket packet(7, std::string("HELLO"));
        assertPacketState(packet, 7, 0, "HELLO", 5);
    }

    void test_set_payload_nullptr_with_nonzero_length_clears()
    {
        // Defensive behavior: if data is nullptr, we treat it as an empty payload regardless of length.
        NetworkPacket packet(12, 204);
        packet.setPayload("ON", 2);
        assertPacketState(packet, 12, 204, "ON", 2);

        packet.setPayload(nullptr, 5);
        assertPacketState(packet, 12, 204, nullptr, 0);
    }

    void test_deserialize_rejects_oversized_payload_length_header()
    {
        // Security check: reject packets that claim a payload length larger than the provided buffer.
        NetworkPacket original(44, 201);
        original.setPayload("SAFE", 4);

        NetworkPacket packetUnderTest = original;

        unsigned int validSize = 0;
        char* validBuffer = original.serialize(validSize);

        // Overwrite the payload length field with an absurd value. The buffer size stays small.
        uint32_t hugeLength = 0xFFFFFFFF;
        std::memcpy(validBuffer + 6, &hugeLength, 4);

        assert(!packetUnderTest.deserialize(validBuffer, validSize));
        assertPacketState(packetUnderTest, 44, 201, "SAFE", 4);

        delete[] validBuffer;
    }

    void test_deserialize_rejects_invalid_inputs_without_mutating_packet()
    {
        NetworkPacket original(44, 201);
        original.setPayload("SAFE", 4);

        NetworkPacket packetUnderTest = original;

        unsigned int validSize = 0;
        char* validBuffer = original.serialize(validSize);

        char* badMagic = new char[validSize];
        std::memcpy(badMagic, validBuffer, validSize);
        badMagic[0] = 0x00;
        assert(!packetUnderTest.deserialize(badMagic, validSize));
        assertPacketState(packetUnderTest, 44, 201, "SAFE", 4);

        char* badVersion = new char[validSize];
        std::memcpy(badVersion, validBuffer, validSize);
        badVersion[1] = 2;
        assert(!packetUnderTest.deserialize(badVersion, validSize));
        assertPacketState(packetUnderTest, 44, 201, "SAFE", 4);

        char* badChecksum = new char[validSize];
        std::memcpy(badChecksum, validBuffer, validSize);
        badChecksum[validSize - 1] ^= 0xFF;
        assert(!packetUnderTest.deserialize(badChecksum, validSize));
        assertPacketState(packetUnderTest, 44, 201, "SAFE", 4);

        assert(!packetUnderTest.deserialize(validBuffer, 5));
        assertPacketState(packetUnderTest, 44, 201, "SAFE", 4);

        delete[] validBuffer;
        delete[] badMagic;
        delete[] badVersion;
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
        {"empty_payload_paths", test_empty_payload_paths},
        {"string_payload_constructor_sets_payload", test_string_payload_constructor_sets_payload},
        {"set_payload_nullptr_with_nonzero_length_clears", test_set_payload_nullptr_with_nonzero_length_clears},
        {"deserialize_rejects_oversized_payload_length_header", test_deserialize_rejects_oversized_payload_length_header},
        {"deserialize_rejects_invalid_inputs_without_mutating_packet", test_deserialize_rejects_invalid_inputs_without_mutating_packet},
    };

    const int result = runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));

    if (result == 0) std::cout << "--- NetworkPacket Tests Passed ---" << std::endl;

    return result;
}
