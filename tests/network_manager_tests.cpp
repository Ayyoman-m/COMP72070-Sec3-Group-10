#include "NetworkManager.h"

#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

namespace
{
struct TestCase
{
    const char* name;
    void (*function)();
};

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

class SocketPair
{
public:
    SocketPair()
    {
        SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert(listener != INVALID_SOCKET);

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;

        assert(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
        assert(listen(listener, 1) == 0);

        int addressLength = sizeof(address);
        assert(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &addressLength) == 0);

        client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert(client != INVALID_SOCKET);

        std::thread acceptThread([&]() {
            server = accept(listener, nullptr, nullptr);
        });

        assert(connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
        acceptThread.join();

        closesocket(listener);
        assert(server != INVALID_SOCKET);
    }

    ~SocketPair()
    {
        if (client != INVALID_SOCKET)
        {
            closesocket(client);
        }

        if (server != INVALID_SOCKET)
        {
            closesocket(server);
        }
    }

    SOCKET client{INVALID_SOCKET};
    SOCKET server{INVALID_SOCKET};
};

void assertPacketEquals(const Packet& packet, unsigned short commandId, unsigned short statusCode, const char* payload, unsigned int payloadLength)
{
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

void test_send_all_and_recv_all()
{
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

void test_send_all_zero_size_returns_true()
{
    const bool ok = NetworkManager::sendAll(INVALID_SOCKET, "", 0);
    assert(ok);
}

void test_recv_all_zero_size_returns_true()
{
    char buffer[1] = {};
    const bool ok = NetworkManager::recvAll(INVALID_SOCKET, buffer, 0);
    assert(ok);
}

void test_recv_all_multiple_iterations()
{
    SocketPair sockets;
    const char payload[] = "chunked recv";
    char received[sizeof(payload)] = {};

    std::thread sender([&]() {
        const int firstChunkSize = 4;
        const int secondChunkSize = 3;
        const int thirdChunkSize = static_cast<int>(sizeof(payload)) - firstChunkSize - secondChunkSize;

        assert(send(sockets.client, payload, firstChunkSize, 0) == firstChunkSize);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        assert(send(sockets.client, payload + firstChunkSize, secondChunkSize, 0) == secondChunkSize);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        assert(send(sockets.client, payload + firstChunkSize + secondChunkSize, thirdChunkSize, 0) == thirdChunkSize);
    });

    const bool ok = NetworkManager::recvAll(sockets.server, received, static_cast<int>(sizeof(payload)));
    assert(ok);

    sender.join();
    assert(std::memcmp(received, payload, sizeof(payload)) == 0);
}

void test_send_packet_and_receive_packet()
{
    SocketPair sockets;
    const char payload[] = {'L', 'I', 'G', 'H', 'T', '_', 'O', 'N', '\0', 'X'};

    Packet sentPacket(42, 200);
    sentPacket.setPayload(payload, sizeof(payload));

    Packet receivedPacket;

    std::thread receiver([&]() {
        const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
        assert(ok);
    });

    const bool ok = NetworkManager::sendPacket(sockets.client, sentPacket);
    assert(ok);

    receiver.join();
    assertPacketEquals(receivedPacket, 42, 200, payload, sizeof(payload));
}

void test_send_packet_and_receive_packet_with_empty_payload()
{
    SocketPair sockets;
    Packet sentPacket(15, 204);
    Packet receivedPacket;

    std::thread receiver([&]() {
        const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
        assert(ok);
    });

    const bool ok = NetworkManager::sendPacket(sockets.client, sentPacket);
    assert(ok);

    receiver.join();
    assertPacketEquals(receivedPacket, 15, 204, nullptr, 0);
}

void test_send_all_failure_on_invalid_socket()
{
    const char payload[] = "fail";
    const bool ok = NetworkManager::sendAll(INVALID_SOCKET, payload, static_cast<int>(sizeof(payload)));
    assert(!ok);
}

void test_send_packet_failure_on_invalid_socket()
{
    Packet packet(3, 4);
    const char payload[] = "send packet failure";
    packet.setPayload(payload, sizeof(payload) - 1);

    const bool ok = NetworkManager::sendPacket(INVALID_SOCKET, packet);
    assert(!ok);
}

void test_recv_all_failure_on_invalid_socket()
{
    char buffer[8] = {};
    const bool ok = NetworkManager::recvAll(INVALID_SOCKET, buffer, static_cast<int>(sizeof(buffer)));
    assert(!ok);
}

void test_receive_packet_header_failure_on_invalid_socket()
{
    Packet packet;
    const bool ok = NetworkManager::receivePacket(INVALID_SOCKET, packet);
    assert(!ok);
}

void test_receive_packet_rejects_invalid_checksum()
{
    SocketPair sockets;
    Packet packet(91, 500);
    const char payload[] = "BAD";
    packet.setPayload(payload, sizeof(payload) - 1);

    unsigned int size = 0;
    char* serialized = packet.serialize(size);
    serialized[size - 1] ^= 0x01;

    Packet receivedPacket;

    std::thread sender([&]() {
        const bool ok = NetworkManager::sendAll(sockets.client, serialized, static_cast<int>(size));
        assert(ok);
    });

    const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
    assert(!ok);

    sender.join();
    delete[] serialized;
}

void test_receive_packet_rejects_invalid_magic()
{
    SocketPair sockets;
    Packet packet(12, 34);
    const char payload[] = "magic";
    packet.setPayload(payload, sizeof(payload) - 1);

    unsigned int size = 0;
    char* serialized = packet.serialize(size);
    serialized[0] = 0x01;

    std::thread sender([&]() {
        const bool ok = NetworkManager::sendAll(sockets.client, serialized, static_cast<int>(size));
        assert(ok);
    });

    Packet receivedPacket;
    const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
    assert(!ok);

    sender.join();
    delete[] serialized;
}

void test_receive_packet_rejects_invalid_version()
{
    SocketPair sockets;
    Packet packet(56, 78);
    const char payload[] = "version";
    packet.setPayload(payload, sizeof(payload) - 1);

    unsigned int size = 0;
    char* serialized = packet.serialize(size);
    serialized[1] = 0x02;

    std::thread sender([&]() {
        const bool ok = NetworkManager::sendAll(sockets.client, serialized, static_cast<int>(size));
        assert(ok);
    });

    Packet receivedPacket;
    const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
    assert(!ok);

    sender.join();
    delete[] serialized;
}

void test_receive_packet_fails_on_incomplete_payload()
{
    SocketPair sockets;
    Packet packet(7, 8);
    const char payload[] = "partial";
    packet.setPayload(payload, sizeof(payload) - 1);

    unsigned int size = 0;
    char* serialized = packet.serialize(size);
    const int truncatedSize = static_cast<int>(size - 2);

    std::thread sender([&]() {
        const bool ok = NetworkManager::sendAll(sockets.client, serialized, truncatedSize);
        assert(ok);
        shutdown(sockets.client, SD_SEND);
    });

    Packet receivedPacket;
    const bool ok = NetworkManager::receivePacket(sockets.server, receivedPacket);
    assert(!ok);

    sender.join();
    delete[] serialized;
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

    std::cerr << "Unknown NetworkManager test: " << selectedTest << std::endl;
    return 1;
}
}

int main(int argc, char** argv)
{
    std::cout << "--- Starting NetworkManager Tests ---" << std::endl;

    WinsockSession winsock;

    const TestCase tests[] = {
        {"send_all_and_recv_all", test_send_all_and_recv_all},
        {"send_all_zero_size_returns_true", test_send_all_zero_size_returns_true},
        {"recv_all_zero_size_returns_true", test_recv_all_zero_size_returns_true},
        {"recv_all_multiple_iterations", test_recv_all_multiple_iterations},
        {"send_packet_and_receive_packet", test_send_packet_and_receive_packet},
        {"send_packet_and_receive_packet_with_empty_payload", test_send_packet_and_receive_packet_with_empty_payload},
        {"send_all_failure_on_invalid_socket", test_send_all_failure_on_invalid_socket},
        {"send_packet_failure_on_invalid_socket", test_send_packet_failure_on_invalid_socket},
        {"recv_all_failure_on_invalid_socket", test_recv_all_failure_on_invalid_socket},
        {"receive_packet_header_failure_on_invalid_socket", test_receive_packet_header_failure_on_invalid_socket},
        {"receive_packet_rejects_invalid_checksum", test_receive_packet_rejects_invalid_checksum},
        {"receive_packet_rejects_invalid_magic", test_receive_packet_rejects_invalid_magic},
        {"receive_packet_rejects_invalid_version", test_receive_packet_rejects_invalid_version},
        {"receive_packet_fails_on_incomplete_payload", test_receive_packet_fails_on_incomplete_payload},
    };

    const int result = runSelectedTests(argc, argv, tests, static_cast<int>(sizeof(tests) / sizeof(tests[0])));

    if (result == 0)
    {
        std::cout << "--- NetworkManager Tests Passed ---" << std::endl;
    }

    return result;
}
