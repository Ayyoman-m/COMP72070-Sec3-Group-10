# --- Client Configuration ---

# 1. Enable Qt's automatic compilation tools for this folder
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 2. Find Qt6 Widgets
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# 3. Create the ClientApp target (CRITICAL: This must come before linking)
add_executable(ClientApp 
    main.cpp 
    SmartHomeClient.cpp 
    SmartHomeClient.h
)

# 4. Set include paths so the client can find server-side headers/packets
target_include_directories(ClientApp PRIVATE 
    ${CMAKE_SOURCE_DIR}/src/server/Header
    ${CMAKE_SOURCE_DIR}/src/server
)

# 5. Link libraries: 
# - server_lib: The shared logic from the server folder
# - Qt6::Widgets: For the GUI
# - ws2_32: The Windows Socket library
target_link_libraries(ClientApp PRIVATE 
    server_lib 
    Qt6::Widgets 
    ws2_32
)

# 6. Optional: Set output directory for the executable
set_target_properties(ClientApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")