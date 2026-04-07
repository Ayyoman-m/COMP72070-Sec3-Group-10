# Packet tests
add_executable(PacketTests
    packet_tests.cpp
    ../src/server/NetworkPacket.cpp
)

target_include_directories(PacketTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
)

target_link_options(PacketTests PRIVATE /PROFILE)

# NetworkManager tests
add_executable(NetworkManagerTests
    network_manager_tests.cpp
    ../src/server/NetworkPacket.cpp
    ../src/server/NetworkManager.cpp
)

target_include_directories(NetworkManagerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
)

target_link_libraries(NetworkManagerTests PRIVATE ws2_32)
target_link_options(NetworkManagerTests PRIVATE /PROFILE)
# Link ws2_32 to BOTH test projects
target_link_libraries(PacketTests PRIVATE ws2_32)
target_link_libraries(NetworkManagerTests PRIVATE ws2_32)

function(add_named_case target case_name)
    add_test(NAME ${target}.${case_name} COMMAND ${target} ${case_name})
endfunction()

add_named_case(PacketTests default_constructor)
add_named_case(PacketTests parameterized_constructor_and_setters)
add_named_case(PacketTests copy_constructor_deep_copy)
add_named_case(PacketTests assignment_operator_and_self_assignment)
add_named_case(PacketTests serialize_and_deserialize_round_trip)
add_named_case(PacketTests empty_payload_paths)
add_named_case(PacketTests deserialize_rejects_invalid_inputs_without_mutating_packet)

add_named_case(NetworkManagerTests send_all_and_recv_all)
add_named_case(NetworkManagerTests send_all_zero_size_returns_true)
add_named_case(NetworkManagerTests recv_all_zero_size_returns_true)
add_named_case(NetworkManagerTests recv_all_multiple_iterations)
add_named_case(NetworkManagerTests send_packet_and_receive_packet)
add_named_case(NetworkManagerTests send_packet_and_receive_packet_with_empty_payload)
add_named_case(NetworkManagerTests send_all_failure_on_invalid_socket)
add_named_case(NetworkManagerTests send_packet_failure_on_invalid_socket)
add_named_case(NetworkManagerTests recv_all_failure_on_invalid_socket)
add_named_case(NetworkManagerTests receive_packet_header_failure_on_invalid_socket)
add_named_case(NetworkManagerTests receive_packet_rejects_invalid_checksum)
add_named_case(NetworkManagerTests receive_packet_rejects_invalid_magic)
add_named_case(NetworkManagerTests receive_packet_rejects_invalid_version)
add_named_case(NetworkManagerTests receive_packet_fails_on_incomplete_payload)

# Server tests
add_executable(Test_Server
    tests_Server.cpp
)

target_link_libraries(Test_Server PRIVATE server_lib)

target_include_directories(Test_Server PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server/Header
)

add_test(NAME AllServerTests COMMAND Test_Server)