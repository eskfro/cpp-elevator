cmake_minimum_required(VERSION 3.25)

project(cpp-elevator
    VERSION 0.1.0
    DESCRIPTION "Heislab 2"
    LANGUAGES CXX
)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Legge til subdirs
add_subdirectory(libs)
add_subdirectory(services)

# Generate compile_commands.json for clangd / clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to a sensible build type when one isn't specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")