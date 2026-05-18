#pragma once

#include <cstddef>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_UNFNC (inclusive) .. AS_MATHTBL (exclusive)
// Name normalization: none (assembler label AS_UNFNC kept verbatim).

/// Common signature for all Applesoft unary/built-in function handlers.
/// Handlers operate on global state (AS_FAC, AS_ARG, etc.) and return void.
using AS_UNFNC_fn = void (*)();

/// AS_Look up the unary/built-in function handler for the given index
/// (index = token - 0xD2).
/// Maps tokens $D2 (AS_SGN) through $EA (MID$), i.e. indices 0–24.
/// Unlike AS_TOKEN_ADDRESS_TABLE, AS_UNFNC entries are direct addresses (no
/// -1). The caller is responsible for invoking the returned pointer.
AS_UNFNC_fn AS_UNFNC(std::size_t index);

// AS_Labels: AS_VAL (inclusive) .. AS_L_VAL_1 (exclusive)
void AS_VAL();

} // namespace applesoft::asm_port
