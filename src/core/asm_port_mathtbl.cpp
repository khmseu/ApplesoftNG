// Source:
// SourceMaterial/Combo/asrom.lst
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

#include <cmath>
#include <cstdint>

namespace applesoft::asm_port {

void AS_OR();
void AS_RELOPS();
void AS_SNGFLT(std::uint8_t value);
void AS_ANDOP();
void AS_NORMALIZE_FAC_2();
static void AS_COPY_RESULT_INTO_FAC(); // forward declaration (defined below)

// ---------------------------------------------------------------------------
// Stub implementations for math operator handlers not yet ported.
// ---------------------------------------------------------------------------

namespace {
// static void AS_FADDT()  {} // Removed to avoid conflict with
// asm_port_math.cpp

// ---------------------------------------------------------------------------
// Local helpers: convert AS_FAC / AS_ARG to/from double.
// The packed-float format: byte[0] = biased exponent (0 = zero), byte[1..4] =
// mantissa with implied leading 1 in bit 7 of byte[1], sign = separate byte.
// ---------------------------------------------------------------------------
static double facToDouble() {
  const auto &cv = variables_const();
  const std::uint8_t exponent = cv.AS_FAC[0];
  if (exponent == 0u) {
    return 0.0;
  }
  const std::uint32_t mantissa =
      (static_cast<std::uint32_t>(cv.AS_FAC[1]) << 24u) |
      (static_cast<std::uint32_t>(cv.AS_FAC[2]) << 16u) |
      (static_cast<std::uint32_t>(cv.AS_FAC[3]) << 8u) |
      static_cast<std::uint32_t>(cv.AS_FAC[4]);
  const double fraction = static_cast<double>(mantissa) / 4294967296.0;
  const double value = std::ldexp(fraction, static_cast<int>(exponent) - 128);
  return (cv.AS_FAC_SIGN != 0u) ? -value : value;
}

static double argToDouble() {
  const auto &cv = variables_const();
  const std::uint8_t exponent = cv.AS_ARG[0];
  if (exponent == 0u) {
    return 0.0;
  }
  const std::uint32_t mantissa =
      (static_cast<std::uint32_t>(cv.AS_ARG[1]) << 24u) |
      (static_cast<std::uint32_t>(cv.AS_ARG[2]) << 16u) |
      (static_cast<std::uint32_t>(cv.AS_ARG[3]) << 8u) |
      static_cast<std::uint32_t>(cv.AS_ARG[4]);
  const double fraction = static_cast<double>(mantissa) / 4294967296.0;
  const double value = std::ldexp(fraction, static_cast<int>(exponent) - 128);
  return (cv.AS_ARG[5] != 0u) ? -value : value;
}

static void doubleToFac(double value) {
  auto &vars = variables();
  if (value == 0.0) {
    vars.AS_FAC[0] = 0u;
    vars.AS_FAC_SIGN = 0u;
    return;
  }
  const bool negative = value < 0.0;
  value = std::fabs(value);
  int exponent2 = 0;
  const double fraction = std::frexp(value, &exponent2);
  std::uint8_t exponent8 =
      static_cast<std::uint8_t>(static_cast<int>(exponent2) + 128);
  std::uint64_t mantissa = static_cast<std::uint64_t>(std::ldexp(fraction, 32));
  if (mantissa >= 0x1'0000'0000ull) {
    mantissa >>= 1u;
    ++exponent8;
  }
  vars.AS_FAC[0] = exponent8;
  vars.AS_FAC[1] = static_cast<std::uint8_t>((mantissa >> 24u) & 0xffu);
  vars.AS_FAC[2] = static_cast<std::uint8_t>((mantissa >> 16u) & 0xffu);
  vars.AS_FAC[3] = static_cast<std::uint8_t>((mantissa >> 8u) & 0xffu);
  vars.AS_FAC[4] = static_cast<std::uint8_t>(mantissa & 0xffu);
  vars.AS_FAC_SIGN = negative ? 0xffu : 0x00u;
}

} // namespace
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: FMULTT (inclusive) .. LOAD_ARG_FROM_YA (exclusive)
// Name normalization: FMULTT -> AS_FMULTT (AS_ prefix convention).
//
// Multiplies FAC by ARG, storing the result in FAC.  Computes the biased
// result exponent as FAC_exp + ARG_exp - 128, handles zero/overflow/underflow,
// then multiplies the two mantissas using 128-bit integer arithmetic and
// stores the high 40 bits into RESULT/FAC_EXTENSION before calling
// AS_COPY_RESULT_INTO_FAC for normalization.
static void AS_FMULTT() {
  const auto &cvars = variables_const();
  auto &vars = variables();

  // If FAC == 0, result is 0 (FAC already holds 0).
  if (cvars.AS_FAC[0] == 0u) {
    return;
  }

  // If ARG == 0, set FAC to 0 and return.
  if (cvars.AS_ARG[0] == 0u) {
    vars.AS_FAC[0] = 0u;
    return;
  }

  // Compute result exponent: arg_exp + fac_exp - 128 (re-biased).
  const std::int16_t new_exp = static_cast<std::int16_t>(cvars.AS_ARG[0]) +
                               static_cast<std::int16_t>(cvars.AS_FAC[0]) - 128;
  if (new_exp > 255) {
    AS_ERROR(AS_ERR_OVERFLOW);
    return;
  }
  if (new_exp <= 0) {
    // Underflow: result is 0.
    vars.AS_FAC[0] = 0u;
    return;
  }
  vars.AS_FAC[0] = static_cast<std::uint8_t>(new_exp);

  // Set combined sign for the product.
  vars.AS_FAC_SIGN =
      static_cast<std::uint8_t>(cvars.AS_FAC_SIGN ^ cvars.AS_ARG[5]);

  // Compute product of FAC mantissa (40-bit) × ARG mantissa (32-bit).
  // FAC mantissa: bytes [1..4] MSB..LSB, plus FAC_EXTENSION as guard byte.
  const std::uint64_t fac_m =
      (static_cast<std::uint64_t>(cvars.AS_FAC[1]) << 32u) |
      (static_cast<std::uint64_t>(cvars.AS_FAC[2]) << 24u) |
      (static_cast<std::uint64_t>(cvars.AS_FAC[3]) << 16u) |
      (static_cast<std::uint64_t>(cvars.AS_FAC[4]) << 8u) |
      static_cast<std::uint64_t>(cvars.AS_FAC_EXTENSION);
  // ARG mantissa: bytes [1..4] MSB..LSB.
  const std::uint64_t arg_m =
      (static_cast<std::uint64_t>(cvars.AS_ARG[1]) << 24u) |
      (static_cast<std::uint64_t>(cvars.AS_ARG[2]) << 16u) |
      (static_cast<std::uint64_t>(cvars.AS_ARG[3]) << 8u) |
      static_cast<std::uint64_t>(cvars.AS_ARG[4]);

  // 40-bit × 32-bit = 72-bit product; keep high 40 bits (product >> 32).
  // RESULT[0..3] = bits [39..8], FAC_EXTENSION = bits [7..0].
  // NOLINTNEXTLINE(misc-include-cleaner) — __int128 is a builtin extension
  const unsigned __int128 product =
      static_cast<unsigned __int128>(fac_m) * arg_m;
  const std::uint64_t result_40 = static_cast<std::uint64_t>(product >> 32u);

  vars.AS_RESULT[0] = static_cast<std::uint8_t>((result_40 >> 32u) & 0xffu);
  vars.AS_RESULT[1] = static_cast<std::uint8_t>((result_40 >> 24u) & 0xffu);
  vars.AS_RESULT[2] = static_cast<std::uint8_t>((result_40 >> 16u) & 0xffu);
  vars.AS_RESULT[3] = static_cast<std::uint8_t>((result_40 >> 8u) & 0xffu);
  vars.AS_FAC_EXTENSION = static_cast<std::uint8_t>(result_40 & 0xffu);

  AS_COPY_RESULT_INTO_FAC();
}

// Source:
// SourceMaterial/Combo/asrom.lst
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
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FDIVT (inclusive) .. AS_COPY_RESULT_INTO_FAC (exclusive)
// Name normalization: none (assembler label AS_FDIVT kept verbatim).
static void AS_FDIVT() {
  constexpr std::uint8_t kFacMant = ApplesoftVariables::ZP_AS_FAC_MANTISSA;
  constexpr std::uint8_t kArgMant = ApplesoftVariables::ZP_AS_ARG_MANTISSA;
  constexpr std::uint8_t kResult3 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_RESULT + 3u);
  constexpr std::uint8_t kFacExt = ApplesoftVariables::ZP_AS_FAC_EXTENSION;
  auto &vars = variables();
  const auto &cvars = variables_const();

  if (cvars.AS_FAC[0] == 0u) {
    AS_ERROR(AS_ERR_ZERODIV);
    return;
  }

  AS_ROUND_FAC();

  const std::uint8_t facExp = cvars.AS_FAC[0];
  vars.AS_FAC[0] = static_cast<std::uint8_t>(0u - facExp);

  AS_ADD_EXPONENTS();

  const std::uint8_t adjustedExp =
      static_cast<std::uint8_t>(cvars.AS_FAC[0] + 1u);
  vars.AS_FAC[0] = adjustedExp;
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
        vars.writeByte(kFacExt, quotientByte);
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
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: FPWRT (inclusive) .. NEGOP (exclusive)
// Name normalization: FPWRT -> AS_FPWRT (AS_ prefix convention).
//
// Computes AS_FAC = AS_ARG ^ AS_FAC (ARG raised to the FAC power).  Uses the
// ROM identity: ARG ^ FAC = exp(FAC * log(ARG)), with special cases for zero
// operands, negative ARG with integral exponent, and non-finite results.
static void AS_FPWRT() {
  const double exponent = facToDouble();
  const double base = argToDouble();

  // If exponent is 0, result is 1 regardless of base.
  if (exponent == 0.0) {
    doubleToFac(1.0);
    return;
  }

  // If base is 0, result is 0 (0 ^ positive = 0; ROM maps to EXP(0)=1 for
  // zero exponent which was handled above).
  if (base == 0.0) {
    doubleToFac(0.0);
    return;
  }

  // Negative base is only valid for integral exponents.
  if (base < 0.0) {
    const double exp_int = std::floor(exponent);
    if (exponent != exp_int) {
      AS_ERROR(AS_ERR_ILLQTY);
      return;
    }
    // Compute |base|^exponent and negate if exponent is odd.
    const double result = std::pow(-base, exponent);
    if (!std::isfinite(result)) {
      AS_ERROR(AS_ERR_OVERFLOW);
      return;
    }
    const bool odd = (static_cast<long long>(exp_int) & 1LL) != 0LL;
    doubleToFac(odd ? -result : result);
    return;
  }

  const double result = std::pow(base, exponent);
  if (!std::isfinite(result)) {
    AS_ERROR(AS_ERR_OVERFLOW);
    return;
  }
  doubleToFac(result);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_OR (inclusive) .. AS_ANDOP (exclusive)
// Name normalization: AS_OR -> AS_OR_op in AS_MATHTBL dispatch (AS_OR is
// table label).
void AS_OR_op() { AS_OR(); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_NEGOP (inclusive) .. AS_CON_LOG_E (exclusive)
// Name normalization: none (assembler label AS_NEGOP kept verbatim).
void AS_NEGOP() {

  auto &vars = variables();
  const auto &cvars = variables_const();

  // ROM: if AS_FAC exponent is zero, value is 0 so sign toggle is skipped.
  if (cvars.AS_FAC[0] == 0u) {
    return;
  }

  const std::uint8_t sign = cvars.AS_FAC_SIGN;
  vars.AS_FAC_SIGN = static_cast<std::uint8_t>(sign ^ 0xffu);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_EQUOP (inclusive) .. AS_FN_ (exclusive)
// Name normalization: none (assembler label AS_EQUOP kept verbatim).
//
// Tests whether AS_FAC == 0.  In the Applesoft float format the exponent byte
// is stored at ZP_AS_FAC ($9D); a zero exponent means the value is exactly
// 0.0. Returns AS_FAC = 1.0 (true) when the operand is zero, 0.0 (false)
// otherwise. Used both as the "=" operator handler (via AS_MEQUU table entry)
// and as the implementation of the NOT pseudo-function (via AS_NOT_ ->
// AS_EQUL -> AS_EQUOP).
void AS_EQUOP() {

  const std::uint8_t facExponent = variables_const().AS_FAC[0];
  AS_SNGFLT(facExponent == 0u ? static_cast<std::uint8_t>(1u)
                              : static_cast<std::uint8_t>(0u));
}

// ---------------------------------------------------------------------------
// Math operator table: precedence + handler for tokens $C8-$D1.
// Index = token - $C8.
// AS_Labels: AS_MATHTBL (inclusive) .. AS_TOKEN_NAME_TABLE (exclusive)
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
