#include "core/asm_port_print.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_strlit.hpp"
#include "platform/asm_port_outdo.hpp"

#include <cstdint>

namespace applesoft::asm_port {

// ---------------------------------------------------------------------------
// Forward declarations for callees not yet in public headers.
// ---------------------------------------------------------------------------

// CHRGET ($b1/$00 in ROM): advance TXTPTR and return the next character.
// TODO(asm-port): route through the real CHRGET port once available.
static std::uint8_t CHRGET() { return 0; }

// CHRGOT ($b7/$00 in ROM): return the current character without advancing.
// TODO(asm-port): route through the real CHRGOT port once available.
static std::uint8_t CHRGOT() { return 0; }

// FRMEVL: evaluate the expression at TXTPTR; result goes into FAC / VALTYP.
// TODO(asm-port): port FRMEVL.
static void FRMEVL() {}

// FOUT: convert the floating-point value in FAC to an ASCII string buffer;
// on return (A, Y) point to the buffer.  Used inside the PRINT2 number path.
// TODO(asm-port): port FOUT.
static void FOUT() {}

// SYNERR: raise a syntax error.
// TODO(asm-port): port SYNERR.
static void SYNERR() {}

// HANDLERR: dispatch to the ON ERR handler with error code in X-register.
// TODO(asm-port): port HANDLERR.
static void HANDLERR() {}

// GTBYTC: advance TXTPTR (via CHRGET), evaluate a numeric expression, and
// return the result clamped to a byte (0-255) in the X register equivalent.
// After this call, CHRGOT() returns the character immediately following the
// expression (expected to be ')' by the callers in this file).
// TODO(asm-port): port GTBYTC / GETBYT / CONINT chain.
static std::uint8_t GTBYTC() { return 0; }

// FREFAC: dereference the string descriptor at FAC[3]/FAC[4], optionally
// release the temporary descriptor, store the data pointer in INDEX, and
// return the string length.
// TODO(asm-port): FRETMS (temporary-release part) not yet implemented.
 std::uint8_t FREFAC() {
    const std::uint16_t descriptorAddress =
        ApplesoftVariables::makeWord(variables_const().FAC[3], variables_const().FAC[4]);
    const auto descriptorPtr = variables_const().pointer(descriptorAddress);

    // String descriptor layout: [length, data_lo, data_hi]
    const std::uint8_t length = descriptorPtr.read();

    variables().INDEX = ApplesoftVariables::makeWord(descriptorPtr.read(1u), descriptorPtr.read(2u));

    return length;
}

// ---------------------------------------------------------------------------
// Token constants used in PRINT2.
// ---------------------------------------------------------------------------

constexpr std::uint8_t TOKEN_TAB = 0xc0u; // TAB(
constexpr std::uint8_t TOKEN_SPC = 0xc3u; // SPC(

// ---------------------------------------------------------------------------
// STRPRT
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRPRT (inclusive) .. OUTSP (exclusive)
// Name normalization: none (assembler label STRPRT kept verbatim).
void STRPRT() {
    // jsr FREFAC — obtain string length (A) and data address (INDEX)
    const std::uint8_t length = FREFAC();
    const auto strPtr = variables_const().pointer(variables_const().INDEX);

    // tax / ldy #0 / inx: set up counter X = length+1 for dex-first loop.
    // The original uses a dec-before-test loop: inx then dex/beq.
    // In C++ the equivalent is iterating 'length' times starting from index 0.
    for (std::uint8_t i = 0u; i < length; ++i) {
        const std::uint8_t ch = strPtr.read(i);
        OUTDO(ch);
    }
    // Note: The original contains a three-instruction block
    //   cmp #$0d / bne L_STRPRT_1 / jsr NEGATE / jmp L_STRPRT_1
    // which the source listing explicitly marks "<<< ABOVE THREE LINES ARE USELESS >>>".
    // It is intentionally omitted here.
}

// ---------------------------------------------------------------------------
// STROUT
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STROUT (inclusive) .. STRPRT (exclusive)
// Name normalization: none (assembler label STROUT kept verbatim).
void STROUT(std::uint16_t address) {
    // jsr STRLIT — build a FAC string descriptor from the address.
    STRLIT(address);
    // Fall through to STRPRT (range end is exclusive; modeled as call).
    STRPRT();
}

void STROUT(std::uint8_t a, std::uint8_t y) {
    STROUT(ApplesoftVariables::makeWord(a, y));
}

// ---------------------------------------------------------------------------
// Internal helper: shared PRINT list loop body
// ---------------------------------------------------------------------------
//
// Processes items in a PRINT list starting with character 'a'.
// When 'expr_cr' is true the caller is the PRINT path (end-of-list after
// an expression triggers a carriage return); when false it is the PRINT2
// path (end-of-list after a separator returns silently).
//
// This function captures the control-flow merge of PR_STRING → PRINT → PRINT2
// that exists in the 6502 source via fall-through and jmp.

static void print_list_loop(std::uint8_t a, bool expr_cr) {
    while (true) {
        if (a == TOKEN_TAB) {
            // cmp #TOKEN_TAB / beq PR_TAB_OR_SPC (C=1 for TAB)
            PR_TAB_OR_SPC(true);
        } else if (a == TOKEN_SPC) {
            // cmp #TOKEN_SPC / beq PR_TAB_OR_SPC (C=0 for SPC)
            PR_TAB_OR_SPC(false);
        } else if (a == static_cast<std::uint8_t>(',' & 0x7fu)) {
            // cmp #(",",&$7f) / beq PR_COMMA
            PR_COMMA();
        } else if (a == static_cast<std::uint8_t>(';' & 0x7fu)) {
            // cmp #(";"&$7f) / beq PR_NEXT_CHAR — no additional action
        } else {
            // jsr FRMEVL — evaluate expression into FAC / VALTYP
            FRMEVL();

            if (variables_const().VALTYP & 0x80u) {
                // bit VALTYP / bmi PR_STRING — string result
                STRPRT();
            } else {
                // jsr FOUT — convert float to ASCII buffer
                FOUT();
                // jsr STRLIT — wrap buffer as FAC string descriptor.
                // TODO(asm-port): pass actual (a, y) from FOUT output buffer.
                STRLIT(0x0000u);
                // jmp PR_STRING (which calls STRPRT then re-enters PRINT)
                STRPRT();
            }

            // PR_STRING: jsr CHRGOT — get char after the printed expression
            a = CHRGOT();

            // Fall-through to PRINT (T:dad5): beq CRDO
            if (a == 0u) {
                if (expr_cr) { CRDO(); }
                return;
            }
            // Fall-through to PRINT2 (T:dad7): beq RTS_8 — not taken since a != 0
            continue; // loop back without calling CHRGET again
        }

        // PR_NEXT_CHAR: jsr CHRGET / jmp PRINT2
        a = CHRGET();
        if (a == 0u) {
            // PRINT2: beq RTS_8 — end of list after separator: no CR
            return;
        }
        // PRINT2 re-entry: beq RTS_8 not taken, continue loop
    }
}

// ---------------------------------------------------------------------------
// PR_STRING
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PR_STRING (inclusive) .. PRINT (exclusive)
// Name normalization: none (assembler label PR_STRING kept verbatim).
void PR_STRING() {
    // jsr STRPRT — print string at FAC
    STRPRT();
    // jsr CHRGOT — get next character in print list
    const std::uint8_t a = CHRGOT();
    // Fall-through to PRINT (T:dad5): beq CRDO — end-of-list gives CR
    if (a == 0u) {
        CRDO();
        return;
    }
    // Fall-through to PRINT2 (T:dad7): a != 0, continue print list
    PRINT2(a);
}

// ---------------------------------------------------------------------------
// PRINT
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PRINT (inclusive) .. PRINT2 (exclusive)
// Name normalization: none (assembler label PRINT kept verbatim).
void PRINT(std::uint8_t a) {
    // beq CRDO (T:dad5) — empty print list prints a carriage return
    if (a == 0u) {
        CRDO();
        return;
    }
    // Fall through to PRINT2: process print list with end-of-list CR
    PRINT2(a);
}

// ---------------------------------------------------------------------------
// PRINT2
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PRINT2 (inclusive) .. CRDO (exclusive)
// Name normalization: none (assembler label PRINT2 kept verbatim).
void PRINT2(std::uint8_t a) {
    // beq RTS_8 (T:dad7) — end of list after separator: no CR
    if (a == 0u) {
        return;
    }
    print_list_loop(a, /*expr_cr=*/true);
}

// ---------------------------------------------------------------------------
// CRDO
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CRDO (inclusive) .. NEGATE (exclusive)
// Name normalization: none (assembler label CRDO kept verbatim).
void CRDO() {
    // lda #$0d / jsr OUTDO — print carriage-return character
    const std::uint8_t a = OUTDO(0x0du);
    // Fall-through to NEGATE (range end is exclusive; modeled as call).
    // NEGATE: eor #$ff / rts — noted "<<< WHY??? >>>" in source.
    // The return value of NEGATE is not used by CRDO's callers in C++.
    (void)NEGATE(a);
}

// ---------------------------------------------------------------------------
// NEGATE
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: NEGATE (inclusive) .. PR_COMMA (exclusive)
// Name normalization: none (assembler label NEGATE kept verbatim).
// The source listing comments this entry "<<< WHY??? >>>".
// It is reachable both as a fall-through from CRDO and as a direct call
// from STRPRT's (useless) CR-detection block.
std::uint8_t NEGATE(std::uint8_t a) {
    // eor #$ff / rts
    return a ^ 0xffu;
}

// ---------------------------------------------------------------------------
// PR_COMMA
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PR_COMMA (inclusive) .. PR_TAB_OR_SPC (exclusive)
// Name normalization: none (assembler label PR_COMMA kept verbatim).
// Note: The original uses 24 ($18) as the zone-change threshold; the source
// comments this as "<<< BUG: IT SHOULD BE 32 >>>".  The value is preserved
// faithfully here.
void PR_COMMA() {
    const std::uint8_t ch = variables_const().MON_CH; // lda MON_CH ($24)

    if (ch >= 24u) {
        // cmp #24 / bcs: at or past last zone — print CR, start new line
        // jsr CRDO / bne PR_NEXT_CHAR (always, since CRDO returns $f2 ≠ 0)
        CRDO();
    } else {
        // adc #16 / and #$f0 — advance to next comma zone boundary (16 or 32)
        // Carry is 0 (ch < 24 < 128, adc with C=0 never produces carry here).
        const std::uint8_t new_ch =
            static_cast<std::uint8_t>((static_cast<unsigned>(ch) + 16u) & 0xf0u);
        variables().MON_CH = new_ch;
    }
    // Both branches fall through to PR_NEXT_CHAR in the original.
    // PR_NEXT_CHAR (jsr CHRGET / jmp PRINT2) is handled by the PRINT2 loop caller.
}

// ---------------------------------------------------------------------------
// PR_TAB_OR_SPC
// ---------------------------------------------------------------------------

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PR_TAB_OR_SPC (inclusive) .. STROUT (exclusive)
// Name normalization: none (assembler label PR_TAB_OR_SPC kept verbatim).
// This range also contains PR_NEXT_CHAR (T:db2f) and DOSPC/NXSPC (T:db35).
// PR_NEXT_CHAR (jsr CHRGET / jmp PRINT2) is the loop-back mechanism within
// the PRINT2 caller; it is handled there rather than here.
void PR_TAB_OR_SPC(bool is_tab) {
    // php — save carry (is_tab encodes this)
    // jsr GTBYTC — advance, evaluate arg, result clamped to byte in X-reg
    const std::uint8_t n = GTBYTC();

    // cmp #(")"&$7f) = $29 — check for closing parenthesis
    const std::uint8_t next = CHRGOT();
    if (next != static_cast<std::uint8_t>(')' & 0x7fu)) {
        // jmp SYNERR
        SYNERR();
        return;
    }

    // plp — restore carry (is_tab)
    std::uint8_t spaces_to_print = 0u;

    if (is_tab) {
        // TAB( path: C=1
        // dex / txa / sbc MON_CH (with C=1 from plp): A = n-1-MON_CH
        // bcc PR_NEXT_CHAR — if MON_CH >= n, already past column
        const std::uint8_t ch = variables_const().MON_CH;
        if (ch >= n) {
            // bcc: fall through to PR_NEXT_CHAR (caller handles)
            return;
        }
        // tax / inx: X = n - MON_CH spaces needed
        spaces_to_print = static_cast<std::uint8_t>(n - ch);
    } else {
        // SPC( path: C=0 — inx makes X = n+1 for dex-first loop → prints n spaces
        spaces_to_print = n;
    }

    // NXSPC: dex / bne DOSPC — loop: print spaces_to_print spaces
    // DOSPC: jsr OUTSP / bne NXSPC (always, OUTSP returns $20 ≠ 0)
    for (std::uint8_t i = 0u; i < spaces_to_print; ++i) {
        OUTSP();
    }

    // Fall through to PR_NEXT_CHAR (caller handles jsr CHRGET / jmp PRINT2).
}

} // namespace applesoft::asm_port
