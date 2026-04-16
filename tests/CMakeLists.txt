
# If the user has QTDIR set, make sure CMake can find Qt without hardcoded paths.
if(DEFINED ENV{QTDIR})
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()
find_package(Qt6 COMPONENTS Widgets REQUIRED)

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

# Client tests
add_executable(ClientTests
    client_tests.cpp
    ../src/client/SmartHomeClient.cpp
    ../src/client/NetworkManager.cpp
    ../src/client/pages/HomePage.cpp
    ../src/client/pages/MapPage.cpp
    ../src/client/pages/ProfilePage.cpp
    ../src/client/pages/SettingsPage.cpp
    ../src/client/pages/RoomDetailPage.cpp
    ../src/client/pages/SignUpPage.cpp
    ../src/client/components/ClimateWidget.cpp
    ../src/server/NetworkPacket.cpp
)

target_include_directories(ClientTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/client
    ${CMAKE_SOURCE_DIR}/src/client/pages
    ${CMAKE_SOURCE_DIR}/src/client/components
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/Header
)

target_link_libraries(ClientTests PRIVATE
    server_lib
    Qt6::Widgets
    ws2_32
)

set_target_properties(ClientTests PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_options(ClientTests PRIVATE /PROFILE)

get_target_property(QT_CORE_DLL Qt6::Core IMPORTED_LOCATION_DEBUG)
if(NOT QT_CORE_DLL)
    get_target_property(QT_CORE_DLL Qt6::Core IMPORTED_LOCATION_RELEASE)
endif()

if(QT_CORE_DLL)
    get_filename_component(QT_BIN_DIR "${QT_CORE_DLL}" DIRECTORY)
    set(QT_WINDEPLOYQT "${QT_BIN_DIR}/windeployqt.exe")
    if(EXISTS "${QT_WINDEPLOYQT}")
        add_custom_command(TARGET ClientTests POST_BUILD
            COMMAND "${QT_WINDEPLOYQT}" --debug --no-translations --dir "$<TARGET_FILE_DIR:ClientTests>" "$<TARGET_FILE:ClientTests>"
        )
    endif()
endif()

# Request/handler integration tests
add_executable(RequestIntegrationTests
    request_integration_tests.cpp
)

target_link_libraries(RequestIntegrationTests PRIVATE server_lib)
target_link_options(RequestIntegrationTests PRIVATE /PROFILE)

target_include_directories(RequestIntegrationTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/Header
)

function(add_named_case target case_name test_id)
    set(test_name SmartHomeTests.${target}.${test_id}_${case_name})
    add_test(NAME ${test_name} COMMAND ${target} ${case_name})
    if(target STREQUAL "ClientTests" AND DEFINED ENV{QTDIR})
        set_tests_properties(${test_name} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$ENV{QTDIR}/bin")
    endif()
endfunction()

add_named_case(PacketTests default_constructor UT-PKT-001)
add_named_case(PacketTests parameterized_constructor_and_setters UT-PKT-002)
add_named_case(PacketTests copy_constructor_deep_copy UT-PKT-003)
add_named_case(PacketTests assignment_operator_and_self_assignment UT-PKT-004)
add_named_case(PacketTests serialize_and_deserialize_round_trip UT-PKT-005)
add_named_case(PacketTests empty_payload_paths UT-PKT-006)
add_named_case(PacketTests deserialize_rejects_invalid_inputs_without_mutating_packet UT-PKT-007)
add_named_case(PacketTests deserialize_rejects_invalid_inputs UT-PKT-008)
add_named_case(PacketTests string_payload_constructor_sets_payload UT-PKT-009)
add_named_case(PacketTests set_payload_nullptr_with_nonzero_length_clears UT-PKT-010)
add_named_case(PacketTests deserialize_rejects_oversized_payload_length_header UT-PKT-011)

add_named_case(ClientTests login_success_shows_dashboard_and_persists_user UT-CLT-001)
add_named_case(ClientTests invalid_credentials_show_error UT-CLT-002)
add_named_case(ClientTests show_sign_up_page_navigates_to_registration UT-CLT-003)
add_named_case(ClientTests registration_adds_user_and_returns_to_login UT-CLT-004)
add_named_case(ClientTests registered_user_can_login UT-CLT-005)
add_named_case(ClientTests room_selection_loads_garage_detail_page UT-CLT-006)
add_named_case(ClientTests device_selection_routes_kitchen_devices UT-CLT-007)
add_named_case(ClientTests device_selection_defaults_to_living_room UT-CLT-008)
add_named_case(ClientTests toggle_sidebar_collapses_and_expands UT-CLT-009)
add_named_case(ClientTests logout_clears_password_and_returns_to_login UT-CLT-010)
add_named_case(ClientTests mode_change_sends_expected_packet UT-CLT-011)
add_named_case(ClientTests login_when_server_offline_shows_system_error UT-CLT-012)
add_named_case(ClientTests saved_last_user_loads_on_startup UT-CLT-013)
add_named_case(ClientTests living_room_device_selection_routes_correctly UT-CLT-014)
add_named_case(ClientTests mode_change_without_connection_still_invokes_safely UT-CLT-015)
add_named_case(ClientTests request_security_image_without_connection_is_handled UT-CLT-016)
add_named_case(ClientTests request_security_image_valid_image_updates_monitor UT-CLT-017)
add_named_case(ClientTests request_security_image_zero_size_does_not_update_monitor UT-CLT-018)
add_named_case(ClientTests request_security_image_oversized_does_not_update_monitor UT-CLT-019)
add_named_case(ClientTests request_security_image_corrupt_payload_does_not_update_monitor UT-CLT-020)

add_named_case(NetworkManagerTests send_all_and_recv_all IT-001)
add_named_case(NetworkManagerTests send_all_zero_size_returns_true IT-002)
add_named_case(NetworkManagerTests recv_all_zero_size_returns_true IT-003)
add_named_case(NetworkManagerTests recv_all_multiple_iterations IT-004)
add_named_case(NetworkManagerTests send_packet_and_receive_packet IT-005)
add_named_case(NetworkManagerTests send_packet_and_receive_packet_with_empty_payload IT-006)
add_named_case(NetworkManagerTests send_all_failure_on_invalid_socket IT-007)
add_named_case(NetworkManagerTests send_packet_failure_on_invalid_socket IT-008)
add_named_case(NetworkManagerTests recv_all_failure_on_invalid_socket IT-009)
add_named_case(NetworkManagerTests receive_packet_header_failure_on_invalid_socket IT-010)
add_named_case(NetworkManagerTests receive_packet_rejects_invalid_checksum IT-011)
add_named_case(NetworkManagerTests receive_packet_rejects_invalid_magic IT-012)
add_named_case(NetworkManagerTests receive_packet_rejects_invalid_version IT-013)
add_named_case(NetworkManagerTests receive_packet_fails_on_incomplete_payload IT-014)
add_named_case(NetworkManagerTests receive_packet_rejects_oversized_payload_length IT-033)

add_named_case(RequestIntegrationTests network_login_success IT-015)
add_named_case(RequestIntegrationTests network_login_invalid_credentials IT-016)
add_named_case(RequestIntegrationTests network_login_invalid_format IT-017)
add_named_case(RequestIntegrationTests network_get_status_initially_locked IT-018)
add_named_case(RequestIntegrationTests network_get_status_after_away_transition IT-019)
add_named_case(RequestIntegrationTests network_get_status_after_maintenance_transition IT-020)
add_named_case(RequestIntegrationTests network_authenticated_mode_change_sequence IT-021)
add_named_case(RequestIntegrationTests network_set_mode_unknown_mode IT-022)
add_named_case(RequestIntegrationTests network_return_to_locked_from_away IT-023)
add_named_case(RequestIntegrationTests network_return_to_locked_from_maintenance IT-024)
add_named_case(RequestIntegrationTests network_unauthenticated_device_command_rejected IT-025)
add_named_case(RequestIntegrationTests network_turn_on_and_get_device_status IT-026)
add_named_case(RequestIntegrationTests network_turn_off_device IT-027)
add_named_case(RequestIntegrationTests network_turn_on_invalid_device_returns_failure IT-028)
add_named_case(RequestIntegrationTests network_turn_off_invalid_device_returns_failure IT-029)
add_named_case(RequestIntegrationTests network_get_all_device_status IT-030)
add_named_case(RequestIntegrationTests network_maintenance_rejects_device_command IT-031)
add_named_case(RequestIntegrationTests network_invalid_command_id IT-032)

# Server tests
add_executable(Test_Server
    tests_Server.cpp
)

target_link_libraries(Test_Server PRIVATE server_lib)
target_link_options(Test_Server PRIVATE /PROFILE)

target_include_directories(Test_Server PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server/Header
)

add_named_case(Test_Server initial_state UT-SVR-001)
add_named_case(Test_Server locked_to_home UT-SVR-002)
add_named_case(Test_Server locked_to_away_invalid UT-SVR-003)
add_named_case(Test_Server home_to_away UT-SVR-004)
add_named_case(Test_Server get_status_initially_locked UT-SVR-005)
add_named_case(Test_Server set_mode_without_auth_fails UT-SVR-006)
add_named_case(Test_Server set_mode_with_auth_works UT-SVR-007)
add_named_case(Test_Server valid_login UT-SVR-008)
add_named_case(Test_Server invalid_login UT-SVR-009)
add_named_case(Test_Server turn_on_device UT-SVR-010)
add_named_case(Test_Server turn_off_device UT-SVR-011)
add_named_case(Test_Server invalid_device UT-SVR-012)
add_named_case(Test_Server turn_on_device_through_handler UT-SVR-013)
add_named_case(Test_Server get_device_status_through_handler UT-SVR-014)
add_named_case(Test_Server device_command_rejected_in_locked UT-SVR-015)
add_named_case(Test_Server device_command_rejected_in_maintenance UT-SVR-016)
add_named_case(Test_Server device_command_works_in_home UT-SVR-017)
add_named_case(Test_Server get_all_device_status UT-SVR-018)
add_named_case(Test_Server log_event UT-SVR-019)
add_named_case(Test_Server home_to_maintenance UT-SVR-020)
add_named_case(Test_Server away_to_locked UT-SVR-021)
add_named_case(Test_Server login_invalid_format UT-SVR-022)
add_named_case(Test_Server set_mode_unknown_mode UT-SVR-023)
add_named_case(Test_Server invalid_command UT-SVR-024)
add_named_case(Test_Server home_to_locked UT-SVR-025)
add_named_case(Test_Server away_to_home UT-SVR-026)
add_named_case(Test_Server maintenance_to_locked UT-SVR-027)
add_named_case(Test_Server set_mode_away_with_auth_works UT-SVR-028)
add_named_case(Test_Server turn_off_device_through_handler UT-SVR-029)
add_named_case(Test_Server get_invalid_device_status_through_handler UT-SVR-030)
add_named_case(Test_Server maintenance_to_home UT-SVR-031)
add_named_case(Test_Server away_to_maintenance_invalid UT-SVR-032)
add_named_case(Test_Server turn_on_invalid_device_through_handler UT-SVR-033)
add_named_case(Test_Server turn_off_invalid_device_through_handler UT-SVR-034)
