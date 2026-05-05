# AGENTS.md

## Purpose
Guidance for AI coding agents working in this repository.

## Project At A Glance
- C++ Applesoft clone with current executable target: ApplesoftClone.
- Language standard: C++23 (set in CMake and intended for both GCC 13.3 and Clang 18.1 toolchains).
- Historical Apple II assembly/listing material is reference-only and lives in SourceMaterial.
- Start with project overview in [README.md](README.md).

## Build And Run
- Configure: `cmake -S . -B build`
- Build: `cmake --build build`
- Run: `./build/ApplesoftClone`

These commands are verified in this workspace.

## Code Boundaries
- Runtime C++ implementation is in [src](src) and [include](include).
- Core interpreter logic: [src/core](src/core), public headers in [include/core](include/core).
- Console abstraction: [src/platform](src/platform), headers in [include/platform](include/platform).
- Reference corpus (not compiled into runtime): [SourceMaterial](SourceMaterial).

## Important Pitfalls
- Do not introduce runtime coupling to historical assembler/listing files in SourceMaterial.
- The legacy assembler folders under src/include may still exist; treat them as non-authoritative unless explicitly requested.
- Tests in [tests/core_tests.cpp](tests/core_tests.cpp) are not currently wired into CMake and may not match current interfaces; verify before relying on them.

## Editing Conventions
- Keep changes minimal and localized.
- Prefer preserving existing style (straightforward modern C++23 headers/sources).
- Update [README.md](README.md) when project structure or build/run behavior changes.

## Reference Links
- Main project guide: [README.md](README.md)
- Historical source index: [SourceMaterial/Apple-II-Source-slim/modules.md](SourceMaterial/Apple-II-Source-slim/modules.md)
- Historical project notes: [SourceMaterial/Apple-II-Source-slim/project-description.md](SourceMaterial/Apple-II-Source-slim/project-description.md)

## AI Conversion Support
- Label window rule for asm conversion: start label is inclusive, end label is exclusive.
- Conversion instructions: [.github/instructions/asm-to-cpp.instructions.md](.github/instructions/asm-to-cpp.instructions.md)
- Incremental conversion skill: [.github/skills/asm-incremental-port/SKILL.md](.github/skills/asm-incremental-port/SKILL.md)
- Prompt template for two-label conversion: [.github/prompts/port-asm-label-range.prompt.md](.github/prompts/port-asm-label-range.prompt.md)
