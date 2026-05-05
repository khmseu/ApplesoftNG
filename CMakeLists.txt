cmake_minimum_required(VERSION 3.10)

project(ApplesoftClone)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/core/interpreter.cpp
    src/core/memory.cpp
    src/platform/console.cpp
)

# Add executable
add_executable(ApplesoftClone ${SOURCES})

# Link libraries if needed
# target_link_libraries(ApplesoftClone <libraries>)