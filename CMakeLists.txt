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
    src/platform/asm_port_getln.cpp
    src/platform/asm_port_outdo.cpp
    src/platform/console.cpp
    src/core/asm_port_inlin.cpp
    src/core/asm_port_inlin2.cpp
    src/core/asm_port_input.cpp
    src/core/asm_port_nxin.cpp
    src/core/asm_port_strtxt.cpp
    src/core/asm_port_strlit.cpp
    src/core/asm_port_strlt2.cpp
    src/core/asm_port_token_address_table.cpp
    src/core/asm_port_unfnc.cpp
    src/core/asm_port_mathtbl.cpp
    src/core/asm_port_token_name_table.cpp
    src/core/asm_port_error_messages.cpp
    src/core/asm_port_qt_error.cpp
    src/core/asm_port_gtforpnt.cpp
    src/core/asm_port_bltu.cpp
    src/core/asm_port_chkmem.cpp
    src/core/asm_port_reason.cpp
    src/core/asm_port_memerr.cpp
    src/core/applesoft_variables.cpp
    src/core/asm_port_error.cpp
    src/core/asm_port_print.cpp
)

# Add executable
add_executable(ApplesoftClone ${SOURCES})

# Link libraries if needed
# target_link_libraries(ApplesoftClone <libraries>)