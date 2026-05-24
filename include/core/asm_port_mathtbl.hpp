#pragma once

#include "core/applesoft_dual_pointer.hpp"

#include <cstddef>
#include <cstdint>

namespace applesoft::asm_port {

// Precedence codes (equates from the source, used by the math operator
// evaluator).
inline constexpr std::uint8_t AS_P_OR = 0x46; // "AS_OR"  - lowest precedence
inline constexpr std::uint8_t AS_P_AND = 0x50;
inline constexpr std::uint8_t AS_P_REL = 0x64; // relational operators
inline constexpr std::uint8_t AS_P_ADD = 0x79; // binary + and -
inline constexpr std::uint8_t AS_P_MUL = 0x7b; // * and /
inline constexpr std::uint8_t AS_P_PWR = 0x7d; // exponentiation
inline constexpr std::uint8_t AS_P_NEQ = 0x7f; // unary - and comparison =

/// Common signature for math operator handler functions.
using AS_MATHTBL_fn = void (*)();

/// One entry in the math operator table: precedence byte + handler pointer.
/// Corresponds to the 3-byte (.byt precedence / .word handler-1) format in
/// source. The -1 RTS-dispatch artifact is dropped; the caller invokes handler
/// directly.
struct MathTblEntry {
  std::uint8_t precedence;
  AS_MATHTBL_fn handler;
};

// Sub-label indices (direct entry points referenced elsewhere in the
// interpreter).
inline constexpr std::size_t AS_M_NEG_IDX = 7; // token $CF...207...>
inline constexpr std::size_t AS_MEQUU_IDX = 8; // token $D0...208...=
inline constexpr std::size_t AS_M_REL_IDX = 9; // token $D1...209...<

void AS_NEGOP();
void AS_EQUOP();

/// AS_Look up the math operator table entry for the given index (index = token
/// - 0xC8). Maps tokens $C8 (+) through $D1 (<), i.e. indices 0-9. Returns a
/// MathTblEntry; the caller is responsible for invoking entry.handler.
MathTblEntry AS_MATHTBL(std::size_t index);

/// Canonical label API for AS_MATHTBL lookup.
ApplesoftDualPointer<const MathTblEntry> AS_MATHTBL_ptr(std::size_t index);

/// Decode MathTblEntry from canonical dual-pointer input.
MathTblEntry AS_MATHTBL(ApplesoftDualPointer<const MathTblEntry> entry_ptr);

} // namespace applesoft::asm_port
