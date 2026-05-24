// This ROM region stores three short, null-terminated status strings used by
// the error-print path: " AS_ERROR"+bell, " IN ", and CR+"BREAK"+bell.
// The original stream includes explicit terminator bytes; this conversion
// exposes decoded payload strings by index.

#include "core/asm_port_qt_error.hpp"

#include <array>
#include <cstring>

namespace {

using applesoft::asm_port::ApplesoftDualPointer;

constexpr std::array<const char *, 3> kQtErrorTable = {
    " AS_ERROR\a", // AS_QT_ERROR: leading space + AS_ERROR + bell ($07)
    " IN ",        // AS_QT_IN: surrounding spaces
    "\rBREAK\a",   // AS_QT_BREAK: CR prefix + BREAK + bell ($07)
};

constexpr char kEmptyQtError[] = "";

} // namespace

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_QT_ERROR (inclusive) .. AS_GTFORPNT (exclusive)
// Name normalization: minimal suffix required for C++ overload disambiguation.
ApplesoftDualPointer<const std::uint8_t> AS_QT_ERROR_ptr(std::size_t index) {
  if (index >= kQtErrorTable.size()) {
    return ApplesoftDualPointer<const std::uint8_t>::native(
        reinterpret_cast<const std::uint8_t *>(kEmptyQtError));
  }

  return ApplesoftDualPointer<const std::uint8_t>::native(
      reinterpret_cast<const std::uint8_t *>(kQtErrorTable[index]));
}

std::string_view
AS_QT_ERROR(ApplesoftDualPointer<const std::uint8_t> text_ptr) {
  if (!text_ptr.isNative()) {
    return {};
  }

  const std::uint8_t *bytes = text_ptr.nativePointer();
  if (bytes == nullptr || bytes[0] == 0u) {
    return {};
  }

  const char *text = reinterpret_cast<const char *>(bytes);
  return std::string_view(text, std::strlen(text));
}

std::string_view AS_QT_ERROR(std::size_t index) {
  return AS_QT_ERROR(AS_QT_ERROR_ptr(index));
}

} // namespace applesoft::asm_port
