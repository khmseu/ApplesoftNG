#pragma once

#include <cstdint>

namespace applesoft::asm_port {

struct Inlin2Result {
    std::uint8_t a;
    std::uint8_t x;
    std::uint8_t y;

    std::uint16_t address() const {
        return static_cast<std::uint16_t>(static_cast<std::uint16_t>(y) << 8 | x);
    }
};


// MON_RDKEY is a monitor alias for RDKEY.
std::uint8_t MON_RDKEY() ;



// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INLIN2 (inclusive) .. INCHR (exclusive)
// Name normalization: none (assembler label INLIN2 kept verbatim).
Inlin2Result INLIN2(std::uint8_t x);

// Labels: INCHR (inclusive) .. PARSE_INPUT_LINE (exclusive)
// Name normalization: none (assembler label INCHR kept verbatim).
std::uint8_t INCHR();

} // namespace applesoft::asm_port