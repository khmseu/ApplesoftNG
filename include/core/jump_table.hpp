#pragma once
#include "core/applesoft_variables.hpp"
#include "core/asm_port_core.hpp"
#include "core/asm_port_token_address_table.hpp"
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
  static R JumpTo(std::uint16_t address, Args &&...args);

  /**
   * @brief Executes a jump by reading the destination from a memory word.
   */
  template <typename R = void, typename... Args>
  static R JumpFromWord(std::uint16_t wordAddress, Args &&...args);

  /**
   * @brief Executes a jump by reading the destination from a JMP instruction.
   */
  template <typename R = void, typename... Args>
  static R JumpFromInstruction(std::uint16_t jumpAddress, Args &&...args);

  // Well-known monitor entry points
  static constexpr std::uint16_t ADDR_AS_USR =
      applesoft::asm_port::ApplesoftVariables::ZP_AS_USR;
  static constexpr std::uint16_t ADDR_AS_AMPERSAND = 0x03f5u;
  static constexpr std::uint16_t ADDR_MON_KEYIN = 0xFD1Bu;
  static constexpr std::uint16_t ADDR_MON_OLDBRK = 0xFA59u;
  static constexpr std::uint16_t ADDR_MON_COUT1 = 0xFDF0u;
  static constexpr std::uint16_t ADDR_AS_BASIC = 0xE000u;
  static constexpr std::uint16_t ADDR_AS_BASIC2 = 0xE003u;
};

/**
 * @brief Actual implementations for ExternalJumpDispatcher::Jump.
 * These are moved to the header to facilitate template instantiation.
 */
} // namespace ApplesoftNG

#include <functional>
#include <stdexcept>
#include <string>

namespace applesoft::asm_port {
void AS_BASIC();
void AS_BASIC2();
void AS_USR();
void AS_USR_impl();
std::uint8_t MON_GETLN();
std::uint8_t MON_KEYIN();
void MON_COUT1(std::uint8_t a);
} // namespace applesoft::asm_port

namespace ApplesoftNG {

template <typename R, typename... Args>
R ExternalJumpDispatcher::JumpTo(std::uint16_t address, Args &&...args) {
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
  case ADDR_AS_USR:
    return invoke(AS_USR_impl);

  case ADDR_AS_BASIC:
    return invoke(AS_BASIC);

  case ADDR_AS_BASIC2:
    return invoke(AS_BASIC2);

  case ADDR_MON_KEYIN:
    return invoke(MON_KEYIN);

  case ADDR_MON_OLDBRK:
    return invoke(MON_OLDBRK);

  case ADDR_MON_COUT1:
    return invoke(MON_COUT1);

  case applesoft::asm_port::ApplesoftVariables::ADDR_MON_NMI_VECTOR:
    break;

  case ADDR_AS_AMPERSAND:
    return invoke(AS_AMPERSAND);

  case 0xc100u:
  case 0xc200u:
  case 0xc300u:
  case 0xc400u:
  case 0xc500u:
  case 0xc600u:
  case 0xc700u:
    // RESET slot scan (Cx00) fallback
    break;
  default:
    break;
  }

  if constexpr (std::is_void_v<R>) {
    return;
  } else {
    throw std::runtime_error("ExternalJumpDispatcher: Unhandled jump address " +
                             std::to_string(address));
  }
}

template <typename R, typename... Args>
R ExternalJumpDispatcher::JumpFromWord(std::uint16_t wordAddress,
                                       Args &&...args) {
  return JumpTo<R>(applesoft::asm_port::variables_const().readWord(wordAddress),
                   std::forward<Args>(args)...);
}

template <typename R, typename... Args>
R ExternalJumpDispatcher::JumpFromInstruction(std::uint16_t jumpAddress,
                                              Args &&...args) {
  constexpr std::uint8_t kJmpOpcode = 0x4cu;

  if (applesoft::asm_port::variables_const().readByte(jumpAddress) !=
      kJmpOpcode) {
    throw std::runtime_error("ExternalJumpDispatcher: Expected JMP opcode at "
                             "address " +
                             std::to_string(jumpAddress));
  }

  const std::uint16_t target = applesoft::asm_port::variables_const().readWord(
      static_cast<std::uint16_t>(jumpAddress + 1u));
  return JumpTo<R>(target, std::forward<Args>(args)...);
}

} // namespace ApplesoftNG
