#pragma once

#include "core/applesoft_dual_pointer.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

/// AS_Look up decoded token text by index (index = token - 0x80).
/// Maps tokens $80 (END) through $EA (MID$), i.e. indices 0-106.
std::string_view AS_TOKEN_NAME_TABLE(std::size_t index);

/// Canonical label API for AS_TOKEN_NAME_TABLE lookup.
ApplesoftDualPointer<const std::uint8_t>
AS_TOKEN_NAME_TABLE_ptr(std::size_t index);

/// Decode token text from canonical dual-pointer input.
std::string_view
AS_TOKEN_NAME_TABLE(ApplesoftDualPointer<const std::uint8_t> token_ptr);

} // namespace applesoft::asm_port
