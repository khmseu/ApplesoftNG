#include "core/asm_port_print.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_core.hpp"
#include "core/asm_port_error_handling.hpp"
#include "core/asm_port_parser.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_strlit.hpp"
#include "platform/asm_port_outdo.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>

namespace applesoft::asm_port {
// ---------------------------------------------------------------------------
// facToDouble: convert Applesoft 5-byte FAC to native double.
//
// Applesoft float format:
//   FAC[0] = biased exponent (excess-128); 0 means value is zero
//   FAC[1..4] = mantissa, with implicit leading 1 (bit 7 of FAC[1] = MSB)
//   FAC_SIGN = 0x00 for positive, 0xFF for negative
//
// Value = (-1)^sign * 2^(exp-128) * (0.mantissa_with_leading_1)
// i.e.  value = (-1)^sign * 2^(exp-129) * (1 + mantissa/2^31)
// ---------------------------------------------------------------------------
static double facToDouble() {
  const auto &cv = variables_const();
  const std::uint8_t exp8 = cv.AS_FAC[0];
  if (exp8 == 0u)
    return 0.0;
  const std::uint32_t mantissa =
      (static_cast<std::uint32_t>(cv.AS_FAC[1]) << 24u) |
      (static_cast<std::uint32_t>(cv.AS_FAC[2]) << 16u) |
      (static_cast<std::uint32_t>(cv.AS_FAC[3]) << 8u) |
      static_cast<std::uint32_t>(cv.AS_FAC[4]);
  // Mantissa has implicit leading 1 at bit 31; actual fraction = mantissa/2^32
  // biased exponent: real exponent = exp8 - 128; value = 2^(exp8-128) *
  // (mant/2^32)
  const double fraction = static_cast<double>(mantissa) / 4294967296.0; // /2^32
  const double value = std::ldexp(fraction, static_cast<int>(exp8) - 128);
  return (cv.AS_FAC_SIGN != 0u) ? -value : value;
}

// ---------------------------------------------------------------------------
// foutImpl: core float-to-string conversion, matching Applesoft FOUT/FOUT_1.
//
// startAddress: absolute destination address for the first emitted character.
// FOUT uses 0x0101; STR$ entry (FOUT_1) uses 0x00FF.
// On return writes null-terminated string, and updates AS_STRNG2 to the
// address of the last non-null character.
// ---------------------------------------------------------------------------
static void foutImpl(std::uint16_t startAddress) {
  // Each byte is written to 0x0100+index (the 6502 stack page).
  // The assembly writes to STACK-1,Y where Y is 1-based; we use 0-based index
  // into the stack page (0x0100 = index 0, 0x0101 = index 1, ...).
  //
  // We build into a local buffer then copy, to avoid partial writes on error.
  constexpr std::uint8_t kMaxLen = 20u; // sufficient for any Applesoft number
  char buf[kMaxLen + 1u];
  std::uint8_t len = 0u;

  auto emit = [&](char c) {
    if (len < kMaxLen)
      buf[len++] = c;
  };

  const double val = facToDouble();
  const bool isNeg = val < 0.0;
  const double absVal = isNeg ? -val : val;

  if (isNeg)
    emit('-');

  if (absVal == 0.0) {
    emit('0');
  } else {
    // Determine the power-of-10 exponent (floor of log10).
    // TMPEXP in ROM = number of digits before decimal point minus 1.
    // Applesoft uses exactly 9 significant digits.
    // Decimal form when -2 <= tmpexp <= 9 (i.e. 0.01 <= |val| < 1e10).
    // Outside that range, exponential form "d.xxxxxxxxE±yy".
    constexpr int kDigits = 9;
    // Compute tmpexp: floor(log10(absVal)).
    int tmpexp = 0;
    if (absVal != 0.0) {
      tmpexp = static_cast<int>(std::floor(std::log10(absVal)));
    }
    // Normalise to 9-digit integer: absVal / 10^(tmpexp-8) rounded.
    // After QINT the ROM has a 32-bit integer in FAC[1..4].
    double scaled =
        absVal * std::pow(10.0, static_cast<double>(kDigits - 1 - tmpexp));
    // Round to nearest integer.
    long long iVal = static_cast<long long>(scaled + 0.5);
    // Clamp to [100000000, 999999999].
    if (iVal >= 1000000000LL) {
      iVal /= 10;
      tmpexp++;
    }
    if (iVal < 100000000LL && iVal > 0) {
      iVal *= 10;
      tmpexp--;
    }
    // expon = tmpexp - (kDigits-1), adjusted to 0 for decimal form.
    // Applesoft's EXPON = TMPEXP + 10 - 2 - (digits before decimal point)
    // We need to decide: decimal or exponential form?
    // Decimal form: tmpexp in [-2, 9].
    const bool decimalForm = (tmpexp >= -2 && tmpexp <= 9);
    // Build 9-digit string, with decimal point inserted.
    char digits[kDigits + 1];
    std::snprintf(digits, sizeof(digits), "%09lld",
                  static_cast<long long>(iVal < 0 ? 0 : iVal));
    if (decimalForm) {
      // dotPos: number of digits before the decimal point.
      // tmpexp==0 means value is 1.xxxxxxxx (1 digit before dot).
      // tmpexp==1 means 2 digits before dot, etc.
      const int dotPos = tmpexp + 1; // digits before decimal point
      if (dotPos <= 0) {
        // Value like 0.0x...  => "0." followed by (-dotPos) zeros then digits
        emit('0');
        emit('.');
        for (int z = 0; z < -dotPos; ++z)
          emit('0');
        for (int d = 0; d < kDigits; ++d)
          emit(digits[d]);
      } else {
        // dotPos >= 1: emit dotPos digits, then '.', then rest
        for (int d = 0; d < kDigits; ++d) {
          if (d == dotPos)
            emit('.');
          emit(digits[d]);
        }
        // If dotPos >= kDigits, no decimal point was emitted (pure integer).
      }
    } else {
      // Exponential form: "d.xxxxxxxxE±yy"
      emit(digits[0]);
      emit('.');
      for (int d = 1; d < kDigits; ++d)
        emit(digits[d]);
      // E value = tmpexp
      const int eVal = tmpexp;
      emit('E');
      if (eVal < 0) {
        emit('-');
        const int absE = -eVal;
        emit(static_cast<char>('0' + absE / 10));
        emit(static_cast<char>('0' + absE % 10));
      } else {
        emit('+');
        emit(static_cast<char>('0' + eVal / 10));
        emit(static_cast<char>('0' + eVal % 10));
      }
    }
    // Strip trailing zeros and trailing decimal point.
    // Only strip within the mantissa (not from E notation digits).
    if (decimalForm) {
      // Walk back from end, removing '0' and possibly '.'.
      while (len > 0u && buf[len - 1u] == '0')
        --len;
      if (len > 0u && buf[len - 1u] == '.')
        --len;
      // Handle negative: if we stripped everything after '-', that shouldn't
      // happen for non-zero absVal, but guard anyway.
    } else {
      // For exponential: strip trailing zeros from mantissa only.
      // Find 'E' position.
      std::uint8_t ePos = 0u;
      for (std::uint8_t i = 0u; i < len; ++i) {
        if (buf[i] == 'E') {
          ePos = i;
          break;
        }
      }
      // Trim zeros before ePos.
      std::uint8_t mantEnd = ePos;
      while (mantEnd > 0u && buf[mantEnd - 1u] == '0')
        --mantEnd;
      if (mantEnd > 0u && buf[mantEnd - 1u] == '.')
        --mantEnd;
      // Shift E-part to follow mantissa.
      const std::uint8_t tailLen = static_cast<std::uint8_t>(len - ePos);
      for (std::uint8_t i = 0u; i < tailLen; ++i)
        buf[mantEnd + i] = buf[ePos + i];
      len = static_cast<std::uint8_t>(mantEnd + tailLen);
    }
  }
  buf[len] = '\0';

  // Write to the selected destination start address.
  for (std::uint8_t i = 0u; i <= len; ++i) {
    WriteProgramByte(static_cast<std::uint16_t>(startAddress + i),
                     static_cast<std::uint8_t>(buf[i]));
  }
  variables().AS_STRNG2 = static_cast<std::uint16_t>(startAddress + len);
}

static void PrintDecimalUnsigned(std::uint16_t value) {
  char digits[5];
  std::uint8_t length = 0;

  do {
    digits[length++] = static_cast<char>('0' + static_cast<char>(value % 10u));
    value = static_cast<std::uint16_t>(value / 10u);
  } while (value != 0u);

  while (length != 0u) {
    --length;
    AS_OUTDO(static_cast<std::uint8_t>(digits[length]));
  }
}

void AS_LINPRT() { PrintDecimalUnsigned(variables_const().AS_CURLIN); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_INPRT (inclusive) .. AS_GO_STROUT (exclusive)
// Name normalization: none (assembler label AS_INPRT kept verbatim).
void AS_INPRT() {
  const std::string_view prefix = AS_QT_ERROR(AS_QT_IN_INDEX);
  for (const char ch : prefix) {
    AS_OUTDO(static_cast<std::uint8_t>(ch));
  }
  AS_LINPRT();
}

void AS_STROUT(std::string_view text) {
  for (const char ch : text) {
    AS_OUTDO(static_cast<std::uint8_t>(ch));
  }
}

// ---------------------------------------------------------------------------
// Forward declarations for callees not yet in public headers.
// ---------------------------------------------------------------------------

// AS_FOUT: convert the floating-point value in AS_FAC to an AS_ASCII string
// buffer; on return (A, Y) point to the buffer. Used inside the AS_PRINT2
// number path.
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FOUT (inclusive) .. AS_SQR (exclusive)
// Name normalization: none (assembler label AS_FOUT kept verbatim).
static void AS_FOUT() {
  // FOUT entry: Y=1, string starts at STACK+1.
  foutImpl(0x0101u);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FOUT_1 (inclusive) .. AS_FOUT_3 (exclusive)
// Name normalization: none (assembler label AS_FOUT_1 kept verbatim).
void AS_FOUT_1() {
  // FOUT_1 entry: Y=0, string starts at STACK-1 (used by STR$).
  foutImpl(0x00ffu);
}

// AS_GTBYTC: advance AS_TXTPTR (via AS_CHRGET), evaluate a numeric expression,
// and return the result clamped to a byte (0-255) in the X register equivalent.
// After this call, AS_CHRGOT() returns the character immediately following the
// expression (expected to be ')' by the callers in this file).
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GTBYTC (inclusive) .. AS_GETBYT (exclusive)
// Name normalization: AS_GTBYTC_PRINT maps to AS_GTBYTC.
static std::uint8_t AS_GTBYTC_PRINT() {
  AS_CHRGET();
  return AS_GETBYT();
}

// ---------------------------------------------------------------------------
// Token constants used in AS_PRINT2.
// ---------------------------------------------------------------------------

constexpr std::uint8_t AS_TOKEN_TAB = 0xc0u; // TAB(
constexpr std::uint8_t AS_TOKEN_SPC = 0xc3u; // SPC(

// ---------------------------------------------------------------------------
// AS_STRPRT
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_STRPRT (inclusive) .. AS_OUTSP (exclusive)
// Name normalization: none (assembler label AS_STRPRT kept verbatim).
void AS_STRPRT() {
  // jsr AS_FREFAC — obtain string length (A) and data address (AS_INDEX)
  const std::uint8_t length = AS_FREFAC();
  const auto strPtr = variables_const().pointer(variables_const().AS_INDEX);

  // tax / ldy #0 / inx: set up counter X = length+1 for dex-first loop.
  // The original uses a dec-before-test loop: inx then dex/beq.
  // In C++ the equivalent is iterating 'length' times starting from index 0.
  for (std::uint8_t i = 0u; i < length; ++i) {
    const std::uint8_t ch = strPtr.read(i);
    AS_OUTDO(ch);
  }
  // Note: The original contains a three-instruction block
  //   cmp #$0d / bne AS_L_STRPRT_1 / jsr AS_NEGATE / jmp AS_L_STRPRT_1
  // which the source listing explicitly marks "<<< ABOVE THREE AS_LINES ARE
  // USELESS >>>". It is intentionally omitted here.
}

// ---------------------------------------------------------------------------
// AS_STROUT
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_STROUT (inclusive) .. AS_STRPRT (exclusive)
// Name normalization: none (assembler label AS_STROUT kept verbatim).
void AS_STROUT(std::uint16_t address) {
  // jsr AS_STRLIT — build a AS_FAC string descriptor from the address.
  AS_STRLIT(address);
  // Fall through to AS_STRPRT (range end is exclusive; modeled as call).
  AS_STRPRT();
}

// ---------------------------------------------------------------------------
// Internal helper: shared AS_PRINT list loop body
// ---------------------------------------------------------------------------
//
// Processes items in a AS_PRINT list starting with character 'a'.
// When 'expr_cr' is true the caller is the AS_PRINT path (end-of-list after
// an expression triggers a carriage return); when false it is the AS_PRINT2
// path (end-of-list after a separator returns silently).
//
// This function captures the control-flow merge of AS_PR_STRING → AS_PRINT →
// AS_PRINT2 that exists in the 6502 source via fall-through and jmp.

static void PrintNumericExpression() {
  // jsr AS_FOUT — convert the floating-point value to an AS_ASCII buffer.
  AS_FOUT();
  // jsr AS_STRLIT — wrap the buffer as a temporary AS_FAC string descriptor.
  // FOUT writes starting at 0x0101; pass that address to STRLIT.
  AS_STRLIT(0x0101u);
  // jmp AS_PR_STRING — print the temporary string and re-enter the list loop.
  AS_STRPRT();
}

static void print_list_loop(std::uint8_t a, bool expr_cr) {
  while (true) {
    if (a == AS_TOKEN_TAB) {
      // cmp #AS_TOKEN_TAB / beq AS_PR_TAB_OR_SPC (C=1 for TAB)
      AS_PR_TAB_OR_SPC(true);
    } else if (a == AS_TOKEN_SPC) {
      // cmp #AS_TOKEN_SPC / beq AS_PR_TAB_OR_SPC (C=0 for SPC)
      AS_PR_TAB_OR_SPC(false);
    } else if (a == static_cast<std::uint8_t>(',' & 0x7fu)) {
      // cmp #(",",&$7f) / beq AS_PR_COMMA
      AS_PR_COMMA();
    } else if (a == static_cast<std::uint8_t>(';' & 0x7fu)) {
      // cmp #(";"&$7f) / beq AS_PR_NEXT_CHAR — no additional action
    } else {
      // jsr AS_FRMEVL — evaluate expression into AS_FAC / AS_VALTYP
      AS_FRMEVL();

      if (variables_const().AS_VALTYP & 0x80u) {
        // bit AS_VALTYP / bmi AS_PR_STRING — string result
        AS_STRPRT();
      } else {
        PrintNumericExpression();
      }

      // AS_PR_STRING: jsr AS_CHRGOT — get char after the printed expression
      a = AS_CHRGOT();

      // Fall-through to AS_PRINT (T:dad5): beq AS_CRDO
      if (a == 0u) {
        if (expr_cr) {
          AS_CRDO();
        }
        return;
      }
      // Fall-through to AS_PRINT2 (T:dad7): beq AS_RTS_8 — not taken since a !=
      // 0
      continue; // loop back without calling AS_CHRGET again
    }

    // AS_PR_NEXT_CHAR: jsr AS_CHRGET / jmp AS_PRINT2
    a = AS_CHRGET();
    if (a == 0u) {
      // AS_PRINT2: beq AS_RTS_8 — end of list after separator: no CR
      return;
    }
    // AS_PRINT2 re-entry: beq AS_RTS_8 not taken, continue loop
  }
}

// ---------------------------------------------------------------------------
// AS_PR_STRING
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PR_STRING (inclusive) .. AS_PRINT (exclusive)
// Name normalization: none (assembler label AS_PR_STRING kept verbatim).
void AS_PR_STRING() {
  // jsr AS_STRPRT — print string at AS_FAC
  AS_STRPRT();
  // jsr AS_CHRGOT — get next character in print list
  const std::uint8_t a = AS_CHRGOT();
  // Fall-through to AS_PRINT (T:dad5): beq AS_CRDO — end-of-list gives CR
  if (a == 0u) {
    AS_CRDO();
    return;
  }
  // Fall-through to AS_PRINT2 (T:dad7): a != 0, continue print list
  AS_PRINT2(a);
}

// ---------------------------------------------------------------------------
// AS_PRINT
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PRINT (inclusive) .. AS_PRINT2 (exclusive)
// Name normalization: none (assembler label AS_PRINT kept verbatim).
void AS_PRINT(std::uint8_t a) {
  // beq AS_CRDO (T:dad5) — empty print list prints a carriage return
  if (a == 0u) {
    AS_CRDO();
    return;
  }
  // Fall through to AS_PRINT2: process print list with end-of-list CR
  AS_PRINT2(a);
}

// ---------------------------------------------------------------------------
// AS_PRINT2
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PRINT2 (inclusive) .. AS_CRDO (exclusive)
// Name normalization: none (assembler label AS_PRINT2 kept verbatim).
void AS_PRINT2(std::uint8_t a) {
  // beq AS_RTS_8 (T:dad7) — end of list after separator: no CR
  if (a == 0u) {
    return;
  }
  print_list_loop(a, /*expr_cr=*/true);
}

// ---------------------------------------------------------------------------
// AS_CRDO
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CRDO (inclusive) .. AS_NEGATE (exclusive)
// Name normalization: none (assembler label AS_CRDO kept verbatim).
void AS_CRDO() {
  // lda #$0d / jsr AS_OUTDO — print carriage-return character
  const std::uint8_t a = AS_OUTDO(0x0du);
  // Fall-through to AS_NEGATE (range end is exclusive; modeled as call).
  // AS_NEGATE: eor #$ff / rts — noted "<<< WHY??? >>>" in source.
  // The return value of AS_NEGATE is not used by AS_CRDO's callers in C++.
  (void)AS_NEGATE(a);
}

// ---------------------------------------------------------------------------
// AS_NEGATE
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_NEGATE (inclusive) .. AS_PR_COMMA (exclusive)
// Name normalization: none (assembler label AS_NEGATE kept verbatim).
// The source listing comments this entry "<<< WHY??? >>>".
// It is reachable both as a fall-through from AS_CRDO and as a direct call
// from AS_STRPRT's (useless) CR-detection block.
std::uint8_t AS_NEGATE(std::uint8_t a) {
  // eor #$ff / rts
  return a ^ 0xffu;
}

// ---------------------------------------------------------------------------
// AS_PR_COMMA
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PR_COMMA (inclusive) .. AS_PR_TAB_OR_SPC (exclusive)
// Name normalization: none (assembler label AS_PR_COMMA kept verbatim).
// Note: The original uses 24 ($18) as the zone-change threshold; the source
// comments this as "<<< BUG: IT SHOULD BE 32 >>>".  The value is preserved
// faithfully here.
void AS_PR_COMMA() {
  const std::uint8_t ch = variables_const().MON_CH; // lda MON_CH ($24)

  if (ch >= 24u) {
    // cmp #24 / bcs: at or past last zone — print CR, start new line
    // jsr AS_CRDO / bne AS_PR_NEXT_CHAR (always, since AS_CRDO returns $f2 ≠ 0)
    AS_CRDO();
  } else {
    // adc #16 / and #$f0 — advance to next comma zone boundary (16 or 32)
    // Carry is 0 (ch < 24 < 128, adc with C=0 never produces carry here).
    const std::uint8_t new_ch =
        static_cast<std::uint8_t>((static_cast<unsigned>(ch) + 16u) & 0xf0u);
    variables().MON_CH = new_ch;
  }
  // Both branches fall through to AS_PR_NEXT_CHAR in the original.
  // AS_PR_NEXT_CHAR (jsr AS_CHRGET / jmp AS_PRINT2) is handled by the AS_PRINT2
  // loop caller.
}

// ---------------------------------------------------------------------------
// AS_PR_TAB_OR_SPC
// ---------------------------------------------------------------------------

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PR_TAB_OR_SPC (inclusive) .. AS_STROUT (exclusive)
// Name normalization: none (assembler label AS_PR_TAB_OR_SPC kept verbatim).
// This range also contains AS_PR_NEXT_CHAR (T:db2f) and AS_DOSPC/AS_NXSPC
// (T:db35). AS_PR_NEXT_CHAR (jsr AS_CHRGET / jmp AS_PRINT2) is the loop-back
// mechanism within the AS_PRINT2 caller; it is handled there rather than here.
void AS_PR_TAB_OR_SPC(bool is_tab) {
  // php — save carry (is_tab encodes this)
  // jsr AS_GTBYTC — advance, evaluate arg, result clamped to byte in X-reg
  const std::uint8_t n = AS_GTBYTC_PRINT();

  // cmp #(")"&$7f) = $29 — check for closing parenthesis
  const std::uint8_t next = AS_CHRGOT();
  if (next != static_cast<std::uint8_t>(')' & 0x7fu)) {
    // jmp AS_SYNERR
    AS_SYNERR();
    return;
  }

  // plp — restore carry (is_tab)
  std::uint8_t spaces_to_print = 0u;

  if (is_tab) {
    // TAB( path: C=1
    // dex / txa / sbc MON_CH (with C=1 from plp): A = n-1-MON_CH
    // bcc AS_PR_NEXT_CHAR — if MON_CH >= n, already past column
    const std::uint8_t ch = variables_const().MON_CH;
    if (ch >= n) {
      // bcc: fall through to AS_PR_NEXT_CHAR (caller handles)
      return;
    }
    // tax / inx: X = n - MON_CH spaces needed
    spaces_to_print = static_cast<std::uint8_t>(n - ch);
  } else {
    // SPC( path: C=0 — inx makes X = n+1 for dex-first loop → prints n spaces
    spaces_to_print = n;
  }

  // AS_NXSPC: dex / bne AS_DOSPC — loop: print spaces_to_print spaces
  // AS_DOSPC: jsr AS_OUTSP / bne AS_NXSPC (always, AS_OUTSP returns $20 ≠ 0)
  for (std::uint8_t i = 0u; i < spaces_to_print; ++i) {
    AS_OUTSP();
  }

  // Fall through to AS_PR_NEXT_CHAR (caller handles jsr AS_CHRGET / jmp
  // AS_PRINT2).
}

} // namespace applesoft::asm_port
