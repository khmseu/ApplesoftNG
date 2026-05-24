#pragma once

#include "core/applesoft_dual_pointer.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

/// Index constants for the short quoted-status strings block.
inline constexpr std::size_t AS_QT_ERROR_INDEX = 0; // " AS_ERROR" + bell
inline constexpr std::size_t AS_QT_IN_INDEX = 1;    // " IN "
inline constexpr std::size_t AS_QT_BREAK_INDEX = 2; // CR + "BREAK" + bell

/// Return one decoded quoted-status string from the contiguous QT block.
/// Caller chooses entry by index: 0=AS_QT_ERROR, 1=AS_QT_IN, 2=AS_QT_BREAK.
std::string_view AS_QT_ERROR(std::size_t index);

/// Canonical label API for AS_QT_ERROR block lookup.
ApplesoftDualPointer<const std::uint8_t> AS_QT_ERROR_ptr(std::size_t index);

/// Decode quoted-status text from canonical dual-pointer input.
std::string_view AS_QT_ERROR(ApplesoftDualPointer<const std::uint8_t> text_ptr);

} // namespace applesoft::asm_port
