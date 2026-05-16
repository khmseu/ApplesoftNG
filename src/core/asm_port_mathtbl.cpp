// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_MATHTBL (inclusive) .. AS_TOKEN_NAME_TABLE (exclusive)
// Name normalization: AS_OR -> AS_OR_op (AS_OR is a C++ keyword).
//
// AS_MATHTBL is a 10-entry table mixing a one-byte precedence code with a
// two-byte RTS-dispatch handler address for each math operator token $C8-$D1.
// Sub-labels AS_M_NEG (index 7), AS_MEQUU (index 8), AS_M_REL (index 9) mark
// entries that the interpreter jumps to directly; exposed as AS_M_NEG_IDX etc.
// in the header. The -1 RTS-dispatch offset is dropped; callers invoke
// entry.handler directly.

#include "core/asm_port_mathtbl.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_math.hpp"

namespace applesoft::asm_port {

void AS_OR();
void AS_RELOPS();
void AS_SNGFLT(std::uint8_t value);
void AS_ANDOP();
void AS_NORMALIZE_FAC_2();

// ---------------------------------------------------------------------------
// Stub implementations for math operator handlers not yet ported.
// ---------------------------------------------------------------------------

namespace {
// static void AS_FADDT()  {} // Removed to avoid conflict with
// asm_port_math.cpp
} // namespace
static void AS_FMULTT() {} // TODO(asm-port): AS_FMULTT $CA...202...*

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: COPY_RESULT_INTO_FAC (inclusive, 0x1ae6)
//            .. LOAD_FAC_FROM_YA (exclusive, 0x1af9)
// Intent:
//   - Copy 4-byte RESULT register into FAC mantissa (bytes +1 through +4)
//   - Call NORMALIZE_FAC_2 to process the loaded value
//   - Simple byte-by-byte copy with call-through pattern
static void AS_COPY_RESULT_INTO_FAC() {
  auto &vars = variables();
  const auto &result = vars.AS_RESULT;

  // Copy each byte from RESULT into FAC+1..FAC+4
  vars.AS_FAC[1] = result[0]; // RESULT -> FAC+1
  vars.AS_FAC[2] = result[1]; // RESULT+1 -> FAC+2
  vars.AS_FAC[3] = result[2]; // RESULT+2 -> FAC+3
  vars.AS_FAC[4] = result[3]; // RESULT+3 -> FAC+4

  // Fall through to normalize the value in FAC
  AS_NORMALIZE_FAC_2();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FDIVT (inclusive) .. AS_COPY_RESULT_INTO_FAC (exclusive)
// Name normalization: none (assembler label AS_FDIVT kept verbatim).
static void AS_FDIVT() {
  constexpr std::uint8_t kFac = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kFacMant = ApplesoftVariables::ZP_AS_FAC_MANTISSA;
  constexpr std::uint8_t kArgMant = ApplesoftVariables::ZP_AS_ARG_MANTISSA;
  constexpr std::uint8_t kResult3 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_RESULT + 3u);
  constexpr std::uint8_t kFacExt = ApplesoftVariables::ZP_AS_FAC_EXTENSION;

  if (variables_const().readByte(kFac) == 0u) {
    AS_ERROR(AS_ERR_ZERODIV);
    return;
  }

  AS_ROUND_FAC();

  const std::uint8_t facExp = variables_const().readByte(kFac);
  variables().writeByte(kFac, static_cast<std::uint8_t>(0u - facExp));

  AS_ADD_EXPONENTS();

  const std::uint8_t adjustedExp =
      static_cast<std::uint8_t>(variables_const().readByte(kFac) + 1u);
  variables().writeByte(kFac, adjustedExp);
  if (adjustedExp == 0u) {
    AS_ERROR(0x45u); // AS_OVERFLOW
    return;
  }

  std::uint8_t x = 0xfcu;
  std::uint8_t quotientByte = 0x01u;

  while (true) {
    bool facCanBeSubtracted = true;
    for (std::uint8_t i = 0u; i < 4u; ++i) {
      const std::uint8_t arg =
          variables_const().readByte(static_cast<std::uint8_t>(kArgMant + i));
      const std::uint8_t fac =
          variables_const().readByte(static_cast<std::uint8_t>(kFacMant + i));
      if (arg != fac) {
        facCanBeSubtracted = arg > fac;
        break;
      }
    }

    const bool sentinelRolledOut = (quotientByte & 0x80u) != 0u;
    quotientByte = static_cast<std::uint8_t>((quotientByte << 1u) |
                                             (facCanBeSubtracted ? 1u : 0u));

    if (sentinelRolledOut) {
      x = static_cast<std::uint8_t>(x + 1u);
      const std::uint8_t storeAddr = static_cast<std::uint8_t>(kResult3 + x);
      variables().writeByte(storeAddr, quotientByte);

      if (quotientByte == 0u) {
        quotientByte = 0x40u;
      } else if ((quotientByte & 0x80u) == 0u) {
        for (int i = 0; i < 6; ++i) {
          quotientByte = static_cast<std::uint8_t>(quotientByte << 1u);
        }
        variables().writeByte(kFacExt, quotientByte);
        AS_COPY_RESULT_INTO_FAC();
        return;
      } else {
        quotientByte = 0x01u;
      }
    }

    if (facCanBeSubtracted) {
      std::uint16_t borrow = 0u;
      for (int i = 3; i >= 0; --i) {
        const std::uint8_t argAddr =
            static_cast<std::uint8_t>(kArgMant + static_cast<std::uint8_t>(i));
        const std::uint8_t facAddr =
            static_cast<std::uint8_t>(kFacMant + static_cast<std::uint8_t>(i));
        const std::uint16_t arg = variables_const().readByte(argAddr);
        const std::uint16_t fac = variables_const().readByte(facAddr);
        const std::uint16_t diff =
            static_cast<std::uint16_t>(arg - fac - borrow);
        variables().writeByte(argAddr, static_cast<std::uint8_t>(diff & 0xffu));
        borrow = (arg < (fac + borrow)) ? 1u : 0u;
      }
    }

    // Shift ARG+4..ARG+1 left by one bit as in ASL/ROL chain in ROM code.
    std::uint8_t carry = 0u;
    for (int i = 3; i >= 0; --i) {
      const std::uint8_t addr =
          static_cast<std::uint8_t>(kArgMant + static_cast<std::uint8_t>(i));
      const std::uint8_t value = variables_const().readByte(addr);
      const std::uint8_t nextCarry = static_cast<std::uint8_t>(value >> 7u);
      const std::uint8_t shifted =
          static_cast<std::uint8_t>((value << 1u) | carry);
      variables().writeByte(addr, shifted);
      carry = nextCarry;
    }
  }
}
static void AS_FPWRT() {} // TODO(asm-port): AS_FPWRT  $CC...204...^

void AS_OR_op() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_OR (inclusive) .. AS_ANDOP (exclusive)
  // Name normalization: AS_OR -> AS_OR_op in AS_MATHTBL dispatch (AS_OR is
  // table label).

  AS_OR();
}

void AS_NEGOP() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_NEGOP (inclusive) .. AS_CON_LOG_E (exclusive)
  // Name normalization: none (assembler label AS_NEGOP kept verbatim).

  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_FAC_SIGN = ApplesoftVariables::ZP_AS_FAC_SIGN;

  // ROM: if AS_FAC exponent is zero, value is 0 so sign toggle is skipped.
  if (variables_const().readByte(kAS_FAC) == 0u) {
    return;
  }

  const std::uint8_t sign = variables_const().readByte(kAS_FAC_SIGN);
  variables().writeByte(kAS_FAC_SIGN, static_cast<std::uint8_t>(sign ^ 0xffu));
}

void AS_EQUOP() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_EQUOP (inclusive) .. AS_FN_ (exclusive)
  // Name normalization: none (assembler label AS_EQUOP kept verbatim).
  //
  // Tests whether AS_FAC == 0.  In the Applesoft float format the exponent byte
  // is stored at ZP_AS_FAC ($9D); a zero exponent means the value is exactly
  // 0.0. Returns AS_FAC = 1.0 (true) when the operand is zero, 0.0 (false)
  // otherwise. Used both as the "=" operator handler (via AS_MEQUU table entry)
  // and as the implementation of the NOT pseudo-function (via AS_NOT_ ->
  // AS_EQUL -> AS_EQUOP).

  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;

  const std::uint8_t facExponent = variables_const().readByte(kAS_FAC);
  AS_SNGFLT(facExponent == 0u ? static_cast<std::uint8_t>(1u)
                              : static_cast<std::uint8_t>(0u));
}

// ---------------------------------------------------------------------------
// Math operator table: precedence + handler for tokens $C8-$D1.
// Index = token - $C8.
// ---------------------------------------------------------------------------
MathTblEntry AS_MATHTBL(std::size_t index) {
  static constexpr MathTblEntry table[] = {
      {AS_P_ADD, AS_FADDT},  // [0] M_ADD  $C8...200...+
      {AS_P_ADD, AS_FSUBT},  // [1]        $C9...201...-
      {AS_P_MUL, AS_FMULTT}, // [2]        $CA...202...*
      {AS_P_MUL, AS_FDIVT},  // [3]        $CB...203.../
      {AS_P_PWR, AS_FPWRT},  // [4]        $CC...204...^
      {AS_P_AND, AS_ANDOP},  // [5]        $CD...205...AND
      {AS_P_OR, AS_OR_op},   // [6]        $CE...206...AS_OR
      {AS_P_NEQ, AS_NEGOP},  // [7] AS_M_NEG  $CF...207...>
      {AS_P_NEQ, AS_EQUOP},  // [8] AS_MEQUU  $D0...208...=
      {AS_P_REL,
       AS_RELOPS}, // [9] AS_M_REL  $D1...209...< (dispatches to core AS_RELOPS)
  };
  return table[index];
}

} // namespace applesoft::asm_port
