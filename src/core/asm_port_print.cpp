#include "core/asm_port_print.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_strlit.hpp"
#include "platform/asm_port_outdo.hpp"

#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t AS_CHRGET();
std::uint8_t AS_CHRGOT();
std::uint8_t AS_GETBYT();
void AS_FRMEVL();
bool IsDirectMode();
void AS_RESTART();
void AS_SYNERR();

void PrintDecimalUnsigned(std::uint16_t value) {
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// buffer; on return (A, Y) point to the buffer.  Used inside the AS_PRINT2
// number path.
// TODO(asm-port): port AS_FOUT.
static void AS_FOUT() {}

// AS_GTBYTC: advance AS_TXTPTR (via AS_CHRGET), evaluate a numeric expression,
// and return the result clamped to a byte (0-255) in the X register equivalent.
// After this call, AS_CHRGOT() returns the character immediately following the
// expression (expected to be ')' by the callers in this file).
static std::uint8_t AS_GTBYTC_PRINT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GTBYTC (inclusive) .. AS_GETBYT (exclusive)
  // Name normalization: none (assembler label AS_GTBYTC kept verbatim).

  AS_CHRGET();
  return AS_GETBYT();
}

// AS_FREFAC: dereference the string descriptor at AS_FAC[3]/AS_FAC[4],
// optionally release the temporary descriptor, store the data pointer in
// AS_INDEX, and return the string length.
// TODO(asm-port): AS_FRETMS (temporary-release part) not yet implemented.
std::uint8_t AS_FREFAC() {
  const std::uint16_t descriptorAddress = ApplesoftVariables::makeWord(
      variables_const().AS_FAC[3], variables_const().AS_FAC[4]);
  const auto descriptorPtr = variables_const().pointer(descriptorAddress);

  // String descriptor layout: [length, data_lo, data_hi]
  const std::uint8_t length = descriptorPtr.read();

  variables().AS_INDEX = ApplesoftVariables::makeWord(descriptorPtr.read(1u),
                                                      descriptorPtr.read(2u));

  return length;
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
  AS_STRLIT(0x0000u);
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
