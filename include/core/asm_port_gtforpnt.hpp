#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GTFORPNT (inclusive) .. BLTU (exclusive)
// Name normalization: none (assembler label GTFORPNT kept verbatim).

inline constexpr std::uint8_t TOKEN_FOR = 0x81;

// Minimal state needed by GTFORPNT while broader runtime memory wiring is pending.
struct GTFORPNTState {
    // 6502 stack page: logical addresses $0100..$01FF map to indices 0..255.
    std::array<std::uint8_t, 256> stackPage{};

    // Zero-page FORPNT ($85/$86): variable address to search for.
    std::uint8_t forpntLo = 0;
    std::uint8_t forpntHi = 0;
};

struct GTFORPNTResult {
    bool found = false;

    // Final X register value on return (stack frame pointer examined/found).
    std::uint8_t x = 0;
};

// Scan FOR frames on the 6502 stack for a variable pointer match.
// `stackPointer` is the incoming 6502 SP before `TSX`.
// Side effect: when called from NEXT with no variable (`forpntHi == 0`),
// FORPNT is loaded from the first FOR frame encountered (matching ROM behavior).
GTFORPNTResult GTFORPNT(std::uint8_t stackPointer, GTFORPNTState& state);

} // namespace applesoft::asm_port
