
cmake_minimum_required(VERSION 3.10)
project(SmartHomeSystem)

add_compile_options(/EHsc)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the shared folder so everyone can see Birendra's packets
include_directories(src/shared)

# Tell CMake where to find the code
add_subdirectory(src/client)
add_subdirectory(src/server)


enable_testing()
add_subdirectory(tests)