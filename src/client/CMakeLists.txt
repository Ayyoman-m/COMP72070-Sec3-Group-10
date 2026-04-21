# --- Client Configuration ---

# 1. Enable Qt's automatic compilation tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 2. Find Qt6 Widgets
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

# --- 8. Deployment for Demo (Copy DLLs and Plugins) ---
if(WIN32)
    # Define paths for clarity
    set(QT_BIN_DIR "C:/Qt/6.11.0/msvc2022_64/bin")
    set(QT_PLUGINS_DIR "C:/Qt/6.11.0/msvc2022_64/plugins")
    set(DEPLOY_DEST "${CMAKE_BINARY_DIR}/bin")

    # Copy the necessary Debug DLLs
    file(COPY 
        "${QT_BIN_DIR}/Qt6Cored.dll"
        "${QT_BIN_DIR}/Qt6Guid.dll"
        "${QT_BIN_DIR}/Qt6Widgetsd.dll"
        "${QT_BIN_DIR}/Qt6Networkd.dll"
        DESTINATION ${DEPLOY_DEST}
    )

    # Create platforms directory and copy the Windows plugin (CRITICAL for GUI)
    file(MAKE_DIRECTORY "${DEPLOY_DEST}/platforms")
    file(COPY 
        "${QT_PLUGINS_DIR}/platforms/qwindowsd.dll"
        DESTINATION "${DEPLOY_DEST}/platforms"
    )
    
    # Optional: Create a logs folder for the client if your NetworkManager uses one
    file(MAKE_DIRECTORY "${DEPLOY_DEST}/logs")
endif()