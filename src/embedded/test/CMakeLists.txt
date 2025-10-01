cmake_minimum_required(VERSION 3.10)
project(SensorController_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find nlohmann_json package
find_package(nlohmann_json 3.2.0 REQUIRED)

# Add the test executable
add_executable(test_controller 
    test_controller.cpp
    ../include/MainController.cpp
)

# Include directories
target_include_directories(test_controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link against required libraries
target_link_libraries(test_controller PRIVATE
    nlohmann_json::nlohmann_json
    pthread
)