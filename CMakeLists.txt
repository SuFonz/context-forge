cmake_minimum_required(VERSION 3.16)
project(ContextForge CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/components/*.cpp")

# executable
add_executable(
    ContextForge
    src/main.cpp
    ${SRC}
)

# dll
add_library(ContextForgeCOM SHARED
    src/com/command.hpp
    src/com/dllmain.cpp
)

# include directories
target_include_directories(ContextForge PRIVATE include)
target_include_directories(ContextForgeCOM PRIVATE include)

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
target_link_libraries(
    ContextForge 
    PRIVATE 
    CLI11::CLI11 
    nlohmann_json::nlohmann_json
    Rpcrt4
)


set_target_properties(ContextForge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

target_compile_options(ContextForge PRIVATE
    -finput-charset=UTF-8
)

set_target_properties(ContextForgeCOM PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

target_compile_options(ContextForgeCOM PRIVATE
    -finput-charset=UTF-8
)
