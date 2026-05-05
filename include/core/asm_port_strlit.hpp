#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRLIT (inclusive) .. STRLT2 (exclusive)
// Name normalization: none (assembler label STRLIT kept verbatim).
void STRLIT(std::uint8_t a, std::uint8_t y);

} // namespace applesoft::asm_port