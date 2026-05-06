#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHKMEM (inclusive) .. REASON (exclusive)
// Name normalization: none (assembler label CHKMEM kept verbatim).

struct CHKMEMState {
    // Input A register on entry. The ROM doubles this requirement and adds 54 bytes.
    std::uint8_t a = 0;

    // Zero-page INDEX written by CHKMEM.
    std::uint8_t index = 0;

    // Incoming 6502 stack pointer consumed by TSX.
    std::uint8_t stackPointer = 0;

    // X register state after TSX or MEMERR.
    std::uint8_t x = 0;

    // Indicates that CHKMEM branched to MEMERR instead of returning normally.
    bool memerrCalled = false;
};

struct CHKMEMResult {
    bool ok = false;
    std::uint8_t a = 0;
    std::uint8_t x = 0;
};

// Check whether the current stack pointer leaves enough room for runtime frames.
CHKMEMResult CHKMEM(CHKMEMState& state);

} // namespace applesoft::asm_port