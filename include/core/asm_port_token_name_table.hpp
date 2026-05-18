#pragma once

#include <cstddef>
#include <string_view>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_TOKEN_NAME_TABLE (inclusive) .. AS_ERROR_MESSAGES (exclusive)
// Name normalization: none (assembler label AS_TOKEN_NAME_TABLE kept verbatim).

/// AS_Look up decoded token text by index (index = token - 0x80).
/// Maps tokens $80 (END) through $EA (MID$), i.e. indices 0-106.
std::string_view AS_TOKEN_NAME_TABLE(std::size_t index);

} // namespace applesoft::asm_port
