#include "core/asm_port_gtforpnt.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_local_utils.hpp"
#include "core/asm_port_tokens.hpp"

namespace applesoft::asm_port {
namespace {

constexpr std::uint8_t kFrameSize = 18;

std::uint8_t stack_at(const AS_GTFORPNTState &state, std::uint8_t x,
                      std::uint8_t plus) {
  return state.stackPage[local_utils::add_u8(x, plus)];
}

std::uint16_t stack_word_at(const AS_GTFORPNTState &state, std::uint8_t x,
                            std::uint8_t plus) {
  return ApplesoftVariables::makeWord(
      stack_at(state, x, plus),
      stack_at(state, x, local_utils::add_u8(plus, 1u)));
}

} // namespace

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GTFORPNT (inclusive) .. AS_BLTU (exclusive)
// Name normalization: none (assembler label AS_GTFORPNT kept verbatim).
AS_GTFORPNTResult AS_GTFORPNT(std::uint8_t stackPointer,
                              AS_GTFORPNTState &state) {
  // TSX + 4: skip return address and caller context to first candidate frame.
  std::uint8_t x = local_utils::add_u8(stackPointer, 4u);

  while (x != 0) {
    // FRAME MARKER at AS_STACK+1,X must be AS_TOKEN_FOR ($81).
    if (stack_at(state, x, 1) == token_byte(ASToken::FOR)) {
      const std::uint16_t frameVariablePointer = stack_word_at(state, x, 2);

      // AS_NEXT with no variable: bind AS_FORPNT from current frame first.
      if (state.forpntHi == 0) {
        state.setVariablePointer(frameVariablePointer);
      }

      // Compare AS_FORPNT against frame variable pointer (hi then lo, as ROM).
      if (state.variablePointer() == frameVariablePointer) {
        return AS_GTFORPNTResult{true, x};
      }
    }

    // Advance to next frame candidate, wrapping in 8-bit space exactly as 6502.
    x = local_utils::add_u8(x, kFrameSize);
  }

  return AS_GTFORPNTResult{false, x};
}

} // namespace applesoft::asm_port
