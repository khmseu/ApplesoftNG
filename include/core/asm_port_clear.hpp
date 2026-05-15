#ifndef ASM_PORT_CLEAR_HPP
#define ASM_PORT_CLEAR_HPP

#include <cstdint>

namespace applesoft::asm_port {

/**
 * @brief "AS_CLEAR" statement logic ($066A).
 * Resets string area, variable area, array area, AS_DATA pointer, and stack.
 */
void AS_CLEAR();

/**
 * @brief Internal clearance routine ($066C).
 * AS_Like AS_CLEAR but doesn't check for end-of-statement.
 */
void AS_CLEARC();

/**
 * @brief Initialize stack and temp pointers ($0683).
 */
void AS_STKINI();

/**
 * @brief Set AS_TXTPTR to beginning of program (AS_TXTTAB - 1) ($0697).
 */
void AS_STXTPT();

/**
 * @brief "AS_RESTORE" statement logic ($0849).
 * Sets AS_DATPTR to AS_TXTTAB - 1.
 */
void AS_RESTORE();

/**
 * @brief Helper to set AS_DATPTR ($0853).
 */
void AS_SETDA(std::uint16_t addr);

} // namespace applesoft::asm_port

#endif // ASM_PORT_CLEAR_HPP
