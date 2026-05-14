#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHRGET ($B1) and CHRGOT ($B7)
// These routines scan the next character (CHRGET) or re-scan the current 
// character (CHRGOT) from the program text pointed to by TXTPTR.

/**
 * @brief Scan the next character from program text.
 * Increments TXTPTR, skips spaces, and sets processor flags based on char.
 * A-reg: character found.
 * Carry: Clear if character is a digit ('0'-'9'), set otherwise.
 * Zero: Set if end-of-statement (':' or null), clear otherwise.
 * 
 * @return std::uint8_t The character scanned.
 */
std::uint8_t CHRGET();

/**
 * @brief Re-scan the current character from program text.
 * Similar to CHRGET but without incrementing TXTPTR first.
 * 
 * @return std::uint8_t The character scanned.
 */
std::uint8_t CHRGOT();

} // namespace applesoft::asm_port
