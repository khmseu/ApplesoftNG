#include "core/asm_port_chkmem.hpp"

#include "core/asm_port_error_handling.hpp"

namespace applesoft::asm_port {
CHKMEMResult CHKMEM(CHKMEMState& state) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKMEM (inclusive) .. REASON (exclusive)
    // Name normalization: none (assembler label CHKMEM kept verbatim).

    const std::uint16_t required = static_cast<std::uint16_t>(state.a) * 2u + 54u;
    if (required > 0xffu) {
        state.x = ::applesoft::asm_port::MEMERR();
        state.memerrCalled = true;
        return CHKMEMResult{false, static_cast<std::uint8_t>(required), state.x};
    }

    state.a = static_cast<std::uint8_t>(required);
    state.index = state.a;
    state.x = state.stackPointer;

    if (state.x < state.index) {
        state.x = ::applesoft::asm_port::MEMERR();
        state.memerrCalled = true;
        return CHKMEMResult{false, state.a, state.x};
    }

    return CHKMEMResult{true, state.a, state.x};
}

} // namespace applesoft::asm_port