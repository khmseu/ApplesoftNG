#pragma once

#include <cstddef>
#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MATHTBL (inclusive) .. TOKEN_NAME_TABLE (exclusive)
// Name normalization: OR -> OR_op (OR is a C++ keyword).

// Precedence codes (equates from the source, used by the math operator evaluator).
inline constexpr std::uint8_t P_OR  = 0x46; // "OR"  - lowest precedence
inline constexpr std::uint8_t P_AND = 0x50;
inline constexpr std::uint8_t P_REL = 0x64; // relational operators
inline constexpr std::uint8_t P_ADD = 0x79; // binary + and -
inline constexpr std::uint8_t P_MUL = 0x7b; // * and /
inline constexpr std::uint8_t P_PWR = 0x7d; // exponentiation
inline constexpr std::uint8_t P_NEQ = 0x7f; // unary - and comparison =

/// Common signature for math operator handler functions.
using MATHTBL_fn = void(*)();

/// One entry in the math operator table: precedence byte + handler pointer.
/// Corresponds to the 3-byte (.byt precedence / .word handler-1) format in source.
/// The -1 RTS-dispatch artifact is dropped; the caller invokes handler directly.
struct MathTblEntry {
    std::uint8_t precedence;
    MATHTBL_fn   handler;
};

// Sub-label indices (direct entry points referenced elsewhere in the interpreter).
inline constexpr std::size_t M_NEG_IDX = 7; // token $CF...207...>
inline constexpr std::size_t MEQUU_IDX = 8; // token $D0...208...=
inline constexpr std::size_t M_REL_IDX = 9; // token $D1...209...<

/// Look up the math operator table entry for the given index (index = token - 0xC8).
/// Maps tokens $C8 (+) through $D1 (<), i.e. indices 0-9.
/// Returns a MathTblEntry; the caller is responsible for invoking entry.handler.
MathTblEntry MATHTBL(std::size_t index);

} // namespace applesoft::asm_port
