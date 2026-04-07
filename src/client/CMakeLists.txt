cmake_minimum_required(VERSION 3.10)
project(SmartHomeSystem)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic compilation tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Include server folder so packets are accessible everywhere
include_directories(src/server)
include_directories(src/server/Header)

# Tell CMake where to find the code
add_subdirectory(src/server)
add_subdirectory(src/client)

enable_testing()
add_subdirectory(tests)