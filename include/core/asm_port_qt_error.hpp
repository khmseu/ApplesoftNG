#pragma once

#include <cstddef>
#include <string_view>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_QT_ERROR (inclusive) .. AS_GTFORPNT (exclusive)
// Name normalization: none (assembler label AS_QT_ERROR kept verbatim).

/// Index constants for the short quoted-status strings block.
inline constexpr std::size_t AS_QT_ERROR_INDEX = 0; // " AS_ERROR" + bell
inline constexpr std::size_t AS_QT_IN_INDEX = 1;    // " IN "
inline constexpr std::size_t AS_QT_BREAK_INDEX = 2; // CR + "BREAK" + bell

/// Return one decoded quoted-status string from the contiguous QT block.
/// Caller chooses entry by index: 0=AS_QT_ERROR, 1=AS_QT_IN, 2=AS_QT_BREAK.
std::string_view AS_QT_ERROR(std::size_t index);

} // namespace applesoft::asm_port
