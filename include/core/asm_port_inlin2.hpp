#pragma once

#include <cstdint>

#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {

struct Inlin2Result {
  std::uint8_t a;
  std::uint8_t x;
  std::uint8_t y;

  static Inlin2Result fromAddress(std::uint8_t aValue, std::uint16_t address) {
    return Inlin2Result{aValue, static_cast<std::uint8_t>(address & 0x00ffu),
                        static_cast<std::uint8_t>(address >> 8)};
  }

  std::uint16_t address() const { return ApplesoftVariables::makeWord(x, y); }
};

// MON_RDKEY is a monitor alias for RDKEY.
std::uint8_t MON_RDKEY();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INLIN2 (inclusive) .. AS_INCHR (exclusive)
// Name normalization: none (assembler label AS_INLIN2 kept verbatim).
Inlin2Result AS_INLIN2(std::uint8_t x);

// AS_Labels: AS_INCHR (inclusive) .. AS_PARSE_INPUT_LINE (exclusive)
// Name normalization: none (assembler label AS_INCHR kept verbatim).
std::uint8_t AS_INCHR();

// MON_KEYIN - $fd1b
// Returns the character in A.
std::uint8_t MON_KEYIN();

} // namespace applesoft::asm_port