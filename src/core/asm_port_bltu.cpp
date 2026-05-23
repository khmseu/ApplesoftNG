#include "core/asm_port_bltu.hpp"
#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {
namespace {

void copy_page_backward(AS_BLTU2State &state) {
  const std::uint16_t source = state.sourcePointer();
  const std::uint16_t destination = state.destinationPointer();

  do {
    state.memory[destination + state.y] = state.memory[source + state.y];
    --state.y;
  } while (state.y != 0);

  state.memory[destination] = state.memory[source];
}

} // namespace

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_BLTU (inclusive) .. AS_BLTU2 (exclusive)
// Name normalization: none (assembler label AS_BLTU kept verbatim).
AS_BLTUResult AS_BLTU(AS_BLTUState &state) {
  // AS_BLTU starts by validating/grooming destination via AS_REASON.
  state.reason.a = state.a;
  state.reason.y = state.y;
  const AS_REASONResult reasonResult = AS_REASON(state.reason);
  state.a = reasonResult.a;
  state.y = reasonResult.y;

  // New top of array storage (AS_STREND) is loaded from adjusted A/Y.
  state.setStrend(ApplesoftVariables::makeWord(state.a, state.y));

  // Execution falls through to AS_BLTU2 in the original ROM.
  return AS_BLTUResult{state.a, state.y};
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_BLTU2 (inclusive) .. AS_CHKMEM (exclusive)
// Name normalization: none (assembler label AS_BLTU2 kept verbatim).
AS_BLTU2Result AS_BLTU2(AS_BLTU2State &state) {
  if (state.y != 0) {
    state.setSourcePointer(
        static_cast<std::uint16_t>(state.sourcePointer() - state.y));
    state.setDestinationPointer(
        static_cast<std::uint16_t>(state.destinationPointer() - state.y));
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

  return AS_BLTU2Result{state.x, state.y};
}

} // namespace applesoft::asm_port
