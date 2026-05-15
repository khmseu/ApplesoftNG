#pragma once

#include <cstddef>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_TOKEN_ADDRESS_TABLE (inclusive) .. AS_UNFNC (exclusive)
// Name normalization: none (assembler label AS_TOKEN_ADDRESS_TABLE kept verbatim).

/// Common signature for all Applesoft statement handler functions.
using AS_TOKEN_ADDRESS_TABLE_fn = void(*)();

/// AS_Look up the statement handler for the given index (index = token - 0x80).
/// The caller is responsible for invoking the returned pointer.
/// Maps tokens $80 (END) through $BF (AS_NEW), i.e. indices 0–63.
AS_TOKEN_ADDRESS_TABLE_fn AS_TOKEN_ADDRESS_TABLE(std::size_t index);

} // namespace applesoft::asm_port
