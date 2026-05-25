# Applesoft Clone

This project is a C++ port of the original Applesoft BASIC interpreter. Historical Apple II assembly listings are kept under SourceMaterial as reference material for behavior and compatibility, not as compiled runtime components.

## Project Structure

- **SourceMaterial/**: Reference corpus from original Apple II sources and project analysis.
  - **Combo/**: Authoritative combined ROM assembly artifacts (`asrom.lst`, `asrom.s`, `asrom.sym`, `asrom.err`). These use absolute ROM addresses and include native `AS_`/`MON_` prefixes for all symbols.
  - **Apple-II-Source-slim/**: Curated source snapshot (historical context only).
- **include/core/**: Public headers for the interpreter core, including `applesoft_dual_pointer.hpp`, `io_ports.hpp`, and the `asm_port_*.hpp` module headers.
- **include/platform/**: Console abstraction headers.
- **src/core/**: Core interpreter implementation. This contains the `asm_port_*.cpp` modules plus `interpreter.cpp`, `memory.cpp`, `io_ports.cpp`, and `jump_table.cpp`.
- **src/platform/**: Platform and console I/O implementation.
- **tests/**: Contains the legacy `core_tests.cpp` entry point. It is not currently wired into CMake.
- **tools/**: Utility scripts.
- **CMakeLists.txt**: Configuration file for CMake.

## Build And Run

```bash
cmake -S . -B build
cmake --build build
./build/ApplesoftClone
```

## Development Notes

- Treat `SourceMaterial/Combo/asrom.sym` as the authoritative symbol source.
- Use `ApplesoftDualPointer<T>` when a value may refer to either emulated ROM state or native C++ storage.
- Historical `Apple-II-Source-slim` material is reference-only.
- Keep the docs in sync with module splits and build wiring when those change.

## Contributing

Keep changes minimal and localized, preserve the current module naming scheme, and update the docs when build or runtime behavior changes.
