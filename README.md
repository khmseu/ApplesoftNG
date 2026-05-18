# Applesoft Clone

This project is a clone of the original Applesoft BASIC interpreter, implemented in C++. Historical Apple II assembly listings are kept under SourceMaterial as reference material for behavior and compatibility, not as compiled runtime components.

## Project Structure

- **SourceMaterial/**: Reference corpus from original Apple II sources and project analysis.
  - **report.txt**: Notes and analysis used during reconstruction work.
  - **Combo/**: Authoritative combined ROM assembly artifacts (`asrom.lst`, `asrom.s`, `asrom.sym`). These use absolute ROM addresses and include native `AS_`/`MON_` prefixes for all symbols.
  - **Apple-II-Source-slim/**: Curated source snapshot (historical context only).
    - **src/system/applesoft/**: Applesoft listings and symbols.
    - **src/system/monitor/apple2plus/**: Apple II monitor component listings and symbols.

- **include/**: Contains header files for the project.
  - **core/**: Core components of the interpreter.
    - `interpreter.hpp`: Declaration of the `Interpreter` class.
    - `memory.hpp`: Declaration of the `Memory` class.
  - **platform/**: Platform-independent console input/output functions.
    - `console.hpp`: Declaration of console functions.

- **src/**: Contains the implementation of the project.
  - **core/**: Implementation of core components.
    - `interpreter.cpp`: Implementation of the `Interpreter` class.
    - `memory.cpp`: Implementation of the `Memory` class.
  - **platform/**: Implementation of platform functions.
    - `console.cpp`: Implementation of console functions.
  - `main.cpp`: Entry point of the application.

- **tests/**: Contains unit tests for the project.
  - `core_tests.cpp`: Unit tests for the core components.

- **tools/**: Contains utility scripts.
  - `source_importer.py`: Script for importing and processing original assembly sources.

- **CMakeLists.txt**: Configuration file for CMake.

## Setup Instructions

1. Clone the repository:

   ```bash
   git clone <repository-url>
   cd applesoft-clone
   ```

2. Build the project using CMake:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. Run the interpreter:

   ```bash
   ./ApplesoftClone
   ```

## Contribution Guidelines

Contributions are welcome! Please open an issue or submit a pull request for any enhancements or bug fixes. Make sure to follow the coding standards and include tests for new features.
