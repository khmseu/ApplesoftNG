#pragma once

#include <cstdint>

namespace applesoft::asm_port {

struct Inlin2Result {
    std::uint8_t a;
    std::uint8_t x;
    std::uint8_t y;
};

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INLIN2 (inclusive) .. INCHR (exclusive)
// Name normalization: none (assembler label INLIN2 kept verbatim).
Inlin2Result INLIN2(std::uint8_t x);

} // namespace applesoft::asm_port