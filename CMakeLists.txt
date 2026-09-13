cmake_minimum_required(VERSION 3.16)
project(ContextForge CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ContextForge src/main.cpp)

# include directories
target_include_directories(ContextForge PRIVATE include)

include(FetchContent)

# CLI11
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.2
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(CLI11)

# nlohmann-json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

# link libraries
target_link_libraries(ContextForge PRIVATE CLI11::CLI11 nlohmann_json::nlohmann_json)

if(MSVC)
    target_compile_options(ContextForge PRIVATE /W4 /utf-8)
else()
    target_compile_options(ContextForge PRIVATE -Wall -Wextra -Wpedantic)
endif()
