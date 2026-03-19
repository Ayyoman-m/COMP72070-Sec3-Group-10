add_library(server_lib
    StateMachine.cpp
)

target_include_directories(server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Header
)

add_executable(ServerApp
    main.cpp
)

target_link_libraries(ServerApp PRIVATE server_lib)