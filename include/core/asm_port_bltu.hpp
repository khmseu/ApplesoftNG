#pragma once

#include <array>
#include <cstdint>

#include "core/applesoft_variables.hpp"
#include "core/asm_port_reason.hpp"

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: BLTU (inclusive) .. BLTU2 (exclusive)
// Name normalization: none (assembler label BLTU kept verbatim).

struct BLTUState {
    // Inputs carried in A/Y on entry.
    std::uint8_t a = 0;
    std::uint8_t y = 0;

    // Minimal REASON state needed by the BLTU prologue.
    REASONState reason{};

    // Destination state updated by BLTU.
    std::uint8_t strendLo = 0;
    std::uint8_t strendHi = 0;

    std::uint16_t strend() const {
        return ApplesoftVariables::makeWord(strendLo, strendHi);
    }

    void setStrend(std::uint16_t value) {
        strendLo = static_cast<std::uint8_t>(value & 0x00ffu);
        strendHi = static_cast<std::uint8_t>(value >> 8);
    }
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

    std::uint16_t sourcePointer() const {
        return ApplesoftVariables::makeWord(hightrLo, hightrHi);
    }

    void setSourcePointer(std::uint16_t value) {
        hightrLo = static_cast<std::uint8_t>(value & 0x00ffu);
        hightrHi = static_cast<std::uint8_t>(value >> 8);
    }

    std::uint16_t destinationPointer() const {
        return ApplesoftVariables::makeWord(highdsLo, highdsHi);
    }

    void setDestinationPointer(std::uint16_t value) {
        highdsLo = static_cast<std::uint8_t>(value & 0x00ffu);
        highdsHi = static_cast<std::uint8_t>(value >> 8);
    }
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
