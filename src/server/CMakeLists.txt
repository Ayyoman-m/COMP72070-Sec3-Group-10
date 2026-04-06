# Enable Qt's automatic compilation tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the Qt Widgets module
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Add the server files
add_executable(SmartHomeServer 
    main.cpp 
    SmartHomeServer.cpp 
    SmartHomeServer.h
)

# Link the Qt library
target_link_libraries(SmartHomeServer PRIVATE Qt6::Widgets)