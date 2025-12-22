cmake_minimum_required(VERSION 3.10)

# Set the project name
project(huangh-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable with source files from src directory
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/collections_example.cpp
    src/http_server.cpp
    src/database_manager.cpp
    src/logger.cpp
    src/redis_manager.cpp
    src/config_manager.cpp
)

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(include/huangh-cpp)
include_directories(third_party) 

# Find nlohmann_json package
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

if (APPLE)
    include_directories(/opt/homebrew/include)
    # Add httplib library
    target_link_libraries(${PROJECT_NAME} pthread sqlite3 spdlog::spdlog fmt::fmt nlohmann_json::nlohmann_json /opt/homebrew/lib/libhiredis.dylib)
else()
    target_link_libraries(${PROJECT_NAME} pthread sqlite3 spdlog::spdlog fmt::fmt nlohmann_json::nlohmann_json hiredis)
endif()