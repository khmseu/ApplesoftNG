#include "core/asm_port_reason.hpp"

#include "core/asm_port_error_handling.hpp"
#include "core/asm_port_strlt2.hpp"

namespace applesoft::asm_port {
namespace {

bool has_room(const AS_REASONState& state) {
    return state.y < state.fretopHi ||
           (state.y == state.fretopHi && state.a < state.fretopAS_Lo);
}

void invokeGarbageCollector(AS_REASONState& state) {
    // AS_REASON calls into the global AS_GARBAG routine before retrying the space check.
    ::applesoft::asm_port::AS_GARBAG();
    state.garbageCollected = true;
}

} // namespace

AS_REASONResult AS_REASON(AS_REASONState& state) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_REASON (inclusive) .. AS_MEMERR (exclusive)
    // Name normalization: none (assembler label AS_REASON kept verbatim).

    if (has_room(state)) {
        return AS_REASONResult{true, state.a, state.y, state.x};
    }

    const std::uint8_t savedA = state.a;
    const std::uint8_t savedY = state.y;
    const auto savedScratch = state.temp1ToFacExclusive;

    invokeGarbageCollector(state);

    state.temp1ToFacExclusive = savedScratch;
    state.a = savedA;
    state.y = savedY;

    if (has_room(state)) {
        return AS_REASONResult{true, state.a, state.y, state.x};
    }

    state.x = ::applesoft::asm_port::AS_MEMERR();
    state.memerrCalled = true;
    return AS_REASONResult{false, state.a, state.y, state.x};
}

} // namespace applesoft::asm_port