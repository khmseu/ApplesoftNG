#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst

// FREFAC: dereference the string descriptor at FAC[3]/FAC[4], optionally
// release the temporary descriptor, store the data pointer in INDEX, and
// return the string length.
// TODO(asm-port): FRETMS (temporary-release part) not yet implemented.
 std::uint8_t FREFAC() ;


// Labels: STRPRT (inclusive) .. OUTSP (exclusive)
// Name normalization: none (assembler label STRPRT kept verbatim).
// Prints the string whose descriptor is in FAC[3]/FAC[4] by calling FREFAC to
// obtain the data pointer (stored in INDEX) and length, then calls OUTDO for
// each character.
void STRPRT();

// Labels: STROUT (inclusive) .. STRPRT (exclusive)
// Name normalization: none (assembler label STROUT kept verbatim).
// Makes the string at address (a=lo, y=hi) into a FAC descriptor via STRLIT,
// then falls through to STRPRT.
void STROUT(std::uint8_t a, std::uint8_t y);

// Labels: PR_STRING (inclusive) .. PRINT (exclusive)
// Name normalization: none (assembler label PR_STRING kept verbatim).
// Prints the current FAC string via STRPRT, fetches the next character via
// CHRGOT, and re-enters the PRINT list loop.  Falls through to PRINT in the
// original; modeled by delegating to PRINT after the end-of-list check.
void PR_STRING();

// Labels: PRINT (inclusive) .. PRINT2 (exclusive)
// Name normalization: none (assembler label PRINT kept verbatim).
// Entry point for the PRINT statement.  If a==0 (end of list) prints a
// carriage return; otherwise processes the print list with end-of-list CR.
void PRINT(std::uint8_t a);

// Labels: PRINT2 (inclusive) .. CRDO (exclusive)
// Name normalization: none (assembler label PRINT2 kept verbatim).
// Re-entry point after a separator (,;).  If a==0 returns without printing a
// carriage return; otherwise processes the remaining print list.
void PRINT2(std::uint8_t a);

// Labels: CRDO (inclusive) .. NEGATE (exclusive)
// Name normalization: none (assembler label CRDO kept verbatim).
// Prints a carriage-return character ($0d) via OUTDO and falls through to
// NEGATE (modeled as a call in C++).
void CRDO();

// Labels: NEGATE (inclusive) .. PR_COMMA (exclusive)
// Name normalization: none (assembler label NEGATE kept verbatim).
// Returns a ^ $ff.  In the original source this is reachable as a fall-through
// from CRDO and is commented "<<< WHY??? >>>".
std::uint8_t NEGATE(std::uint8_t a);

// Labels: PR_COMMA (inclusive) .. PR_TAB_OR_SPC (exclusive)
// Name normalization: none (assembler label PR_COMMA kept verbatim).
// Advances the print position to the next comma zone (columns 0, 16, 32).
// If the current column is >= 24 a new line is printed first.
// Falls through to PR_NEXT_CHAR (handled by the PRINT2 loop caller).
void PR_COMMA();

// Labels: PR_TAB_OR_SPC (inclusive) .. STROUT (exclusive)
// Name normalization: none (assembler label PR_TAB_OR_SPC kept verbatim).
// Handles TAB(n) and SPC(n) in a PRINT list.
// is_tab=true  → TAB(: moves to column n (prints n-MON_CH spaces if past).
// is_tab=false → SPC(: prints exactly n spaces.
// Falls through to PR_NEXT_CHAR (handled by the PRINT2 loop caller).
void PR_TAB_OR_SPC(bool is_tab);

} // namespace applesoft::asm_port
