cmake_minimum_required(VERSION 3.10)
project(SensorController_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Find nlohmann_json package
find_package(nlohmann_json 3.2.0 REQUIRED)

# If package wasn't found and FetchContent didn't provide a CMake target,
# try a lightweight fallback: download the single-header file into the build
# directory and expose it via an INTERFACE target so existing linking works.
if(NOT nlohmann_json_FOUND)
    set(NLOHMANN_SINGLE_DIR "${CMAKE_BINARY_DIR}/third_party/nlohmann/include")
    file(MAKE_DIRECTORY "${NLOHMANN_SINGLE_DIR}/nlohmann")
    set(NLOHMANN_URL "https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp")
    file(DOWNLOAD "${NLOHMANN_URL}" "${NLOHMANN_SINGLE_DIR}/nlohmann/json.hpp" STATUS _nlohmann_dl_status SHOW_PROGRESS)
    list(GET _nlohmann_dl_status 0 _nlohmann_dl_code)
    if(_nlohmann_dl_code EQUAL 0)
        message(STATUS "Downloaded nlohmann/json.hpp to ${NLOHMANN_SINGLE_DIR}")
        set(NLOHMANN_INCLUDE_DIR "${NLOHMANN_SINGLE_DIR}")
        # Provide an INTERFACE target so the existing target_link_libraries line still works
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE ${NLOHMANN_INCLUDE_DIR})
        # Create a namespaced alias to match the expected target name
        add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    else()
        message(WARNING "Could not download nlohmann single-header (code=${_nlohmann_dl_code})")
    endif()
endif()

# Add the test executable
add_executable(test_controller 
    test_controller.cpp
    ../lib/controllers/MainController.cpp
)

# Include directories
target_include_directories(test_controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

# Link against required libraries
target_link_libraries(test_controller PRIVATE
    nlohmann_json::nlohmann_json
    pthread
)