#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: REASON (inclusive) .. MEMERR (exclusive)
// Name normalization: none (assembler label REASON kept verbatim).

struct REASONState {
    // Target address in A/Y on entry and return.
    std::uint8_t a = 0;
    std::uint8_t y = 0;

    // FRETOP low/high bytes used for the space check.
    std::uint8_t fretopLo = 0;
    std::uint8_t fretopHi = 0;

    // Minimal scratch range preserved across garbage collection.
    std::array<std::uint8_t, 10> temp1ToFacExclusive{};

    // X is only observed when MEMERR is taken.
    std::uint8_t x = 0;

    bool garbageCollected = false;
    bool memerrCalled = false;
};

struct REASONResult {
    bool ok = false;
    std::uint8_t a = 0;
    std::uint8_t y = 0;
    std::uint8_t x = 0;
};

// Ensure the array-growth address in A/Y stays below FRETOP, attempting GARBAG first.
REASONResult REASON(REASONState& state);

} // namespace applesoft::asm_port