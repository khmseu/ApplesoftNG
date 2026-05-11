#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STR (inclusive) .. STRINI (exclusive)
// Name normalization: none (assembler label STR kept verbatim).
void STR();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRLIT (inclusive) .. STRLT2 (exclusive)
// Name normalization: none (assembler label STRLIT kept verbatim).
void STRLIT(std::uint16_t address);

} // namespace applesoft::asm_port