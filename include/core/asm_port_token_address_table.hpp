#pragma once

#include <cstddef>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: TOKEN_ADDRESS_TABLE (inclusive) .. UNFNC (exclusive)
// Name normalization: none (assembler label TOKEN_ADDRESS_TABLE kept verbatim).

/// Common signature for all Applesoft statement handler functions.
using TOKEN_ADDRESS_TABLE_fn = void(*)();

/// Look up the statement handler for the given index (index = token - 0x80).
/// The caller is responsible for invoking the returned pointer.
/// Maps tokens $80 (END) through $BF (NEW), i.e. indices 0–63.
TOKEN_ADDRESS_TABLE_fn TOKEN_ADDRESS_TABLE(std::size_t index);

} // namespace applesoft::asm_port
