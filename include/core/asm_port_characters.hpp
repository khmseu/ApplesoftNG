#pragma once

#include <cstdint>

namespace applesoft::asm_port {

inline constexpr std::uint8_t kControlMask = 0x1fu;
inline constexpr std::uint8_t kHighBitMask = 0x80u;

inline constexpr std::uint8_t kCharCarriageReturn = '\r';
inline constexpr std::uint8_t kCharEscape = 0x1bu;
inline constexpr std::uint8_t kControlCharC = ('C' & kControlMask);
inline constexpr std::uint8_t kControlCharS = ('S' & kControlMask);

} // namespace applesoft::asm_port