// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: QT_ERROR (inclusive) .. GTFORPNT (exclusive)
// Name normalization: none
//
// This ROM region stores three short, null-terminated status strings used by
// the error-print path: " ERROR"+bell, " IN ", and CR+"BREAK"+bell.
// The original stream includes explicit terminator bytes; this conversion
// exposes decoded payload strings by index.

#include "core/asm_port_qt_error.hpp"

#include <array>

namespace applesoft::asm_port {

std::string_view QT_ERROR(std::size_t index) {
    static constexpr std::array<std::string_view, 3> table = {
        " ERROR\a",   // QT_ERROR: leading space + ERROR + bell ($07)
        " IN ",       // QT_IN: surrounding spaces
        "\rBREAK\a", // QT_BREAK: CR prefix + BREAK + bell ($07)
    };
    return table[index];
}

} // namespace applesoft::asm_port
