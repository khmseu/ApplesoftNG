#include "core/asm_port_strlit.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_strlt2.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

void write_CHARAC(std::uint8_t v) {
    variables().writeByte(0x000du, v);
}

void write_ENDCHR(std::uint8_t v) {
    variables().writeByte(0x000eu, v);
}

} // namespace

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRLIT (inclusive) .. STRLT2 (exclusive)
// Name normalization: none (assembler label STRLIT kept verbatim).
void STRLIT(std::uint16_t address) {
    // Build a literal string descriptor terminated by quote ($22) or $00.
    constexpr std::uint8_t kQuote = 0x22;

    write_CHARAC(kQuote);
    write_ENDCHR(kQuote);

    // Original control flow falls through directly into STRLT2.
    STRLT2(address);
}

void STRLIT(std::uint8_t a, std::uint8_t y) {
    STRLIT(ApplesoftVariables::makeWord(a, y));
}

} // namespace applesoft::asm_port