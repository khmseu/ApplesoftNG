#ifndef ASM_PORT_CLEAR_HPP
#define ASM_PORT_CLEAR_HPP

#include <cstdint>

namespace applesoft::asm_port {

/**
 * @brief "CLEAR" statement logic ($066A).
 * Resets string area, variable area, array area, DATA pointer, and stack.
 */
void CLEAR();

/**
 * @brief Internal clearance routine ($066C).
 * Like CLEAR but doesn't check for end-of-statement.
 */
void CLEARC();

/**
 * @brief Initialize stack and temp pointers ($0683).
 */
void STKINI();

/**
 * @brief Set TXTPTR to beginning of program (TXTTAB - 1) ($0697).
 */
void STXTPT();

/**
 * @brief "RESTORE" statement logic ($0849).
 * Sets DATPTR to TXTTAB - 1.
 */
void RESTORE();

/**
 * @brief Helper to set DATPTR ($0853).
 */
void SETDA(std::uint16_t addr);

} // namespace applesoft::asm_port

#endif // ASM_PORT_CLEAR_HPP
