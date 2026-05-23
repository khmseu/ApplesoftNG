// This ROM region stores three short, null-terminated status strings used by
// the error-print path: " AS_ERROR"+bell, " IN ", and CR+"BREAK"+bell.
// The original stream includes explicit terminator bytes; this conversion
// exposes decoded payload strings by index.

#include "core/asm_port_qt_error.hpp"

#include <array>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_QT_ERROR (inclusive) .. AS_GTFORPNT (exclusive)
// Name normalization: none (assembler label AS_QT_ERROR kept verbatim).
std::string_view AS_QT_ERROR(std::size_t index) {
  static constexpr std::array<std::string_view, 3> table = {
      " AS_ERROR\a", // AS_QT_ERROR: leading space + AS_ERROR + bell ($07)
      " IN ",        // AS_QT_IN: surrounding spaces
      "\rBREAK\a",   // AS_QT_BREAK: CR prefix + BREAK + bell ($07)
  };
  return table[index];
}

} // namespace applesoft::asm_port
