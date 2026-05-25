#pragma once

#include <cstdint>

namespace applesoft::asm_port {

inline constexpr std::uint8_t kHighBitMask = 0x80u;

inline constexpr std::uint8_t kCharCarriageReturn = '\r';
inline constexpr std::uint8_t kCharCarriageReturnHigh =
    (kCharCarriageReturn | kHighBitMask);
inline constexpr std::uint8_t kCharEscape = 0x1bu;
inline constexpr std::uint8_t kCharEscapeHigh = (kCharEscape | kHighBitMask);

inline constexpr std::uint8_t kCharSpace = ' ';
inline constexpr std::uint8_t kCharSpaceHigh = (kCharSpace | kHighBitMask);
inline constexpr std::uint8_t kCharQuestionMark = '?';
inline constexpr std::uint8_t kCharComma = ',';
inline constexpr std::uint8_t kCharSemicolon = ';';
inline constexpr std::uint8_t kCharColon = ':';
inline constexpr std::uint8_t kCharCloseParen = ')';
inline constexpr std::uint8_t kCharPeriod = '.';
inline constexpr std::uint8_t kCharHash = '#';

inline constexpr std::uint8_t kControlMask = 0x1fu;

inline constexpr std::uint8_t kControlCharC = ('C' & kControlMask);
inline constexpr std::uint8_t kControlCharCHigh =
    (kControlCharC | kHighBitMask);
inline constexpr std::uint8_t kControlCharS = ('S' & kControlMask);
inline constexpr std::uint8_t kControlCharSHigh =
    (kControlCharS | kHighBitMask);

} // namespace applesoft::asm_port