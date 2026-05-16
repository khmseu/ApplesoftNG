// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_UNFNC (inclusive) .. AS_MATHTBL (exclusive)
// Name normalization: none
//
// AS_UNFNC is a direct-address table (no RTS-dispatch -1 offset) for Applesoft
// unary and built-in function tokens $D2-$EA. The caller dispatches by
// computing (token - 0xD2) as the index and invoking the returned pointer.

#include "core/asm_port_unfnc.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_strlit.hpp"
#include "core/asm_port_strlt2.hpp"

namespace applesoft::asm_port {

std::uint8_t MON_PREAD();
void AS_CONINT();
void AS_SNGFLT(std::uint8_t value);
void AS_GIVAYF(std::int16_t value);
void AS_FIN();
std::uint8_t AS_GETSTR();
void AS_FRE();
void AS_PEEK();
void AS_QINT();
void AS_ERROR(std::uint8_t error_code_offset);

static void AS_NORMALIZE_FAC_1(std::uint8_t facSign) {
  std::uint64_t integerValue =
      (static_cast<std::uint64_t>(variables_const().AS_FAC[1]) << 24u) |
      (static_cast<std::uint64_t>(variables_const().AS_FAC[2]) << 16u) |
      (static_cast<std::uint64_t>(variables_const().AS_FAC[3]) << 8u) |
      static_cast<std::uint64_t>(variables_const().AS_FAC[4]);

  if ((facSign & 0x80u) != 0u) {
    integerValue = (~integerValue + 1u) & 0xffff'ffffu;
    variables().AS_FAC_SIGN = 0xffu;
  } else {
    variables().AS_FAC_SIGN = 0u;
  }

  if (integerValue == 0u) {
    variables().AS_FAC[0] = 0u;
    variables().AS_FAC_EXTENSION = 0u;
    return;
  }

  std::uint8_t exponent = variables_const().AS_FAC[0];
  while ((integerValue & 0x8000'0000u) == 0u) {
    integerValue <<= 1u;
    if (exponent == 0u) {
      variables().AS_FAC[0] = 0u;
      variables().AS_FAC_SIGN = 0u;
      variables().AS_FAC_EXTENSION = 0u;
      variables().AS_FAC[1] = 0u;
      variables().AS_FAC[2] = 0u;
      variables().AS_FAC[3] = 0u;
      variables().AS_FAC[4] = 0u;
      return;
    }

    --exponent;
  }

  variables().AS_FAC[0] = exponent;
  variables().AS_FAC[1] =
      static_cast<std::uint8_t>((integerValue >> 24u) & 0xffu);
  variables().AS_FAC[2] =
      static_cast<std::uint8_t>((integerValue >> 16u) & 0xffu);
  variables().AS_FAC[3] =
      static_cast<std::uint8_t>((integerValue >> 8u) & 0xffu);
  variables().AS_FAC[4] = static_cast<std::uint8_t>(integerValue & 0xffu);
  variables().AS_FAC_EXTENSION = 0u;
}

void AS_PDL() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PDL (inclusive) .. AS_NXDIM (exclusive)
  // Name normalization: none (assembler label AS_PDL kept verbatim).

  AS_CONINT();
  AS_SNGFLT(MON_PREAD());
}

// ---------------------------------------------------------------------------
// Stub implementations for function handlers not yet ported.
// ---------------------------------------------------------------------------

void AS_SGN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SGN (inclusive) .. AS_ABS (exclusive)
  // Name normalization: none (assembler label AS_SGN kept verbatim).

  // ROM path: jsr AS_SIGN then fall through to AS_FLOAT to convert A=-1/0/+1 to
  // AS_FAC.
  std::int16_t signValue = 0;
  if (variables_const().AS_FAC[0] != 0u) {
    signValue = ((variables_const().AS_FAC_SIGN & 0x80u) != 0u) ? -1 : 1;
  }

  AS_GIVAYF(signValue);
}
void AS_INT_fn() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_INT (inclusive) .. AS_QINT_3 (exclusive)
  // Name normalization: AS_INT -> AS_INT_fn (AS_INT is a C++ keyword).

  // If exponent >= 0xA0, AS_FAC has no fractional bits and AS_INT is already
  // done.
  if (variables_const().AS_FAC[0] >= 0xa0u) {
    return;
  }

  // Convert AS_FAC to integer form in AS_FAC+1..AS_FAC+4 (AS_QINT contract).
  AS_QINT();

  // The ROM uses carry to decide whether the integer needs complementing
  // before re-normalization. Model that explicitly with the saved AS_FAC sign.
  const std::uint8_t facSign = variables_const().AS_FAC_SIGN;
  variables().AS_FAC_EXTENSION = 0u;
  variables().AS_FAC[0] = 0xa0u;
  variables().AS_CHARAC = variables_const().AS_FAC[4];

  AS_NORMALIZE_FAC_1(facSign);
}
void AS_ABS() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ABS (inclusive) .. AS_FCOMP (exclusive)
  // Name normalization: none (assembler label AS_ABS kept verbatim).

  variables().AS_FAC_SIGN =
      static_cast<std::uint8_t>(variables_const().AS_FAC_SIGN >> 1u);
}
static void AS_USR() {} // TODO(asm-port): AS_USR        $D5...213 (user-defined
                        // function via zero-page JMP at $0A)
void AS_FRE_fn() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FRE (inclusive) .. AS_ERROR (exclusive)
  // Name normalization: wrapper name AS_FRE_fn used to avoid clashing with the
  // existing AS_FRE implementation in core.

  AS_FRE();
}
void AS_ERROR_fn() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_UNDFNC (inclusive) .. AS_DEF (exclusive)
  // Name normalization: AS_ERROR_fn used for AS_UNFNC table entry $D7 (SCRN().

  AS_ERROR(AS_ERR_UNDEFFUNC);
}
static void AS_PDL_fn() { AS_PDL(); }
void AS_POS() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_POS (inclusive) .. AS_SNGFLT (exclusive)
  // Name normalization: none (assembler label AS_POS kept verbatim).

  AS_SNGFLT(variables_const().MON_CH);
}
static void AS_SQR() {} // TODO(asm-port): AS_SQR        $DA...218
static void AS_RND() {} // TODO(asm-port): AS_RND        $DB...219
static void AS_LOG() {} // TODO(asm-port): AS_LOG        $DC...220
static void AS_EXP() {} // TODO(asm-port): AS_EXP        $DD...221
static void AS_COS() {} // TODO(asm-port): AS_COS        $DE...222
static void AS_SIN() {} // TODO(asm-port): AS_SIN        $DF...223
static void AS_TAN() {} // TODO(asm-port): AS_TAN        $E0...224
static void AS_ATN() {} // TODO(asm-port): AS_ATN        $E1...225
static void AS_PEEK_fn() { AS_PEEK(); }
static void AS_LEN_fn() { AS_LEN(); }
static void AS_STR_fn() { AS_STR(); }
// AS_Labels: AS_VAL (inclusive) .. AS_L_VAL_1 (exclusive)
// Name normalization: none (assembler label AS_VAL kept verbatim).
void AS_VAL() {
  (void)AS_GETSTR();
  AS_FIN();
}
static void AS_ASC_fn() { AS_ASC(); }
static void AS_CHRSTR_fn() { AS_CHRSTR(); }
static void AS_LEFTSTR_fn() { AS_LEFTSTR(); }
static void AS_RIGHTSTR_fn() { AS_RIGHTSTR(); }
static void AS_MIDSTR_fn() { AS_MIDSTR(); }

// ---------------------------------------------------------------------------
// Dispatch table for Applesoft unary/built-in function tokens $D2-$EA.
// Index = token - $D2. Direct addresses — no RTS-dispatch -1 adjustment.
// ---------------------------------------------------------------------------
AS_UNFNC_fn AS_UNFNC(std::size_t index) {
  static constexpr AS_UNFNC_fn table[] = {
      AS_SGN,      // [0]  $D2...210...AS_SGN
      AS_INT_fn,   // [1]  $D3...211...AS_INT   (AS_INT_fn: AS_INT is a reserved
                   // C++ keyword)
      AS_ABS,      // [2]  $D4...212...AS_ABS
      AS_USR,      // [3]  $D5...213...AS_USR
      AS_FRE_fn,   // [4]  $D6...214...AS_FRE
      AS_ERROR_fn, // [5]  $D7...215...SCRN(
      AS_PDL_fn,   // [6]  $D8...216...AS_PDL
      AS_POS,      // [7]  $D9...217...AS_POS
      AS_SQR,      // [8]  $DA...218...AS_SQR
      AS_RND,      // [9]  $DB...219...AS_RND
      AS_LOG,      // [10] $DC...220...AS_LOG
      AS_EXP,      // [11] $DD...221...AS_EXP
      AS_COS,      // [12] $DE...222...AS_COS
      AS_SIN,      // [13] $DF...223...AS_SIN
      AS_TAN,      // [14] $E0...224...AS_TAN
      AS_ATN,      // [15] $E1...225...AS_ATN
      AS_PEEK_fn,  // [16] $E2...226...AS_PEEK
      AS_LEN_fn,   // [17] $E3...227...AS_LEN
      AS_STR_fn,   // [18] $E4...228...AS_STR$
      AS_VAL,      // [19] $E5...229...AS_VAL
      AS_ASC_fn,   // [20] $E6...230...AS_ASC
      AS_CHRSTR_fn,   // [21] $E7...231...CHR$
      AS_LEFTSTR_fn,  // [22] $E8...232...AS_LEFT$
      AS_RIGHTSTR_fn, // [23] $E9...233...RIGHT$
      AS_MIDSTR_fn,   // [24] $EA...234...MID$
  };
  return table[index];
}

} // namespace applesoft::asm_port
