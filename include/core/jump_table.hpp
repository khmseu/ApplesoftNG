#pragma once
#include <cstdint>

namespace ApplesoftNG {

/**
 * @brief Dispatcher for jumps to unported or external ROM addresses.
 *
 * This class centralizes the mapping between 6502 absolute addresses
 * and their corresponding C++ implementations.
 */
class ExternalJumpDispatcher {
public:
  /**
   * @brief Executes a jump to the specified 16-bit address.
   *
   * @param address The 6502 absolute address to jump to.
   */
  static void Jump(std::uint16_t address);

  // Well-known monitor entry points
  static constexpr std::uint16_t ADDR_MON_KEYIN = 0xFD1Bu;
  static constexpr std::uint16_t ADDR_MON_COUT1 = 0xFDF0u;
  static constexpr std::uint16_t ADDR_MON_GETLN = 0xFD6Au;
  static constexpr std::uint16_t ADDR_MON_PRBYTE = 0xFDDau;
  static constexpr std::uint16_t ADDR_MON_IOREST = 0xFF3Fu;
  static constexpr std::uint16_t ADDR_MON_APPLE2_RESET =
      0xFA62u; // One possible RESET entry
};

} // namespace ApplesoftNG
