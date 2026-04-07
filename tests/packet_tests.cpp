#include "packet.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

namespace
{
struct TestCase
{
    const char* name;
    void (*function)();
};

void assertPayloadEquals(const Packet& packet, const char* expected, unsigned int length)
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

void test_default_constructor()
{
    Packet packet;

    assert(packet.getMagicNumber() == 0x7E);
    assert(packet.getVersion() == 1);
    assert(packet.getCommandId() == 0);
    assert(packet.getStatusCode() == 0);
    assertPayloadEquals(packet, nullptr, 0);
    assert(packet.getChecksum() == packet.calculateChecksum());
    assert(packet.isValid());
}

void test_parameterized_constructor_and_setters()
{
    Packet packet(10, 20);

    assert(packet.getCommandId() == 10);
    assert(packet.getStatusCode() == 20);
    assert(packet.isValid());

    const char payload[] = "LIGHT_ON";
    packet.setCommandId(30);
    packet.setStatusCode(40);
    packet.setPayload(payload, sizeof(payload) - 1);

    assert(packet.getCommandId() == 30);
    assert(packet.getStatusCode() == 40);
    assertPayloadEquals(packet, payload, sizeof(payload) - 1);
    assert(packet.getChecksum() == packet.calculateChecksum());
    assert(packet.isValid());

    packet.setPayload(nullptr, 0);
    assertPayloadEquals(packet, nullptr, 0);

    packet.setPayload(nullptr, 5);
    assertPayloadEquals(packet, nullptr, 0);
}

void test_copy_constructor_deep_copy()
{
    const char payload[] = "admin,1234";
    Packet original(1, 200);
    original.setPayload(payload, sizeof(payload) - 1);

    Packet copy(original);

    assert(copy.getMagicNumber() == original.getMagicNumber());
    assert(copy.getVersion() == original.getVersion());
    assert(copy.getCommandId() == original.getCommandId());
    assert(copy.getStatusCode() == original.getStatusCode());
    assert(copy.getChecksum() == original.getChecksum());
    assertPayloadEquals(copy, payload, sizeof(payload) - 1);
    assert(copy.getPayload() != original.getPayload());

    const char newPayload[] = "guest,0000";
    original.setPayload(newPayload, sizeof(newPayload) - 1);

    assertPayloadEquals(original, newPayload, sizeof(newPayload) - 1);
    assertPayloadEquals(copy, payload, sizeof(payload) - 1);
}

void test_assignment_operator_and_self_assignment()
{
    const char sourcePayload[] = "JSON";
    const char oldPayload[] = "old";

    Packet source(9, 500);
    source.setPayload(sourcePayload, sizeof(sourcePayload) - 1);

    Packet destination(7, 404);
    destination.setPayload(oldPayload, sizeof(oldPayload) - 1);

    destination = source;

    assert(destination.getCommandId() == source.getCommandId());
    assert(destination.getStatusCode() == source.getStatusCode());
    assert(destination.getChecksum() == source.getChecksum());
    assertPayloadEquals(destination, sourcePayload, sizeof(sourcePayload) - 1);
    assert(destination.getPayload() != source.getPayload());

    const char* payloadBeforeSelfAssign = destination.getPayload();
    destination = destination;
    assert(destination.getPayload() == payloadBeforeSelfAssign);
    assertPayloadEquals(destination, sourcePayload, sizeof(sourcePayload) - 1);
}

void test_serialize_and_deserialize_round_trip()
{
    const char payload[] = {'J', 'P', 'E', 'G', '\0', 'B', 'Y', 'T', 'E', 'S'};

    Packet original(55, 201);
    original.setPayload(payload, sizeof(payload));

    unsigned int size = 0;
    char* buffer = original.serialize(size);

    assert(size == 10 + sizeof(payload) + 2);
    assert(buffer != nullptr);

    Packet parsed;
    const bool success = parsed.deserialize(buffer, size);

    assert(success);
    assert(parsed.getMagicNumber() == original.getMagicNumber());
    assert(parsed.getVersion() == original.getVersion());
    assert(parsed.getCommandId() == original.getCommandId());
    assert(parsed.getStatusCode() == original.getStatusCode());
    assert(parsed.getChecksum() == original.getChecksum());
    assertPayloadEquals(parsed, payload, sizeof(payload));

    delete[] buffer;
}

void test_empty_payload_paths()
{
    Packet emptySource(5, 6);
    Packet emptyCopy(emptySource);

    assertPayloadEquals(emptyCopy, nullptr, 0);
    assert(emptyCopy.getChecksum() == emptyCopy.calculateChecksum());

    Packet destination(9, 10);
    const char payload[] = "data";
    destination.setPayload(payload, sizeof(payload) - 1);
    destination = emptySource;

    assert(destination.getCommandId() == emptySource.getCommandId());
    assert(destination.getStatusCode() == emptySource.getStatusCode());
    assertPayloadEquals(destination, nullptr, 0);
    assert(destination.isValid());

    unsigned int size = 0;
    char* buffer = emptySource.serialize(size);
    assert(size == 12);

    Packet parsed;
    assert(parsed.deserialize(buffer, size));
    assert(parsed.getCommandId() == emptySource.getCommandId());
    assert(parsed.getStatusCode() == emptySource.getStatusCode());
    assertPayloadEquals(parsed, nullptr, 0);

    delete[] buffer;
}

void test_deserialize_rejects_invalid_inputs_without_mutating_packet()
{
    const char originalPayload[] = "SAFE";
    Packet packet(88, 777);
    packet.setPayload(originalPayload, sizeof(originalPayload) - 1);

    unsigned int validSize = 0;
    char* validBuffer = packet.serialize(validSize);

    assert(!packet.deserialize(nullptr, validSize));
    assert(!packet.deserialize(validBuffer, 11));
    assertPayloadEquals(packet, originalPayload, sizeof(originalPayload) - 1);

    char* wrongSizeBuffer = new char[validSize];
    std::memcpy(wrongSizeBuffer, validBuffer, validSize);
    wrongSizeBuffer[6] = 0;
    wrongSizeBuffer[7] = 0;
    wrongSizeBuffer[8] = 0;
    wrongSizeBuffer[9] = 0;
    assert(!packet.deserialize(wrongSizeBuffer, validSize));
    assertPayloadEquals(packet, originalPayload, sizeof(originalPayload) - 1);

    char* badMagicBuffer = new char[validSize];
    std::memcpy(badMagicBuffer, validBuffer, validSize);
    badMagicBuffer[0] = 0x01;
    assert(!packet.deserialize(badMagicBuffer, validSize));
    assertPayloadEquals(packet, originalPayload, sizeof(originalPayload) - 1);

    char* badVersionBuffer = new char[validSize];
    std::memcpy(badVersionBuffer, validBuffer, validSize);
    badVersionBuffer[1] = 0x02;
    assert(!packet.deserialize(badVersionBuffer, validSize));
    assertPayloadEquals(packet, originalPayload, sizeof(originalPayload) - 1);

    char* badChecksumBuffer = new char[validSize];
    std::memcpy(badChecksumBuffer, validBuffer, validSize);
    badChecksumBuffer[validSize - 1] ^= 0x01;
    assert(!packet.deserialize(badChecksumBuffer, validSize));
    assertPayloadEquals(packet, originalPayload, sizeof(originalPayload) - 1);

    delete[] validBuffer;
    delete[] wrongSizeBuffer;
    delete[] badMagicBuffer;
    delete[] badVersionBuffer;
    delete[] badChecksumBuffer;
}

int runSelectedTests(int argc, char** argv, const TestCase* tests, int testCount)
{
    if (argc <= 1)
    {
        for (int i = 0; i < testCount; ++i)
        {
            tests[i].function();
        }
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

    std::cerr << "Unknown Packet test: " << selectedTest << std::endl;
    return 1;
}
}

int main(int argc, char** argv)
{
    std::cout << "--- Starting Packet Tests ---" << std::endl;

    const TestCase tests[] = {
        {"default_constructor", test_default_constructor},
        {"parameterized_constructor_and_setters", test_parameterized_constructor_and_setters},
        {"copy_constructor_deep_copy", test_copy_constructor_deep_copy},
        {"assignment_operator_and_self_assignment", test_assignment_operator_and_self_assignment},
        {"serialize_and_deserialize_round_trip", test_serialize_and_deserialize_round_trip},
        {"empty_payload_paths", test_empty_payload_paths},
        {"deserialize_rejects_invalid_inputs_without_mutating_packet", test_deserialize_rejects_invalid_inputs_without_mutating_packet},
    };

    const int result = runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));

    if (result == 0)
    {
        std::cout << "--- Packet Tests Passed ---" << std::endl;
    }

    return result;
}
