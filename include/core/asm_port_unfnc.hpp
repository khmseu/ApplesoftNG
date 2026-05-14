#pragma once

#include <cstddef>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: UNFNC (inclusive) .. MATHTBL (exclusive)
// Name normalization: none (assembler label UNFNC kept verbatim).

/// Common signature for all Applesoft unary/built-in function handlers.
/// Handlers operate on global state (FAC, ARG, etc.) and return void.
using UNFNC_fn = void(*)();

/// Look up the unary/built-in function handler for the given index
/// (index = token - 0xD2).
/// Maps tokens $D2 (SGN) through $EA (MID$), i.e. indices 0–24.
/// Unlike TOKEN_ADDRESS_TABLE, UNFNC entries are direct addresses (no -1).
/// The caller is responsible for invoking the returned pointer.
UNFNC_fn UNFNC(std::size_t index);

// Labels: VAL (inclusive) .. L_VAL_1 (exclusive)
void VAL();

} // namespace applesoft::asm_port
