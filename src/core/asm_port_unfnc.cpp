// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_UNFNC (inclusive) .. AS_MATHTBL (exclusive)
// Name normalization: none
//
// AS_UNFNC is a direct-address table (no RTS-dispatch -1 offset) for Applesoft
// unary and built-in function tokens $D2-$EA. The caller dispatches by
// computing (token - 0xD2) as the index and invoking the returned pointer.

#include "core/asm_port_unfnc.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_core.hpp"
#include "core/asm_port_error_handling.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_math.hpp"
#include "core/asm_port_mathtbl.hpp"
#include "core/asm_port_parser.hpp"
#include "core/asm_port_statements.hpp"
#include "core/asm_port_strlit.hpp"
#include "core/asm_port_strlt2.hpp"
#include "core/jump_table.hpp"

namespace applesoft::asm_port {

static void AS_L_VAL_1();
static void AS_L_VAL_2();
static void AS_SIN();

namespace {
constexpr std::size_t kMathMulIdx = 2u;
constexpr std::size_t kMathDivIdx = 3u;
constexpr std::size_t kMathPowIdx = 4u;
constexpr std::uint16_t kAsTemp1Address = 0x0093u;
constexpr std::uint16_t kAsTemp2Address = 0x0098u;
constexpr std::uint16_t kAsTemp3Address = 0x008au;
constexpr std::uint16_t kAsConHalfAddress = 0xee64u;
constexpr std::uint16_t kAsConOneAddress = 0xe913u;
constexpr std::uint16_t kAsConRnd1Address = 0xefa6u;
constexpr std::uint16_t kAsConRnd2Address = 0xefaau;
constexpr std::uint16_t kAsConPiHalfAddress = 0xf066u;
constexpr std::uint16_t kAsConPiDoubleAddress = 0xf06bu;
constexpr std::uint16_t kAsQuarterAddress = 0xf070u;
constexpr std::uint16_t kAsPolySinAddress = 0xf075u;
constexpr std::uint16_t kAsPolyAtnAddress = 0xf0ceu;
constexpr std::uint16_t kAsConSqrHalfAddress = 0xe92du;
constexpr std::uint16_t kAsConSqrTwoAddress = 0xe932u;
constexpr std::uint16_t kAsConNegHalfAddress = 0xe937u;
constexpr std::uint16_t kAsConLogTwoAddress = 0xe93cu;
constexpr std::uint16_t kAsPolyLogAddress = 0xe918u;
constexpr std::uint16_t kAsConLogEAddress = 0xeedbu;
constexpr std::uint16_t kAsPolyExpAddress = 0xeee0u;

static void loadArgFromPacked(std::uint16_t address) {
  const auto source = variables_const().pointer(address);
  const std::uint8_t signPackedMantissa = source.read(1u);

  auto &vars = variables();
  vars.AS_ARG[0] = source.read(0u);
  vars.AS_ARG[1] = static_cast<std::uint8_t>(signPackedMantissa | 0x80u);
  vars.AS_ARG[2] = source.read(2u);
  vars.AS_ARG[3] = source.read(3u);
  vars.AS_ARG[4] = source.read(4u);
  vars.AS_ARG[5] = signPackedMantissa;
}

static void loadFacFromPacked(std::uint16_t address) {
  const auto source = variables_const().pointer(address);
  const std::uint8_t signPackedMantissa = source.read(1u);

  auto &vars = variables();
  vars.AS_FAC[0] = source.read(0u);
  vars.AS_FAC[1] = static_cast<std::uint8_t>(signPackedMantissa | 0x80u);
  vars.AS_FAC[2] = source.read(2u);
  vars.AS_FAC[3] = source.read(3u);
  vars.AS_FAC[4] = source.read(4u);
  vars.AS_FAC_SIGN = signPackedMantissa;
  vars.AS_FAC_EXTENSION = 0u;
}

static void storeFacToPackedRounded(std::uint16_t address) {
  variables().AS_VARPNT = address;
  AS_STORE_FACDB_YX_ROUNDED();
}

static void AS_POLYNOMIAL(std::uint16_t tableAddress) {
  storeFacToPackedRounded(kAsTemp2Address);

  const auto table = variables_const().pointer(tableAddress);
  std::uint8_t remainingTerms = table.read(0u);
  std::uint16_t coefficientAddress =
      static_cast<std::uint16_t>(tableAddress + 1u);

  loadFacFromPacked(coefficientAddress);
  while (remainingTerms != 0u) {
    loadArgFromPacked(kAsTemp2Address);
    AS_MATHTBL(kMathMulIdx).handler();

    coefficientAddress = static_cast<std::uint16_t>(coefficientAddress + 5u);
    loadArgFromPacked(coefficientAddress);
    AS_FADDT();
    --remainingTerms;
  }
}

static void AS_POLYNOMIAL_ODD(std::uint16_t tableAddress) {
  storeFacToPackedRounded(kAsTemp1Address);
  loadArgFromPacked(kAsTemp1Address);
  AS_MATHTBL(kMathMulIdx).handler();

  AS_POLYNOMIAL(tableAddress);

  loadArgFromPacked(kAsTemp1Address);
  AS_MATHTBL(kMathMulIdx).handler();
}

static std::int8_t AS_SIGN_FAC() {
  if (variables_const().AS_FAC[0] == 0u) {
    return 0;
  }
  return ((variables_const().AS_FAC_SIGN & 0x80u) != 0u) ? -1 : 1;
}

} // namespace

static void write_AS_DEST(std::uint16_t value) { variables().AS_DEST = value; }

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

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PDL (inclusive) .. AS_NXDIM (exclusive)
// Name normalization: none (assembler label AS_PDL kept verbatim).
void AS_PDL() {

  AS_CONINT();
  AS_SNGFLT(MON_PREAD());
}

// ---------------------------------------------------------------------------
// Stub implementations for function handlers not yet ported.
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_SGN (inclusive) .. AS_ABS (exclusive)
// Name normalization: none (assembler label AS_SGN kept verbatim).
void AS_SGN() {

  // ROM path: jsr AS_SIGN then fall through to AS_FLOAT to convert A=-1/0/+1 to
  // AS_FAC.
  std::int16_t signValue = 0;
  if (variables_const().AS_FAC[0] != 0u) {
    signValue = ((variables_const().AS_FAC_SIGN & 0x80u) != 0u) ? -1 : 1;
  }

  AS_GIVAYF(signValue);
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_INT (inclusive) .. AS_QINT_3 (exclusive)
// Name normalization: AS_INT -> AS_INT_fn (AS_INT is a C++ keyword).
void AS_INT_fn() {

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
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ABS (inclusive) .. AS_FCOMP (exclusive)
// Name normalization: none (assembler label AS_ABS kept verbatim).
void AS_ABS() {

  variables().AS_FAC_SIGN =
      static_cast<std::uint8_t>(variables_const().AS_FAC_SIGN >> 1u);
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_L_USR1 (inclusive) .. AS_L_USR2 (exclusive)
// Name normalization: none (assembler label AS_USR kept verbatim).
//
// The original ROM dispatches through a user-supplied machine-language
// vector in zero page ($0A-$0C). The current runtime has no machine-code
// execution bridge, so USR() is treated as an undefined function.
void AS_USR_impl() { AS_ERROR(AS_ERR_UNDEFFUNC); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_USR (inclusive) .. AS_CHARAC (exclusive)
// Name normalization: none (assembler label AS_USR kept verbatim).
//
// The original ROM dispatches through a user-supplied machine-language
// vector in zero page ($0A-$0C). The current runtime has no machine-code
// execution bridge, so USR() is treated as an undefined function.
void AS_USR() {
  ApplesoftNG::ExternalJumpDispatcher::JumpFromInstruction(
      ApplesoftNG::ExternalJumpDispatcher::ADDR_AS_USR);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FRE (inclusive) .. AS_ERROR (exclusive)
// Name normalization: wrapper name AS_FRE_fn used to avoid clashing with the
// existing AS_FRE implementation in core.
void AS_FRE_fn() { AS_FRE(); }
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_UNDFNC (inclusive) .. AS_DEF (exclusive)
// Name normalization: AS_ERROR_fn used for AS_UNFNC table entry $D7 (SCRN().
void AS_ERROR_fn() { AS_ERROR(AS_ERR_UNDEFFUNC); }
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_POS (inclusive) .. AS_SNGFLT (exclusive)
// Name normalization: none (assembler label AS_POS kept verbatim).
static void AS_PDL_fn() { AS_PDL(); }
void AS_POS() { AS_SNGFLT(variables_const().MON_CH); }
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_SQR (inclusive) .. AS_RND (exclusive)
// Name normalization: none (assembler label AS_SQR kept verbatim).
void AS_SQR() {
  AS_COPY_FAC_TO_ARG_ROUNDED();
  loadFacFromPacked(kAsConHalfAddress);
  AS_MATHTBL(kMathPowIdx).handler();
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_RND (inclusive) .. AS_LOG (exclusive)
// Name normalization: none (assembler label AS_RND kept verbatim).
//
// RND uses the packed FAC-format seed at zero-page RNDSEED ($c9-$cd). A
// zero argument returns the current seed unchanged; a negative argument
// reseeds the generator from the argument value; positive arguments advance
// the sequence.
void AS_RND() {
  constexpr std::uint16_t kRndSeedAddress = ApplesoftVariables::ZP_AS_RNDSEED;

  const std::int8_t argumentSign = AS_SIGN_FAC();

  if (argumentSign >= 0) {
    loadFacFromPacked(kRndSeedAddress);
    if (argumentSign == 0) {
      return;
    }

    loadArgFromPacked(kAsConRnd1Address);
    AS_MATHTBL(kMathMulIdx).handler();
    loadArgFromPacked(kAsConRnd2Address);
    AS_FADDT();
  }

  const std::uint8_t originalLowMantissa = variables_const().AS_FAC[4];
  variables().AS_FAC[4] = variables_const().AS_FAC[1];
  variables().AS_FAC[1] = originalLowMantissa;

  variables().AS_FAC_SIGN = 0u;
  variables().AS_FAC_EXTENSION = variables_const().AS_FAC[0];
  variables().AS_FAC[0] = 0x80u;
  AS_NORMALIZE_FAC_2();
  storeFacToPackedRounded(kRndSeedAddress);
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_LOG (inclusive) .. AS_EXP (exclusive)
// Name normalization: none (assembler label AS_LOG kept verbatim).
void AS_LOG() {
  const std::int8_t facSign = AS_SIGN_FAC();
  if (facSign <= 0) {
    AS_ERROR(AS_ERR_ILLQTY);
    return;
  }

  const std::int8_t unbiasedExponent =
      static_cast<std::int8_t>(variables_const().AS_FAC[0] - 0x7fu);

  variables().AS_FAC[0] = 0x80u;

  loadArgFromPacked(kAsConSqrHalfAddress);
  AS_FADDT();

  loadArgFromPacked(kAsConSqrTwoAddress);
  AS_MATHTBL(kMathDivIdx).handler();

  loadArgFromPacked(kAsConOneAddress);
  AS_FSUBT();

  AS_POLYNOMIAL_ODD(kAsPolyLogAddress);

  loadArgFromPacked(kAsConNegHalfAddress);
  AS_FADDT();

  storeFacToPackedRounded(kAsTemp3Address);
  AS_FLOAT(unbiasedExponent);
  loadArgFromPacked(kAsTemp3Address);
  AS_FADDT();

  loadArgFromPacked(kAsConLogTwoAddress);
  AS_MATHTBL(kMathMulIdx).handler();
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_EXP (inclusive) .. AS_COS (exclusive)
// Name normalization: none (assembler label AS_EXP kept verbatim).
void AS_EXP() {
  loadArgFromPacked(kAsConLogEAddress);
  AS_MATHTBL(kMathMulIdx).handler();

  // EXP range gate used in ROM before extracting integer/fractional parts.
  if (variables_const().AS_FAC[0] >= 0x88u) {
    if ((variables_const().AS_FAC_SIGN & 0x80u) != 0u) {
      AS_ZERO_FAC();
      return;
    }
    AS_ERROR(AS_ERR_OVERFLOW);
    return;
  }

  storeFacToPackedRounded(kAsTemp3Address);

  AS_INT_fn();
  const std::int8_t integralPart =
      static_cast<std::int8_t>(variables_const().AS_CHARAC);

  loadArgFromPacked(kAsTemp3Address);
  AS_FSUBT();
  AS_NEGOP();

  AS_POLYNOMIAL(kAsPolyExpAddress);

  const std::int16_t adjustedExponent =
      static_cast<std::int16_t>(variables_const().AS_FAC[0]) + integralPart;
  if (adjustedExponent <= 0) {
    AS_ZERO_FAC();
    return;
  }
  if (adjustedExponent > 0xff) {
    AS_ERROR(AS_ERR_OVERFLOW);
    return;
  }

  variables().AS_FAC[0] = static_cast<std::uint8_t>(adjustedExponent);
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_COS (inclusive) .. AS_SIN (exclusive)
// Name normalization: none (assembler label AS_COS kept verbatim).
static void AS_COS() {
  // ROM identity: COS(X) = SIN(X + PI/2).
  loadArgFromPacked(kAsConPiHalfAddress);
  AS_FADDT();
  AS_SIN();
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_SIN (inclusive) .. AS_TAN (exclusive)
// Name normalization: none (assembler label AS_SIN kept verbatim).
static void AS_SIN() {
  // Reduce angle to a circle fraction: X / (2*PI).
  AS_COPY_FAC_TO_ARG_ROUNDED();
  loadFacFromPacked(kAsConPiDoubleAddress);
  AS_MATHTBL(kMathDivIdx).handler();

  // Keep only fractional part.
  AS_COPY_FAC_TO_ARG_ROUNDED();
  AS_INT_fn();
  variables().AS_SGNCPR = 0u;
  AS_FSUBT();

  // Fold into first quarter before polynomial approximation.
  loadArgFromPacked(kAsQuarterAddress);
  AS_FSUBT();

  const bool savedSignNegative = (variables_const().AS_FAC_SIGN & 0x80u) != 0u;
  if (savedSignNegative) {
    AS_FADDH();
    if ((variables_const().AS_FAC_SIGN & 0x80u) == 0u) {
      AS_NEGOP();
    }
  } else {
    AS_NEGOP();
  }

  loadArgFromPacked(kAsQuarterAddress);
  AS_FADDT();

  if (savedSignNegative) {
    AS_NEGOP();
  }

  AS_POLYNOMIAL_ODD(kAsPolySinAddress);
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_TAN (inclusive) .. AS_ATN (exclusive)
// Name normalization: none (assembler label AS_TAN kept verbatim).
static void AS_TAN() {
  // TAN(X) = SIN(X) / COS(X), preserving FAC/ARG packed-float operations.
  storeFacToPackedRounded(kAsTemp1Address);

  AS_SIN();
  storeFacToPackedRounded(kAsTemp3Address);

  loadFacFromPacked(kAsTemp1Address);
  AS_COS();

  loadArgFromPacked(kAsTemp3Address);
  AS_MATHTBL(kMathDivIdx).handler();
}
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ATN (inclusive) .. AS_GENERIC_CHRGET (exclusive)
// Name normalization: none (assembler label AS_ATN kept verbatim).
static void AS_ATN() {
  const bool startedNegative = (variables_const().AS_FAC_SIGN & 0x80u) != 0u;
  if (startedNegative) {
    AS_NEGOP();
  }

  const bool wasAtLeastOne = variables_const().AS_FAC[0] >= 0x81u;
  if (wasAtLeastOne) {
    loadArgFromPacked(kAsConOneAddress);
    AS_MATHTBL(kMathDivIdx).handler();
  }

  AS_POLYNOMIAL_ODD(kAsPolyAtnAddress);

  if (wasAtLeastOne) {
    loadArgFromPacked(kAsConPiHalfAddress);
    AS_FSUBT();
  }

  if (startedNegative) {
    AS_NEGOP();
  }
}
static void AS_PEEK_fn() { AS_PEEK(); }
// AS_Labels: AS_VAL (inclusive) .. AS_L_VAL_1 (exclusive)
// Name normalization: none (assembler label AS_VAL kept verbatim).
static void AS_LEN_fn() { AS_LEN(); }
static void AS_STR_fn() { AS_STR(); }
void AS_VAL() {
  (void)AS_GETSTR();
  if (variables_const().AS_FAC[4] == 0u) {
    variables().AS_FAC[0] = 0u;
    return;
  }

  AS_L_VAL_1();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_L_VAL_1 (inclusive) .. AS_L_VAL_2 (exclusive)
// Name normalization: none (assembler label AS_L_VAL_1 kept verbatim).
static void AS_L_VAL_1() {
  // Pointer candidates lifted:
  // - AS_TXTPTR and AS_STRNG2 are each used as unified 16-bit text pointers.
  // - AS_INDEX is the unified string-start pointer returned by AS_GETSTR.
  // - AS_DEST stores the unified end-of-string pointer for temporary NUL swap.
  const std::uint16_t savedTextPointer = variables_const().AS_TXTPTR;
  variables().AS_STRNG2 = savedTextPointer;

  const std::uint16_t stringStart = variables_const().AS_INDEX;
  variables().AS_TXTPTR = stringStart;

  const std::uint8_t stringLength = variables_const().AS_FAC[4];
  const std::uint16_t endAddress =
      static_cast<std::uint16_t>(stringStart + stringLength);
  write_AS_DEST(endAddress);

  const std::uint8_t savedByte = variables_const().readByte(endAddress);
  variables().writeByte(endAddress, 0u);

  (void)AS_CHRGOT();
  AS_FIN();

  variables().writeByte(endAddress, savedByte);
  AS_L_VAL_2();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_L_VAL_2 (inclusive) .. AS_POINT (exclusive)
// Name normalization: none (assembler label AS_L_VAL_2 kept verbatim).
static void AS_L_VAL_2() {
  variables().AS_TXTPTR = variables_const().AS_STRNG2;
}
static void AS_ASC_fn() { AS_ASC(); }
static void AS_CHRSTR_fn() { AS_CHRSTR(); }
static void AS_LEFTSTR_fn() { AS_LEFTSTR(); }
static void AS_RIGHTSTR_fn() { AS_RIGHTSTR(); }
static void AS_MIDSTR_fn() { AS_MIDSTR(); }

// ---------------------------------------------------------------------------
// Dispatch table for Applesoft unary/built-in function tokens $D2-$EA.
// Index = token - $D2. Direct addresses — no RTS-dispatch -1 adjustment.
// AS_Labels: AS_UNFNC (inclusive) .. AS_MATHTBL (exclusive)
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
