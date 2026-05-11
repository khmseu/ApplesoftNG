#pragma once

#include <cstdint>

#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {

struct Inlin2Result {
    std::uint8_t a;
    std::uint8_t x;
    std::uint8_t y;

    static Inlin2Result fromAddress(std::uint8_t aValue, std::uint16_t address) {
        return Inlin2Result{
            aValue,
            static_cast<std::uint8_t>(address & 0x00ffu),
            static_cast<std::uint8_t>(address >> 8)
        };
    }

    std::uint16_t address() const {
        return ApplesoftVariables::makeWord(x, y);
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