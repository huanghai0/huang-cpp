cmake_minimum_required(VERSION 3.10)

# Set the project name
project(huangh-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(${PROJECT_NAME} main.cpp collections_example.cpp http_server.cpp database_manager.cpp logger.cpp redis_manager.cpp)

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories if needed
# include_directories(include)

# Find nlohmann_json package
find_package(nlohmann_json REQUIRED)

# Add httplib library
target_link_libraries(${PROJECT_NAME} pthread sqlite3 spdlog fmt nlohmann_json::nlohmann_json hiredis)
