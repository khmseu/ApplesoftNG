#pragma once

#include <cstddef>
#include <string_view>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: QT_ERROR (inclusive) .. GTFORPNT (exclusive)
// Name normalization: none (assembler label QT_ERROR kept verbatim).

/// Index constants for the short quoted-status strings block.
inline constexpr std::size_t QT_ERROR_INDEX = 0; // " ERROR" + bell
inline constexpr std::size_t QT_IN_INDEX    = 1; // " IN "
inline constexpr std::size_t QT_BREAK_INDEX = 2; // CR + "BREAK" + bell

/// Return one decoded quoted-status string from the contiguous QT block.
/// Caller chooses entry by index: 0=QT_ERROR, 1=QT_IN, 2=QT_BREAK.
std::string_view QT_ERROR(std::size_t index);

} // namespace applesoft::asm_port
