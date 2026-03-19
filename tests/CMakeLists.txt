add_executable(Test_Server
    tests_Server.cpp
)

target_link_libraries(Test_Server PRIVATE server_lib)

target_include_directories(Test_Server PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server/Header
)

add_test(NAME AllTests COMMAND Test_Server)