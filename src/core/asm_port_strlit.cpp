#include "core/asm_port_strlit.hpp"

#include "core/asm_port_strlt2.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

// TODO(asm-port): map CHARAC register-backed variable.
void write_CHARAC(std::uint8_t /*v*/) {}

// TODO(asm-port): map ENDCHR register-backed variable.
void write_ENDCHR(std::uint8_t /*v*/) {}

} // namespace

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRLIT (inclusive) .. STRLT2 (exclusive)
// Name normalization: none (assembler label STRLIT kept verbatim).
void STRLIT(std::uint8_t a, std::uint8_t y) {
    // Build a literal string descriptor terminated by quote ($22) or $00.
    constexpr std::uint8_t kQuote = 0x22;

    write_CHARAC(kQuote);
    write_ENDCHR(kQuote);

    // Original control flow falls through directly into STRLT2.
    STRLT2(a, y);
}

} // namespace applesoft::asm_port