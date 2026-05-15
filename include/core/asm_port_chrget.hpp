#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHRGET ($B1) and AS_CHRGOT ($B7)
// These routines scan the next character (AS_CHRGET) or re-scan the current 
// character (AS_CHRGOT) from the program text pointed to by AS_TXTPTR.

/**
 * @brief Scan the next character from program text.
 * Increments AS_TXTPTR, skips spaces, and sets processor flags based on char.
 * A-reg: character found.
 * Carry: Clear if character is a digit ('0'-'9'), set otherwise.
 * Zero: Set if end-of-statement (':' or null), clear otherwise.
 * 
 * @return std::uint8_t The character scanned.
 */
std::uint8_t AS_CHRGET();

/**
 * @brief Re-scan the current character from program text.
 * Similar to AS_CHRGET but without incrementing AS_TXTPTR first.
 * 
 * @return std::uint8_t The character scanned.
 */
std::uint8_t AS_CHRGOT();

} // namespace applesoft::asm_port
