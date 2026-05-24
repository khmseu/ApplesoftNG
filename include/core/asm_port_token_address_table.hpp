#pragma once

#include "core/applesoft_dual_pointer.hpp"

#include <cstddef>

namespace applesoft::asm_port {

/// Common signature for all Applesoft statement handler functions.
using AS_TOKEN_ADDRESS_TABLE_fn = void (*)();

/// AS_Look up the statement handler for the given index (index = token - 0x80).
/// The caller is responsible for invoking the returned pointer.
/// Maps tokens $80 (END) through $BF (AS_NEW), i.e. indices 0–63.
AS_TOKEN_ADDRESS_TABLE_fn AS_TOKEN_ADDRESS_TABLE(std::size_t index);

/// Canonical label API for AS_TOKEN_ADDRESS_TABLE lookup.
ApplesoftDualPointer<const AS_TOKEN_ADDRESS_TABLE_fn>
AS_TOKEN_ADDRESS_TABLE_ptr(std::size_t index);

/// Decode statement handler function pointer from canonical dual-pointer input.
AS_TOKEN_ADDRESS_TABLE_fn AS_TOKEN_ADDRESS_TABLE(
    ApplesoftDualPointer<const AS_TOKEN_ADDRESS_TABLE_fn> handler_ptr);

void AS_AMPERSAND();

} // namespace applesoft::asm_port
