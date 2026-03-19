# This creates a program that specifically runs your tests
add_executable(SmartHomeTests test_main.cpp "tests_Server.cpp")
add_executable(Test_Server
    tests_Server.cpp
)

target_link_libraries(Test_Server PRIVATE server_lib)

target_include_directories(Test_Server PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server/Header
)
