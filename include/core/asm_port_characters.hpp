#pragma once

#include <cstdint>

namespace applesoft::asm_port {

inline constexpr std::uint8_t kCharCarriageReturn = '\r';
inline constexpr std::uint8_t kCharCarriageReturnHigh = ('\r' | 0x80u);

inline constexpr std::uint8_t kControlCharC = ('C' & 0x1fu);
inline constexpr std::uint8_t kControlCharCHigh = (('C' & 0x1fu) | 0x80u);

} // namespace applesoft::asm_port