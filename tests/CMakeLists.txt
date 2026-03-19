# This creates a program that specifically runs your tests
add_executable(SmartHomeTests test_main.cpp)

# Link it to your shared logic so you can test Birendra's packets
target_link_libraries(SmartHomeTests PRIVATE) 

# This registers it with Visual Studio's "Test Explorer"
add_test(NAME AllTests COMMAND SmartHomeTests)