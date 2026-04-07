# 1. Find Qt6 (Required for ServerApp)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# 2. Server library with all logic and networking
add_library(server_lib
    StateMachine.cpp
    RequestHandler.cpp
    AuthManager.cpp
    DeviceManager.cpp
    LogManager.cpp
    NetworkPacket.cpp   # This is your renamed binary packet implementation
    NetworkManager.cpp
 "Header/Packet.h")

# 3. Add BOTH the server root and Header folder to the search path
# This allows "#include <Packet.h>" to be found in Header/
# and "#include <Header/RequestHandler.h>" to be found in src/server/
target_include_directories(server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Header
)

# 4. Link Windows Sockets
target_link_libraries(server_lib PUBLIC ws2_32)

# 5. Enable Qt automatic tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 6. Server executable
add_executable(ServerApp
    main.cpp
    SmartHomeServer.cpp
    SmartHomeServer.h
 "Header/Packet.h")

target_link_libraries(ServerApp PRIVATE 
    server_lib
    Qt6::Widgets
)