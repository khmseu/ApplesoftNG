#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CHKMEM (inclusive) .. AS_REASON (exclusive)
// Name normalization: none (assembler label AS_CHKMEM kept verbatim).

struct AS_CHKMEMState {
  // Input A register on entry. The ROM doubles this requirement and adds 54
  // bytes.
  std::uint8_t a = 0;

  // Zero-page AS_INDEX written by AS_CHKMEM.
  std::uint8_t index = 0;

  // Incoming 6502 stack pointer consumed by TSX.
  std::uint8_t stackPointer = 0;

  // X register state after TSX or AS_MEMERR.
  std::uint8_t x = 0;

  // Indicates that AS_CHKMEM branched to AS_MEMERR instead of returning
  // normally.
  bool memerrCalled = false;
};

struct AS_CHKMEMResult {
  bool ok = false;
  std::uint8_t a = 0;
  std::uint8_t x = 0;
};

// Check whether the current stack pointer leaves enough room for runtime
// frames.
AS_CHKMEMResult AS_CHKMEM(AS_CHKMEMState &state);

} // namespace applesoft::asm_port