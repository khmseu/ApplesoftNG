#pragma once

#include <cstdint>

namespace applesoft::asm_port::local_utils {

inline constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
  return static_cast<std::uint8_t>(lhs + rhs);
}

} // namespace applesoft::asm_port::local_utils
