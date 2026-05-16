#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst

// AS_Labels: AS_OUTDO (inclusive) .. AS_INPUTERR (exclusive)
// Name normalization: none (assembler label AS_OUTDO kept verbatim).
// AS_OUTDO applies Apple II display encoding (set high bit, optionally apply
// AS_FLASH_BIT for non-control characters), delegates to MON_COUT ($fded), then
// performs the MON_WAIT ($fca8) speed delay.
// Returns the output character with the high bit cleared (A & $7f).
std::uint8_t AS_OUTDO(std::uint8_t a);

// Monitor output routines
void MON_COUT(std::uint8_t a);
void MON_COUT1(std::uint8_t a);

// AS_Labels: AS_OUTSP (inclusive) .. AS_OUTQUES (exclusive)
// Name normalization: none.
// Prints a space ($20).  In the 6502 original a .byt $2c trick causes AS_OUTSP
// to bypass AS_OUTQUES's own load and fall directly to AS_OUTDO with A=$20; in
// C++ AS_OUTDO is called directly with the space character.
void AS_OUTSP();

// AS_Labels: AS_OUTQUES (inclusive) .. AS_OUTDO (exclusive)
// Name normalization: none.
// Prints a question-mark ($3f) by falling through to AS_OUTDO.
void AS_OUTQUES();

} // namespace applesoft::asm_port
