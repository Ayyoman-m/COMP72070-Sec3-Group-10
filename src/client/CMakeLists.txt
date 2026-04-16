# --- Client Configuration ---

# 1. Enable Qt's automatic compilation tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 2. Find Qt6 Widgets
# Visual Studio's CMake integration does not always apply presets/workspace settings.
# If the user has QTDIR set, make sure CMake can find Qt without hardcoded paths.
if(DEFINED ENV{QTDIR})
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# 3. Define the File Lists
set(CLIENT_SOURCES
    main.cpp
    SmartHomeClient.cpp
    pages/HomePage.cpp
    pages/MapPage.cpp
    pages/ProfilePage.cpp
    pages/SettingsPage.cpp
    pages/RoomDetailPage.cpp
    components/ClimateWidget.cpp
)

set(CLIENT_HEADERS
    SmartHomeClient.h
    pages/HomePage.h
    pages/MapPage.h
    pages/ProfilePage.h
    pages/SettingsPage.h
    pages/RoomDetailPage.h
    components/ClimateWidget.h
    StyleManager.h
)

# 4. Create the ClientApp target (ONLY ONCE)
add_executable(ClientApp ${CLIENT_SOURCES} ${CLIENT_HEADERS} "pages/SignUpPage.h" "pages/SignUpPage.cpp" "NetworkManager.cpp" "NetworkManager.h")

# 5. Set include paths (Combined Server + UI folders)
target_include_directories(ClientApp PRIVATE 
    # Server/Networking headers
    ${CMAKE_SOURCE_DIR}/src/server/Header
    ${CMAKE_SOURCE_DIR}/src/server
    # Local UI folders
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pages
    ${CMAKE_CURRENT_SOURCE_DIR}/components
)

# 6. Link libraries
target_link_libraries(ClientApp PRIVATE 
    server_lib 
    Qt6::Widgets 
    ws2_32
)

# 7. Set output directory
set_target_properties(ClientApp PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
