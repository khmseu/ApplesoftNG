#include "core/asm_port_input.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_strtxt.hpp"

#include <string_view>

namespace applesoft::asm_port {

void CONTROL_C_TYPED();
void HANDLERR();
void SYNERR();
void STROUT(std::string_view text);

// TODO(asm-port): port CHRGOT label.
std::uint8_t CHRGOT() { return 0; }

namespace {

// --- Dummy callees for incremental porting ---

// TODO(asm-port): port SYNCHR label.
void SYNCHR(std::uint8_t /*expected*/) {}

// TODO(asm-port): port STRPRT label.
void STRPRT() {}

// TODO(asm-port): port OUTQUES label.
void OUTQUES() {}

// TODO(asm-port): port ERRDIR label.
void ERRDIR() {}

// TODO(asm-port): port INLIN label.
void INLIN() {}

// TODO(asm-port): pop one byte from the emulated 6502 stack.
void popStackByte() {}

// TODO(asm-port): port PROCESS_INPUT_LIST label.
void PROCESS_INPUT_LIST() {}

void write_INPUT_BUFFER_minus_1(std::uint8_t v) {
    variables().writeByte(0x01ffu, v);
}

std::uint8_t read_INPUT_BUFFER_0() {
    return variables_const().readByte(0x0200u);
}

} // namespace

InputDispatch INPUT() {
    // "INPUT" statement
    // Check for optional prompt string.
    if (CHRGOT() == static_cast<std::uint8_t>('"')) {
        // INPUT "prompt"; ...
        STRTXT();
        SYNCHR(static_cast<std::uint8_t>(';'));
        STRPRT();
    } else {
        // No string prompt => print "?"
        OUTQUES();
    }

    // Illegal in direct mode.
    ERRDIR();

    // Prime input buffer with comma at INPUT_BUFFER-1.
    write_INPUT_BUFFER_minus_1(static_cast<std::uint8_t>(','));

    // Read a line into input buffer.
    INLIN();

    // If first char is CTRL-C ($03), dispatch control-break path.
    if (read_INPUT_BUFFER_0() == static_cast<std::uint8_t>(0x03)) {
        CONTROL_C_TYPED();
        return InputDispatch::ControlCTyped;
    }

    // Fall-through target in original is INPUT_FLAG_ZERO (outside this slice).
    return InputDispatch::ContinueAt_INPUT_FLAG_ZERO;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: ERLIN (inclusive) .. INPERR (exclusive)
// Name normalization: none (assembler label ERLIN kept verbatim).
void ERLIN(std::uint8_t a, std::uint8_t y) {
    // sta CURLIN / sty CURLIN+1
    variables().CURLIN = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(y) << 8 | a);

    // jmp SYNERR
    SYNERR();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: READERR (inclusive) .. ERLIN (exclusive)
// Name normalization: none (assembler label READERR kept verbatim).
void READERR() {
    // lda DATLIN / ldy DATLIN+1 then fall through to ERLIN.
    ERLIN(
        static_cast<std::uint8_t>(variables_const().DATLIN & 0xffu),
        static_cast<std::uint8_t>(variables_const().DATLIN >> 8));
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: RESPERR (inclusive) .. GET (exclusive)
// Name normalization: none (assembler label RESPERR kept verbatim).
void RESPERR() {
    // bit ERRFLG / bpl L_RESPERR_1
    if ((variables_const().ERRFLG & 0x80u) != 0u) {
        // ldx #254 / jmp HANDLERR
        // TODO(asm-port): propagate X=254 into HANDLERR once register model is shared.
        HANDLERR();
        return;
    }

    // lda #<ERR_REENTRY / ldy #>ERR_REENTRY / jsr STROUT
    STROUT("?REENTER\r");

    // lda OLDTEXT / ldy OLDTEXT+1 / sta TXTPTR / sty TXTPTR+1 / rts
    variables().TXTPTR = variables_const().OLDTEXT;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPERR (inclusive) .. RESPERR (exclusive)
// Name normalization: none (assembler label INPERR kept verbatim).
void INPERR() {
    // pla, then fall through into RESPERR.
    popStackByte();
    RESPERR();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUTERR (inclusive) .. READERR (exclusive)
// Name normalization: none (assembler label INPUTERR kept verbatim).
void INPUTERR() {
    // lda INPUTFLG / beq RESPERR / bmi READERR / ldy #$ff / bne ERLIN(always)
    const std::uint8_t inputFlag = variables_const().INPUTFLG;
    if (inputFlag == 0u) {
        RESPERR();
        return;
    }

    if ((inputFlag & 0x80u) != 0u) {
        READERR();
        return;
    }

    // GET path: line number = $ffff then jump to ERLIN.
    ERLIN(0xffu, 0xffu);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GET (inclusive) .. INPUT (exclusive)
// Name normalization: none (assembler label GET kept verbatim).
void GET() {
    // jsr ERRDIR — illegal in direct mode
    ERRDIR();

    // ldx #<(INPUT_BUFFER+1) / ldy #>(INPUT_BUFFER+1)
    // a9 #0 / sta INPUT_BUFFER+1
    variables().writeByte(0x0201u, 0u);

    // a9 #$40 / jsr PROCESS_INPUT_LIST
    variables().INPUTFLG = 0x40u;
    PROCESS_INPUT_LIST();

    // rts
}

} // namespace applesoft::asm_port