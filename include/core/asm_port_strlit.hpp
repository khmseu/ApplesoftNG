#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_STR (inclusive) .. AS_STRINI (exclusive)
// Name normalization: none (assembler label AS_STR kept verbatim).
void AS_STR();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_STRLIT (inclusive) .. AS_STRLT2 (exclusive)
// Name normalization: none (assembler label AS_STRLIT kept verbatim).
void AS_STRLIT(std::uint16_t address);

} // namespace applesoft::asm_port