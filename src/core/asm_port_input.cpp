#include "core/asm_port_input.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_nxin.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_strtxt.hpp"
#include "platform/asm_port_outdo.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

void CONTROL_C_TYPED();
void HANDLERR();
void SYNERR();
void STROUT(std::string_view text);

// TODO(asm-port): port CHRGOT label.
std::uint8_t CHRGOT() { return 0; }

namespace {

constexpr std::uint16_t kINPUT_BUFFER_MINUS_1 = 0x01ffu;
constexpr std::uint16_t kINPUT_BUFFER = 0x0200u;
constexpr std::uint16_t kINPUT_BUFFER_PLUS_1 = 0x0201u;

// TODO(asm-port): port CHRGET label.
std::uint8_t CHRGET_INPUT() { return 0; }

// TODO(asm-port): port SYNCHR label.
void SYNCHR(std::uint8_t /*expected*/) {}

// TODO(asm-port): port ERRDIR label.
void ERRDIR() {}

// TODO(asm-port): port INLIN label.
void INLIN() {}

// TODO(asm-port): pop one byte from the emulated 6502 stack.
void popStackByte() {}

// TODO(asm-port): port PTRGET label.
std::uint16_t PTRGET() { return 0; }

// TODO(asm-port): monitor key input path used by GET mode in PROCESS_INPUT_LIST.
std::uint8_t MON_RDKEY() { return 0; }

// TODO(asm-port): parse quoted/unquoted string input path.
void parseStringInputAndStore() {}

// TODO(asm-port): parse numeric input and assign variable.
void parseNumericInputAndStore() {}

// TODO(asm-port): FINDATA scanner path (within PROCESS_INPUT_LIST..NEXT range).
void FINDATA() {}

// TODO(asm-port): comma checker used between input variables.
void CHKCOM() {}

// TODO(asm-port): DATPTR setter (SETDA label).
void SETDA(std::uint16_t data_ptr) {
    variables().DATPTR = data_ptr;
}

} // namespace

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUT (inclusive) .. NXIN (exclusive)
// Name normalization: none (assembler label INPUT kept verbatim).
void INPUT() {
    // Optional prompt string: INPUT "prompt";...
    if (CHRGOT() == static_cast<std::uint8_t>('"')) {
        STRTXT();
        SYNCHR(static_cast<std::uint8_t>(';' & 0x7fu));
        STRPRT();
    } else {
        OUTQUES();
    }

    // Illegal in direct mode.
    ERRDIR();

    // Prime input buffer and read a line.
    variables().writeByte(kINPUT_BUFFER_MINUS_1, static_cast<std::uint8_t>(',' & 0x7fu));
    INLIN();

    // CTRL-C abort path.
    if (variables_const().readByte(kINPUT_BUFFER) == 0x03u) {
        CONTROL_C_TYPED();
        return;
    }

    // Falls through to INPUT_FLAG_ZERO in ROM.
    INPUT_FLAG_ZERO(kINPUT_BUFFER_MINUS_1);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: READ (inclusive) .. INPUT_FLAG_ZERO (exclusive)
// Name normalization: none (assembler label READ kept verbatim).
void READ() {
    const std::uint16_t input_ptr = variables_const().DATPTR;

    // A=$98 then .byt $2c skips INPUT_FLAG_ZERO's lda #0 in ROM.
    // Model direct entry into PROCESS_INPUT_LIST with READ mode flag.
    PROCESS_INPUT_LIST(input_ptr, 0x98u);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUT_FLAG_ZERO (inclusive) .. PROCESS_INPUT_LIST (exclusive)
// Name normalization: none (assembler label INPUT_FLAG_ZERO kept verbatim).
void INPUT_FLAG_ZERO(std::uint16_t input_ptr) {
    PROCESS_INPUT_LIST(input_ptr, 0x00u);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PROCESS_INPUT_LIST (inclusive) .. NEXT (exclusive)
// Name normalization: none (assembler label PROCESS_INPUT_LIST kept verbatim).
void PROCESS_INPUT_LIST(std::uint16_t input_ptr, std::uint8_t input_flag) {
    // sta INPUTFLG / stx INPTR / sty INPTR+1
    variables().INPUTFLG = input_flag;
    variables().INPTR = input_ptr;

    while (true) {
        // PROCESS_INPUT_ITEM: jsr PTRGET / sta FORPNT / sty FORPNT+1
        variables().FORPNT = PTRGET();

        // Save current program TXTPTR and switch TXTPTR to input source.
        variables().TXPSV = variables_const().TXTPTR;
        variables().TXTPTR = variables_const().INPTR;

        // jsr CHRGOT
        std::uint8_t current = CHRGOT();
        if (current == 0u) {
            // Empty input source branch.
            if ((variables_const().INPUTFLG & 0x40u) != 0u) {
                // GET mode: fetch single key and treat as one-char input buffer.
                const std::uint8_t ch = static_cast<std::uint8_t>(MON_RDKEY() & 0x7fu);
                variables().writeByte(kINPUT_BUFFER, ch);
                variables().TXTPTR = kINPUT_BUFFER_MINUS_1;
            } else if ((variables_const().INPUTFLG & 0x80u) != 0u) {
                // READ mode: scan for next DATA item.
                FINDATA();
                variables().TXTPTR = variables_const().INPTR;
            } else {
                // INPUT mode: prompt and read another line.
                OUTQUES();
                NXIN();
                variables().TXTPTR = kINPUT_BUFFER_MINUS_1;
            }
        }

        // INSTART: advance to first token/char in source.
        current = CHRGET_INPUT();
        (void)current;

        // String vs numeric assignment paths.
        if ((variables_const().VALTYP & 0x80u) != 0u) {
            parseStringInputAndStore();
        } else {
            parseNumericInputAndStore();
        }

        // INPUT_MORE: expect EOL/colon or comma separator.
        const std::uint8_t sep = CHRGOT();
        if (sep != 0u && sep != static_cast<std::uint8_t>(',' & 0x7fu)) {
            INPUTERR();
            return;
        }

        // Save updated source pointer and restore program pointer.
        variables().INPTR = variables_const().TXTPTR;
        variables().TXTPTR = variables_const().TXPSV;

        // Next program token: end of statement, comma, or syntax issue.
        const std::uint8_t next_program_char = CHRGOT();
        if (next_program_char == 0u) {
            // INPDONE path.
            if ((variables_const().INPUTFLG & 0x80u) != 0u) {
                // READ: store advanced data pointer.
                SETDA(variables_const().INPTR);
            } else {
                // INPUT: if trailing chars remain, print "?EXTRA IGNORED".
                if (variables_const().readByte(variables_const().INPTR) != 0u) {
                    STROUT("?EXTRA IGNORED\r");
                }
            }
            return;
        }

        CHKCOM();
        // Continue to PROCESS_INPUT_ITEM for the next variable.
    }
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: ERLIN (inclusive) .. INPERR (exclusive)
// Name normalization: none (assembler label ERLIN kept verbatim).
void ERLIN(std::uint8_t a, std::uint8_t y) {
    variables().CURLIN =
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(y) << 8 | a);
    SYNERR();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: READERR (inclusive) .. ERLIN (exclusive)
// Name normalization: none (assembler label READERR kept verbatim).
void READERR() {
    ERLIN(
        static_cast<std::uint8_t>(variables_const().DATLIN & 0xffu),
        static_cast<std::uint8_t>(variables_const().DATLIN >> 8));
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: RESPERR (inclusive) .. GET (exclusive)
// Name normalization: none (assembler label RESPERR kept verbatim).
void RESPERR() {
    if ((variables_const().ERRFLG & 0x80u) != 0u) {
        // TODO(asm-port): propagate X=254 into HANDLERR once register model is shared.
        HANDLERR();
        return;
    }

    STROUT("?REENTER\r");
    variables().TXTPTR = variables_const().OLDTEXT;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPERR (inclusive) .. RESPERR (exclusive)
// Name normalization: none (assembler label INPERR kept verbatim).
void INPERR() {
    popStackByte();
    RESPERR();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUTERR (inclusive) .. READERR (exclusive)
// Name normalization: none (assembler label INPUTERR kept verbatim).
void INPUTERR() {
    const std::uint8_t input_flag = variables_const().INPUTFLG;
    if (input_flag == 0u) {
        RESPERR();
        return;
    }

    if ((input_flag & 0x80u) != 0u) {
        READERR();
        return;
    }

    ERLIN(0xffu, 0xffu);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GET (inclusive) .. INPUT (exclusive)
// Name normalization: none (assembler label GET kept verbatim).
void GET() {
    ERRDIR();

    // Simulate GET input source at INPUT_BUFFER+1 containing 0 terminator.
    variables().writeByte(kINPUT_BUFFER_PLUS_1, 0u);

    // Route directly to PROCESS_INPUT_LIST with GET mode ($40).
    PROCESS_INPUT_LIST(kINPUT_BUFFER_PLUS_1, 0x40u);
}

} // namespace applesoft::asm_port
