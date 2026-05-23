#pragma once

#include <array>
#include <cstdint>

#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {

inline constexpr std::uint8_t AS_TOKEN_FOR = 0x81;

// Minimal state needed by AS_GTFORPNT while broader runtime memory wiring is
// pending.
struct AS_GTFORPNTState {
  // 6502 stack page: logical addresses $0100..$01FF map to indices 0..255.
  std::array<std::uint8_t, 256> stackPage{};

  // Zero-page AS_FORPNT ($85/$86): variable address to search for.
  std::uint8_t forpntAS_Lo = 0;
  std::uint8_t forpntHi = 0;

  std::uint16_t variablePointer() const {
    return ApplesoftVariables::makeWord(forpntAS_Lo, forpntHi);
  }

  void setVariablePointer(std::uint16_t value) {
    forpntAS_Lo = static_cast<std::uint8_t>(value & 0x00ffu);
    forpntHi = static_cast<std::uint8_t>(value >> 8);
  }
};

struct AS_GTFORPNTResult {
  bool found = false;

  // Final X register value on return (stack frame pointer examined/found).
  std::uint8_t x = 0;
};

// Scan AS_FOR frames on the 6502 stack for a variable pointer match.
// `stackPointer` is the incoming 6502 SP before `TSX`.
// Side effect: when called from AS_NEXT with no variable (`forpntHi == 0`),
// AS_FORPNT is loaded from the first AS_FOR frame encountered (matching ROM
// behavior).
AS_GTFORPNTResult AS_GTFORPNT(std::uint8_t stackPointer,
                              AS_GTFORPNTState &state);

} // namespace applesoft::asm_port
