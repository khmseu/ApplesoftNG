#include "core/asm_port_strlit.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_strlt2.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

void write_AS_CHARAC(std::uint8_t v) { variables().AS_CHARAC = v; }

void write_AS_ENDCHR(std::uint8_t v) { variables().AS_ENDCHR = v; }

} // namespace

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_STR (inclusive) .. AS_STRINI (exclusive)
// Name normalization: none (assembler label AS_STR kept verbatim).
void AS_STR() {
  // AS_STR$: expression must already be numeric.
  AS_CHKNUM();

  // Convert AS_FAC to text in the AS_STR$ transient stack buffer.
  // In ROM this starts at AS_STACK-1 ($00ff) and then branches to AS_STRLIT.
  AS_FOUT_1();
  AS_STRLIT(0x00ffu);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_STRLIT (inclusive) .. AS_STRLT2 (exclusive)
// Name normalization: none (assembler label AS_STRLIT kept verbatim).
void AS_STRLIT(std::uint16_t address) {
  // Build a literal string descriptor terminated by quote ($22) or $00.
  constexpr std::uint8_t kQuote = 0x22;

  write_AS_CHARAC(kQuote);
  write_AS_ENDCHR(kQuote);

  // Original control flow falls through directly into AS_STRLT2.
  AS_STRLT2(address);
}

} // namespace applesoft::asm_port