#pragma once

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst

// AS_FREFAC: dereference the string descriptor at AS_FAC[3]/AS_FAC[4],
// optionally release the temporary descriptor, store the data pointer in
// AS_INDEX, and return the string length.
// TODO(asm-port): AS_FRETMS (temporary-release part) not yet implemented.
std::uint8_t AS_FREFAC();

// AS_Labels: AS_STRPRT (inclusive) .. AS_OUTSP (exclusive)
// Name normalization: none (assembler label AS_STRPRT kept verbatim).
// Prints the string whose descriptor is in AS_FAC[3]/AS_FAC[4] by calling
// AS_FREFAC to obtain the data pointer (stored in AS_INDEX) and length, then
// calls AS_OUTDO for each character.
void AS_STRPRT();

// AS_Labels: AS_STROUT (inclusive) .. AS_STRPRT (exclusive)
// Name normalization: none (assembler label AS_STROUT kept verbatim).
// Makes the string at address (a=lo, y=hi) into a AS_FAC descriptor via
// AS_STRLIT, then falls through to AS_STRPRT.
void AS_STROUT(std::uint16_t address);

// Helper for direct string literal output used by error/input paths.
void AS_STROUT(std::string_view text);

// AS_Labels: AS_PR_STRING (inclusive) .. AS_PRINT (exclusive)
// Name normalization: none (assembler label AS_PR_STRING kept verbatim).
// Prints the current AS_FAC string via AS_STRPRT, fetches the next character
// via AS_CHRGOT, and re-enters the AS_PRINT list loop.  Falls through to
// AS_PRINT in the original; modeled by delegating to AS_PRINT after the
// end-of-list check.
void AS_PR_STRING();

// AS_Labels: AS_PRINT (inclusive) .. AS_PRINT2 (exclusive)
// Name normalization: none (assembler label AS_PRINT kept verbatim).
// Entry point for the AS_PRINT statement.  If a==0 (end of list) prints a
// carriage return; otherwise processes the print list with end-of-list CR.
void AS_PRINT(std::uint8_t a);

// AS_Labels: AS_PRINT2 (inclusive) .. AS_CRDO (exclusive)
// Name normalization: none (assembler label AS_PRINT2 kept verbatim).
// Re-entry point after a separator (,;).  If a==0 returns without printing a
// carriage return; otherwise processes the remaining print list.
void AS_PRINT2(std::uint8_t a);

// AS_Labels: AS_CRDO (inclusive) .. AS_NEGATE (exclusive)
// Name normalization: none (assembler label AS_CRDO kept verbatim).
// Prints a carriage-return character ($0d) via AS_OUTDO and falls through to
// AS_NEGATE (modeled as a call in C++).
void AS_CRDO();

// AS_Labels: AS_NEGATE (inclusive) .. AS_PR_COMMA (exclusive)
// Name normalization: none (assembler label AS_NEGATE kept verbatim).
// Returns a ^ $ff.  In the original source this is reachable as a fall-through
// from AS_CRDO and is commented "<<< WHY??? >>>".
std::uint8_t AS_NEGATE(std::uint8_t a);

// AS_Labels: AS_PR_COMMA (inclusive) .. AS_PR_TAB_OR_SPC (exclusive)
// Name normalization: none (assembler label AS_PR_COMMA kept verbatim).
// Advances the print position to the next comma zone (columns 0, 16, 32).
// If the current column is >= 24 a new line is printed first.
// Falls through to AS_PR_NEXT_CHAR (handled by the AS_PRINT2 loop caller).
void AS_PR_COMMA();

// AS_Labels: AS_PR_TAB_OR_SPC (inclusive) .. AS_STROUT (exclusive)
// Name normalization: none (assembler label AS_PR_TAB_OR_SPC kept verbatim).
// Handles TAB(n) and SPC(n) in a AS_PRINT list.
// is_tab=true  → TAB(: moves to column n (prints n-MON_CH spaces if past).
// is_tab=false → SPC(: prints exactly n spaces.
// Falls through to AS_PR_NEXT_CHAR (handled by the AS_PRINT2 loop caller).
void AS_PR_TAB_OR_SPC(bool is_tab);

} // namespace applesoft::asm_port
