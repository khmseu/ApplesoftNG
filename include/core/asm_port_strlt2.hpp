#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRLT2 (inclusive) .. PUTNEW (exclusive)
// Name normalization: none (assembler label STRLT2 kept verbatim).
void STRLT2(std::uint8_t a, std::uint8_t y);
void STRLT2(std::uint16_t address);

} // namespace applesoft::asm_port
