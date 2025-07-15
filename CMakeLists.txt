cmake_minimum_required(VERSION 3.16)

project(pebble-db LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Source files
file(GLOB_RECURSE SOURCES
    src/*.cc
    src/*.cpp
    src/*.cxx
    src/*.c
)

# Define the executable
add_executable(pebble-db
    ${SOURCES}
)

# Add the include/ directory for this target
target_include_directories(pebble-db
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
