#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst

// Labels: OUTDO (inclusive) .. INPUTERR (exclusive)
// Name normalization: none (assembler label OUTDO kept verbatim).
// OUTDO applies Apple II display encoding (set high bit, optionally apply
// FLASH_BIT for non-control characters), delegates to MON_COUT ($fded), then
// performs the MON_WAIT ($fca8) speed delay.
// Returns the output character with the high bit cleared (A & $7f).
std::uint8_t OUTDO(std::uint8_t a);

// Labels: OUTSP (inclusive) .. OUTQUES (exclusive)
// Name normalization: none.
// Prints a space ($20).  In the 6502 original a .byt $2c trick causes OUTSP
// to bypass OUTQUES's own load and fall directly to OUTDO with A=$20; in C++
// OUTDO is called directly with the space character.
void OUTSP();

// Labels: OUTQUES (inclusive) .. OUTDO (exclusive)
// Name normalization: none.
// Prints a question-mark ($3f) by falling through to OUTDO.
void OUTQUES();

} // namespace applesoft::asm_port
