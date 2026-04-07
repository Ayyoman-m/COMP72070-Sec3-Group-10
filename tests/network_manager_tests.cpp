#include "NetworkManager.h"
#include "NetworkPacket.h"

#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

namespace
{
    /**
     * A simple structure to map test names to their implementation functions.
     */
    struct TestCase
    {
        const char* name;
        void (*function)();
    };

    /**
     * RAII wrapper for Windows Sockets initialization.
     * Ensures WSAStartup is called once at the start of the test suite
     * and WSACleanup is called at the very end.
     */
    class WinsockSession
    {
    public:
        WinsockSession()
        {
            const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            assert(result == 0);
        }

        ~WinsockSession()
        {
            WSACleanup();
        }

    private:
        WSADATA wsaData{};
    };

    /**
     * Creates a local loopback connection (Client <-> Server) for testing.
     * This allows us to test real network calls without needing two physical machines.
     */
    class SocketPair
    {
    public:
        SocketPair()
        {
            // 1. Create a listener to wait for an incoming connection
            SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            assert(listener != INVALID_SOCKET);

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = 0; // Let the OS pick a random available port

            assert(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
            assert(listen(listener, 1) == 0);

            // Retrieve the actual port number the OS assigned
            int addressLength = sizeof(address);
            assert(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &addressLength) == 0);

            client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            assert(client != INVALID_SOCKET);

            // 2. Accept the connection in a separate thread to avoid blocking
            std::thread acceptThread([&]() {
                server = accept(listener, nullptr, nullptr);
                });

            // 3. Connect the client to our local listener
            assert(connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
            acceptThread.join();

            closesocket(listener);
            assert(server != INVALID_SOCKET);
        }

        ~SocketPair()
        {
            if (client != INVALID_SOCKET) closesocket(client);
            if (server != INVALID_SOCKET) closesocket(server);
        }

        SOCKET client{ INVALID_SOCKET };
        SOCKET server{ INVALID_SOCKET };
    };

    /**
     * Helper function to verify that a received NetworkPacket matches the expected values.
     */
    void assertPacketEquals(const NetworkPacket& packet, unsigned short commandId, unsigned short statusCode, const char* payload, unsigned int payloadLength)
    {
        // The protocol requires magic 0x7E and version 1
        assert(packet.getMagicNumber() == 0x7E);
        assert(packet.getVersion() == 1);
        assert(packet.getCommandId() == commandId);
        assert(packet.getStatusCode() == statusCode);
        assert(packet.getPayloadLength() == payloadLength);

        if (payloadLength == 0)
        {
            assert(packet.getPayload() == nullptr);
        }
        else
        {
            assert(packet.getPayload() != nullptr);
            assert(std::memcmp(packet.getPayload(), payload, payloadLength) == 0);
        }
    }

    // --- ACTUAL TEST IMPLEMENTATIONS ---

    void test_send_all_and_recv_all()
    {
        // Verifies that raw byte streams are fully transmitted across the socket
        SocketPair sockets;
        const char payload[] = "network manager raw bytes";
        char received[sizeof(payload)] = {};

        std::thread receiver([&]() {
            const bool ok = NetworkManager::recvAll(sockets.server, received, static_cast<int>(sizeof(payload)));
            assert(ok);
            });

        const bool ok = NetworkManager::sendAll(sockets.client, payload, static_cast<int>(sizeof(payload)));
        assert(ok);

        receiver.join();
        assert(std::memcmp(received, payload, sizeof(payload)) == 0);
    }

    void test_recv_all_multiple_iterations()
    {
        // Simulates a slow network where data arrives in small chunks.
        // recvAll should wait until the entire buffer is filled.
        SocketPair sockets;
        const char payload[] = "chunked recv";
        char received[sizeof(payload)] = {};

        std::thread sender([&]() {
            const int firstChunkSize = 4;
            const int secondChunkSize = 3;
            const int thirdChunkSize = static_cast<int>(sizeof(payload)) - firstChunkSize - secondChunkSize;

            send(sockets.client, payload, firstChunkSize, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            send(sockets.client, payload + firstChunkSize, secondChunkSize, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            send(sockets.client, payload + firstChunkSize + secondChunkSize, thirdChunkSize, 0);
            });

        const bool ok = NetworkManager::recvAll(sockets.server, received, static_cast<int>(sizeof(payload)));
        assert(ok);

        sender.join();
        assert(std::memcmp(received, payload, sizeof(payload)) == 0);
    }

    void test_send_packet_and_receive_packet()
    {
        // Tests the high-level object transmission (Object -> Bytes -> Object)
        SocketPair sockets;
        const char payload[] = { 'L', 'I', 'G', 'H', 'T', '_', 'O', 'N', '\0', 'X' };

        NetworkPacket sentPacket(42, 200);
        sentPacket.setPayload(payload, sizeof(payload));

        NetworkPacket receivedPacket;

        std::thread receiver([&]() {
            const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
            assert(ok);
            });

        const bool ok = NetworkManager::sendPacket(sockets.client, sentPacket);
        assert(ok);

        receiver.join();
        assertPacketEquals(receivedPacket, 42, 200, payload, sizeof(payload));
    }

    void test_receive_packet_rejects_invalid_checksum()
    {
        // Security check: If a packet is tampered with on the wire, the 
        // NetworkManager should reject it during deserialization.
        SocketPair sockets;
        NetworkPacket packet(91, 500);
        packet.setPayload("BAD", 3);

        unsigned int size = 0;
        char* serialized = packet.serialize(size);

        // Corrupt the checksum byte at the end of the packet
        serialized[size - 1] ^= 0x01;

        std::thread sender([&]() {
            NetworkManager::sendAll(sockets.client, serialized, static_cast<int>(size));
            });

        NetworkPacket receivedPacket;
        const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
        assert(!ok); // Should fail validation

        sender.join();
        delete[] serialized;
    }

    void test_receive_packet_fails_on_incomplete_payload()
    {
        // Edge case: If the connection is dropped halfway through receiving a 
        // packet, the function should return false instead of hanging.
        SocketPair sockets;
        NetworkPacket packet(7, 8);
        packet.setPayload("partial", 7);

        unsigned int size = 0;
        char* serialized = packet.serialize(size);

        // Only send the header and part of the body
        const int truncatedSize = static_cast<int>(size - 2);

        std::thread sender([&]() {
            NetworkManager::sendAll(sockets.client, serialized, truncatedSize);
            shutdown(sockets.client, SD_SEND); // Terminate stream early
            });

        NetworkPacket receivedPacket;
        const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
        assert(!ok);

        sender.join();
        delete[] serialized;
    }

    // --- BOILERPLATE TEST RUNNER ---

    int runSelectedTests(int argc, char** argv, const TestCase* tests, int testCount)
    {
        if (argc <= 1)
        {
            // Run everything if no specific test name is provided
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

        std::cerr << "Unknown NetworkManager test: " << selectedTest << std::endl;
        return 1;
    }
}

int main(int argc, char** argv)
{
    std::cout << "--- Starting NetworkManager Tests ---" << std::endl;

    WinsockSession winsock; // Initialize Windows networking

    const TestCase tests[] = {
        {"send_all_and_recv_all", test_send_all_and_recv_all},
        {"recv_all_multiple_iterations", test_recv_all_multiple_iterations},
        {"send_packet_and_receive_packet", test_send_packet_and_receive_packet},
        {"receive_packet_rejects_invalid_checksum", test_receive_packet_rejects_invalid_checksum},
        {"receive_packet_fails_on_incomplete_payload", test_receive_packet_fails_on_incomplete_payload},
        // ... (Add your other failure cases here as needed)
    };

    const int result = runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));

    if (result == 0) std::cout << "--- NetworkManager Tests Passed ---" << std::endl;

    return result;
}