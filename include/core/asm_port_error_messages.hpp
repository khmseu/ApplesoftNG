#pragma once

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: ERROR_MESSAGES (inclusive) .. QT_ERROR (exclusive)
// Name normalization: none (assembler label ERROR_MESSAGES kept verbatim).

// Original byte offsets into the packed ERROR_MESSAGES table.
inline constexpr std::uint8_t ERR_NOFOR     = 0x00;
inline constexpr std::uint8_t ERR_SYNTAX    = 0x10;
inline constexpr std::uint8_t ERR_NOGOSUB   = 0x16;
inline constexpr std::uint8_t ERR_NODATA    = 0x2a;
inline constexpr std::uint8_t ERR_ILLQTY    = 0x35;
inline constexpr std::uint8_t ERR_OVERFLOW  = 0x45;
inline constexpr std::uint8_t ERR_MEMFULL   = 0x4d;
inline constexpr std::uint8_t ERR_UNDEFSTAT = 0x5a;
inline constexpr std::uint8_t ERR_BADSUBS   = 0x6b;
inline constexpr std::uint8_t ERR_REDIMD    = 0x78;
inline constexpr std::uint8_t ERR_ZERODIV   = 0x85;
inline constexpr std::uint8_t ERR_ILLDIR    = 0x95;
inline constexpr std::uint8_t ERR_BADTYPE   = 0xa3;
inline constexpr std::uint8_t ERR_STRLONG   = 0xb0;
inline constexpr std::uint8_t ERR_FRMCPX    = 0xbf;
inline constexpr std::uint8_t ERR_CANTCONT  = 0xd2;
inline constexpr std::uint8_t ERR_UNDEFFUNC = 0xe0;

/// Decode an Applesoft error message from its original table byte offset.
/// Returns an empty string view for unknown offsets.
std::string_view ERROR_MESSAGES(std::uint8_t offset);

} // namespace applesoft::asm_port
