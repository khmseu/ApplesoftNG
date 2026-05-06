#include "core/asm_port_bltu.hpp"

namespace applesoft::asm_port {
namespace {

std::uint16_t pointer_from(std::uint8_t lo, std::uint8_t hi) {
    return static_cast<std::uint16_t>(lo | (static_cast<std::uint16_t>(hi) << 8));
}

void pointer_to(std::uint16_t value, std::uint8_t& lo, std::uint8_t& hi) {
    lo = static_cast<std::uint8_t>(value & 0x00ff);
    hi = static_cast<std::uint8_t>(value >> 8);
}

void copy_page_backward(BLTU2State& state) {
    std::uint16_t source = pointer_from(state.hightrLo, state.hightrHi);
    std::uint16_t destination = pointer_from(state.highdsLo, state.highdsHi);

    do {
        state.memory[destination + state.y] = state.memory[source + state.y];
        --state.y;
    } while (state.y != 0);

    state.memory[destination] = state.memory[source];
}

} // namespace

BLTUResult BLTU(BLTUState& state) {
    // BLTU starts by validating/grooming destination via REASON.
    state.reason.a = state.a;
    state.reason.y = state.y;
    const REASONResult reasonResult = REASON(state.reason);
    state.a = reasonResult.a;
    state.y = reasonResult.y;

    // New top of array storage (STREND) is loaded from adjusted A/Y.
    state.strendLo = state.a;
    state.strendHi = state.y;

    // Execution falls through to BLTU2 in the original ROM.
    return BLTUResult{state.a, state.y};
}

BLTU2Result BLTU2(BLTU2State& state) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BLTU2 (inclusive) .. CHKMEM (exclusive)
    // Name normalization: none (assembler label BLTU2 kept verbatim).

    if (state.y != 0) {
        pointer_to(static_cast<std::uint16_t>(pointer_from(state.hightrLo, state.hightrHi) - state.y),
                   state.hightrLo,
                   state.hightrHi);
        pointer_to(static_cast<std::uint16_t>(pointer_from(state.highdsLo, state.highdsHi) - state.y),
                   state.highdsLo,
                   state.highdsHi);
        copy_page_backward(state);
    }

    do {
        state.hightrHi = static_cast<std::uint8_t>(state.hightrHi - 1);
        state.highdsHi = static_cast<std::uint8_t>(state.highdsHi - 1);
        --state.x;

        if (state.x == 0) {
            break;
        }

        --state.y;
        copy_page_backward(state);
    } while (true);

    return BLTU2Result{state.x, state.y};
}

} // namespace applesoft::asm_port
