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
   * @tparam R The return type of the called function.
   * @tparam Args The types of the parameters passed to the called function.
   * @param address The 6502 absolute address to jump to.
   * @param args The parameters to forward to the implementation.
   * @return The result of the called function.
   */
  template <typename R = void, typename... Args>
  static R Jump(std::uint16_t address, Args &&...args);

  // Well-known monitor entry points
  static constexpr std::uint16_t ADDR_MON_KEYIN = 0xFD1Bu;
  static constexpr std::uint16_t ADDR_MON_COUT1 = 0xFDF0u;
  static constexpr std::uint16_t ADDR_MON_GETLN = 0xFD6Au;
  static constexpr std::uint16_t ADDR_MON_PRBYTE = 0xFDDau;
  static constexpr std::uint16_t ADDR_MON_IOREST = 0xFF3Fu;
  static constexpr std::uint16_t ADDR_MON_APPLE2_RESET =
      0xFA62u; // One possible RESET entry
};

/**
 * @brief Actual implementations for ExternalJumpDispatcher::Jump.
 * These are moved to the header to facilitate template instantiation.
 */
} // namespace ApplesoftNG

#include "core/applesoft_variables.hpp"
#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>

namespace applesoft::asm_port {
void AS_COLD_START();
std::uint8_t MON_GETLN();
std::uint8_t MON_KEYIN();
void MON_COUT1(std::uint8_t a);
} // namespace applesoft::asm_port

namespace ApplesoftNG {

template <typename R, typename... Args>
R ExternalJumpDispatcher::Jump(std::uint16_t address, Args &&...args) {
  using namespace applesoft::asm_port;

  auto invoke = [&]<typename Func>(Func &&f) -> R {
    if constexpr (std::is_invocable_r_v<R, Func, Args...>) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(std::forward<Func>(f), std::forward<Args>(args)...);
      } else {
        return std::invoke(std::forward<Func>(f), std::forward<Args>(args)...);
      }
    } else {
      // Incompatible signature requested for this address
      throw std::runtime_error("ExternalJumpDispatcher: Incompatible signature "
                               "for address " +
                               std::to_string(address));
    }
  };

  switch (address) {
  case 0xE000u:
    return invoke(AS_COLD_START);

  case ADDR_MON_KEYIN:
    return invoke(MON_KEYIN);

  case ADDR_MON_GETLN:
    return invoke(MON_GETLN);

  case ADDR_MON_COUT1:
    return invoke(MON_COUT1);

  default:
    // RESET slot scan (Cx00) fallback
    if ((address & 0xFF00u) >= 0xC100u && (address & 0xFF00u) <= 0xC700u) {
      return invoke(AS_COLD_START);
    }
    break;
  }

  if constexpr (std::is_void_v<R>) {
    return;
  } else {
    throw std::runtime_error("ExternalJumpDispatcher: Unhandled jump address " +
                             std::to_string(address));
  }
}

} // namespace ApplesoftNG
