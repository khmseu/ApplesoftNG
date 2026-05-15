#include "core/asm_port_reason.hpp"

#include "core/asm_port_error_handling.hpp"

namespace applesoft::asm_port {
namespace {

bool has_room(const REASONState& state) {
    return state.y < state.fretopHi ||
           (state.y == state.fretopHi && state.a < state.fretopLo);
}

// TODO(asm-port): port GARBAG label.
void GARBAG(REASONState& state) {
    state.garbageCollected = true;
}

} // namespace

REASONResult REASON(REASONState& state) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: REASON (inclusive) .. MEMERR (exclusive)
    // Name normalization: none (assembler label REASON kept verbatim).

    if (has_room(state)) {
        return REASONResult{true, state.a, state.y, state.x};
    }

    const std::uint8_t savedA = state.a;
    const std::uint8_t savedY = state.y;
    const auto savedScratch = state.temp1ToFacExclusive;

    GARBAG(state);

    state.temp1ToFacExclusive = savedScratch;
    state.a = savedA;
    state.y = savedY;

    if (has_room(state)) {
        return REASONResult{true, state.a, state.y, state.x};
    }

    state.x = ::applesoft::asm_port::MEMERR();
    state.memerrCalled = true;
    return REASONResult{false, state.a, state.y, state.x};
}

} // namespace applesoft::asm_port