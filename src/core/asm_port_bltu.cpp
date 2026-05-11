#include "core/asm_port_bltu.hpp"
#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {
namespace {

void copy_page_backward(BLTU2State& state) {
    const std::uint16_t source = state.sourcePointer();
    const std::uint16_t destination = state.destinationPointer();

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
    state.setStrend(ApplesoftVariables::makeWord(state.a, state.y));

    // Execution falls through to BLTU2 in the original ROM.
    return BLTUResult{state.a, state.y};
}

BLTU2Result BLTU2(BLTU2State& state) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BLTU2 (inclusive) .. CHKMEM (exclusive)
    // Name normalization: none (assembler label BLTU2 kept verbatim).

    if (state.y != 0) {
        state.setSourcePointer(static_cast<std::uint16_t>(state.sourcePointer() - state.y));
        state.setDestinationPointer(static_cast<std::uint16_t>(state.destinationPointer() - state.y));
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
