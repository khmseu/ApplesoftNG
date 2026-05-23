#include "core/asm_port_strtxt.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_strlit.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

std::uint16_t read_AS_TXTPTR() { return variables_const().AS_TXTPTR; }

bool read_carry_flag() { return variables_const().carryFlag; }

void AS_STRLIT_from_word(std::uint16_t address) { AS_STRLIT(address); }

} // namespace

// AS_Labels: AS_POINT (inclusive) .. AS_GTNUM (exclusive)
// Name normalization: none (assembler label AS_POINT kept verbatim).
void AS_POINT() {
  const std::uint16_t strng2 = variables_const().AS_STRNG2;
  variables().AS_TXTPTR = strng2;
}

namespace {} // namespace

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_STRTXT (inclusive) .. AS_NOT_ (exclusive)
// Name normalization: none (assembler label AS_STRTXT kept verbatim).
void AS_STRTXT() {
  // AS_STRING CONSTANT ELEMENT
  // SET Y,A = (AS_TXTPTR)+CARRY

  const std::uint16_t textPointer = read_AS_TXTPTR();
  const std::uint16_t stringStart = static_cast<std::uint16_t>(
      textPointer + (read_carry_flag() ? static_cast<std::uint16_t>(1)
                                       : static_cast<std::uint16_t>(0)));

  // BUILD DESCRIPTOR TO AS_STRING
  AS_STRLIT_from_word(stringStart);

  // AS_GET ADDRESS OF DESCRIPTOR IN AS_FAC; AS_POINT AS_TXTPTR AFTER TRAILING
  // QUOTE
  AS_POINT();
}

} // namespace applesoft::asm_port
