#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: BLTU (inclusive) .. BLTU2 (exclusive)
// Name normalization: none (assembler label BLTU kept verbatim).

struct BLTUState {
    // Inputs carried in A/Y on entry.
    std::uint8_t a = 0;
    std::uint8_t y = 0;

    // Destination state updated by BLTU.
    std::uint8_t strendLo = 0;
    std::uint8_t strendHi = 0;
};

struct BLTUResult {
    // A/Y after REASON and stores; BLTU2 consumes these next.
    std::uint8_t a = 0;
    std::uint8_t y = 0;
};

// Minimal state needed by BLTU2 while broader runtime memory wiring is pending.
struct BLTU2State {
    // Flat 6502 address space used by the backward copy routine.
    std::array<std::uint8_t, 65536> memory{};

    // Registers on entry to BLTU2.
    std::uint8_t x = 0;
    std::uint8_t y = 0;

    // Zero-page pointers used by the ROM routine.
    std::uint8_t hightrLo = 0;
    std::uint8_t hightrHi = 0;
    std::uint8_t highdsLo = 0;
    std::uint8_t highdsHi = 0;
};

struct BLTU2Result {
    // Registers on return.
    std::uint8_t x = 0;
    std::uint8_t y = 0;
};

// Execute the BLTU prologue before control falls into BLTU2.
BLTUResult BLTU(BLTUState& state);

// Copy bytes backward from HIGHTR to HIGHDS, finishing with full 256-byte pages.
BLTU2Result BLTU2(BLTU2State& state);

} // namespace applesoft::asm_port
