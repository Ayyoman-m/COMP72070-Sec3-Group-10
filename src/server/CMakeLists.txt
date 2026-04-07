# Server library with all logic
add_library(server_lib
    StateMachine.cpp
    RequestHandler.cpp
    AuthManager.cpp
    DeviceManager.cpp
    LogManager.cpp
)

target_include_directories(server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Header
)

# Enable Qt's automatic compilation tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the Qt Widgets module
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Server executable with Qt GUI
add_executable(ServerApp
    main.cpp
    SmartHomeServer.cpp
    SmartHomeServer.h
)

target_link_libraries(ServerApp PRIVATE 
    server_lib
    Qt6::Widgets
)