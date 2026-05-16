#include "core/asm_port_chkmem.hpp"

#include "core/asm_port_error_handling.hpp"

namespace applesoft::asm_port {
AS_CHKMEMResult AS_CHKMEM(AS_CHKMEMState &state) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_CHKMEM (inclusive) .. AS_REASON (exclusive)
  // Name normalization: none (assembler label AS_CHKMEM kept verbatim).

  const std::uint16_t required = static_cast<std::uint16_t>(state.a) * 2u + 54u;
  if (required > 0xffu) {
    state.x = ::applesoft::asm_port::AS_MEMERR();
    state.memerrCalled = true;
    return AS_CHKMEMResult{false, static_cast<std::uint8_t>(required), state.x};
  }

  state.a = static_cast<std::uint8_t>(required);
  state.index = state.a;
  state.x = state.stackPointer;

  if (state.x < state.index) {
    state.x = ::applesoft::asm_port::AS_MEMERR();
    state.memerrCalled = true;
    return AS_CHKMEMResult{false, state.a, state.x};
  }

  return AS_CHKMEMResult{true, state.a, state.x};
}

} // namespace applesoft::asm_port