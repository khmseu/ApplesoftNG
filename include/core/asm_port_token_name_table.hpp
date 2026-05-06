#pragma once

#include <cstddef>
#include <string_view>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: TOKEN_NAME_TABLE (inclusive) .. ERROR_MESSAGES (exclusive)
// Name normalization: none (assembler label TOKEN_NAME_TABLE kept verbatim).

/// Look up decoded token text by index (index = token - 0x80).
/// Maps tokens $80 (END) through $EA (MID$), i.e. indices 0-106.
std::string_view TOKEN_NAME_TABLE(std::size_t index);

} // namespace applesoft::asm_port
