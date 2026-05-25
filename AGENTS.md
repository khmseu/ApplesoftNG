# AGENTS.md

## Purpose

Guidance for AI coding agents working in this repository.

## Project At A Glance

- C++ Applesoft clone with current executable target: ApplesoftClone.
- Language standard: C++23 (set in CMake and intended for both GCC 13.3 and Clang 18.1 toolchains).
- Runtime code is split across `src/core`, `src/platform`, and matching headers in `include/core` and `include/platform`.
- Core port modules now live as `asm_port_*.cpp` / `asm_port_*.hpp` pairs; use the exact label names from `SourceMaterial/Combo/asrom.sym`.
- Authoritative source material: [SourceMaterial/Combo](SourceMaterial/Combo). Symbols in the combined listing (`asrom.lst`) and symbol table (`asrom.sym`) already include `AS_` or `MON_` prefixes.
- Historical Apple II assembly/listing material in `Apple-II-Source-slim` is reference-only.
- Start with project overview in [README.md](README.md).

## Build And Run

- Configure: `cmake -S . -B build`
- Build: `cmake --build build`
- Run: `./build/ApplesoftClone`

These commands are verified in this workspace.

## Code Boundaries

- Runtime C++ implementation is in [src](src) and [include](include).
- Core interpreter logic: [src/core](src/core), public headers in [include/core](include/core).
- Platform console abstraction: [src/platform](src/platform), headers in [include/platform](include/platform).
- Emulated/native pointer bridge: [include/core/applesoft_dual_pointer.hpp](include/core/applesoft_dual_pointer.hpp).
- I/O-mapped state and soft-switch placeholders: [src/core/io_ports.cpp](src/core/io_ports.cpp) and [include/core/io_ports.hpp](include/core/io_ports.hpp).
- Reference corpus: [SourceMaterial/Combo](SourceMaterial/Combo).

## Important Pitfalls

- Do not introduce runtime coupling to historical assembler/listing files.
- Symbols are now literal: Use the exact label name found in `SourceMaterial/Combo/asrom.sym` (e.g., `AS_CHRGET`, `MON_COUT`).
- Tests in [tests/core_tests.cpp](tests/core_tests.cpp) are not currently wired into CMake and may not match current interfaces; verify before relying on them.

## Editing Conventions

- Keep changes minimal and localized.
- Prefer preserving existing style (straightforward modern C++23 headers/sources).
- Update [README.md](README.md) when project structure or build/run behavior changes.

## Reference Links

- Main project guide: [README.md](README.md)
- Authoritative symbol table: [SourceMaterial/Combo/asrom.sym](SourceMaterial/Combo/asrom.sym)
- Authoritative listing: [SourceMaterial/Combo/asrom.lst](SourceMaterial/Combo/asrom.lst)

## AI Conversion Support

- Label window rule for asm conversion: start label is inclusive, end label is exclusive.
- Conversion instructions: [.github/instructions/asm-to-cpp.instructions.md](.github/instructions/asm-to-cpp.instructions.md)
- Incremental conversion skill: [.github/skills/asm-incremental-port/SKILL.md](.github/skills/asm-incremental-port/SKILL.md)
- Prompt template for two-label conversion: [.github/prompts/port-asm-label-range.prompt.md](.github/prompts/port-asm-label-range.prompt.md)
