#pragma once

#include <cstdint>

namespace applesoft::asm_port {

inline constexpr std::uint8_t kCharCarriageReturn = '\r';
inline constexpr std::uint8_t kCharCarriageReturnHigh = ('\r' | 0x80u);
inline constexpr std::uint8_t kCharEscape = 0x1bu;
inline constexpr std::uint8_t kCharEscapeHigh = (0x1bu | 0x80u);

inline constexpr std::uint8_t kCharSpace = ' ';
inline constexpr std::uint8_t kCharSpaceHigh = (' ' | 0x80u);
inline constexpr std::uint8_t kCharQuestionMark = '?';
inline constexpr std::uint8_t kCharComma = ',';
inline constexpr std::uint8_t kCharSemicolon = ';';
inline constexpr std::uint8_t kCharColon = ':';
inline constexpr std::uint8_t kCharCloseParen = ')';
inline constexpr std::uint8_t kCharPeriod = '.';
inline constexpr std::uint8_t kCharHash = '#';

inline constexpr std::uint8_t kControlCharC = ('C' & 0x1fu);
inline constexpr std::uint8_t kControlCharCHigh = (('C' & 0x1fu) | 0x80u);
inline constexpr std::uint8_t kControlCharS = ('S' & 0x1fu);
inline constexpr std::uint8_t kControlCharSHigh = (('S' & 0x1fu) | 0x80u);

} // namespace applesoft::asm_port