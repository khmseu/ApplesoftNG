#pragma once

#include <cstdint>

namespace applesoft::asm_port {

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
