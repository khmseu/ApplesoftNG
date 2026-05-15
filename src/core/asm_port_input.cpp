#include "core/asm_port_input.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_nxin.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_strtxt.hpp"
#include "platform/asm_port_outdo.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_input.hpp"
#include "core/asm_port_inlin.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/asm_port_clear.hpp"
#include "core/asm_port_stack.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

void AS_CONTROL_C_TYPED();
void AS_HANDLERR();
void AS_SYNERR();
void AS_ERRDIR();
void AS_SYNCHR(std::uint8_t expected);
void SetPendingErrorCode(std::uint8_t errorCode);

namespace {

constexpr std::uint16_t kAS_INPUT_BUFFER_MINUS_1 = ApplesoftVariables::ADDR_AS_INPUT_BUFFER_MINUS_1;
constexpr std::uint16_t kAS_INPUT_BUFFER = ApplesoftVariables::ADDR_AS_INPUT_BUFFER;
constexpr std::uint16_t kAS_INPUT_BUFFER_PLUS_1 = ApplesoftVariables::ADDR_AS_INPUT_BUFFER + 1u;

std::uint8_t AS_CHRGET_INPUT() { return AS_CHRGET(); }

// TODO(asm-port): port AS_INLIN label.
// void AS_INLIN() {}

// TODO(asm-port): port AS_PTRGET label.
// std::uint16_t AS_PTRGET() { return 0; }

// TODO(asm-port): monitor key input path used by AS_GET mode in AS_PROCESS_INPUT_LIST.
// std::uint8_t MON_RDKEY() { return 0; }

// TODO(asm-port): parse quoted/unquoted string input path.
void parseStringInputAndStore() {}

// TODO(asm-port): parse numeric input and assign variable.
void parseNumericInputAndStore() {}

// TODO(asm-port): AS_FINDATA scanner path (within AS_PROCESS_INPUT_LIST..AS_NEXT range).
void AS_FINDATA() {}

// TODO(asm-port): comma checker used between input variables.
// void AS_CHKCOM() {}

} // namespace

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPUT (inclusive) .. AS_NXIN (exclusive)
// Name normalization: none (assembler label AS_INPUT kept verbatim).
void AS_INPUT() {
    // Optional prompt string: AS_INPUT "prompt";...
    if (AS_CHRGOT() == static_cast<std::uint8_t>('"')) {
        AS_STRTXT();
        AS_SYNCHR(static_cast<std::uint8_t>(';' & 0x7fu));
        AS_STRPRT();
    } else {
        AS_OUTQUES();
    }

    // Illegal in direct mode.
    AS_ERRDIR();

    // Prime input buffer and read a line.
    variables().writeByte(kAS_INPUT_BUFFER_MINUS_1, static_cast<std::uint8_t>(',' & 0x7fu));
    AS_INLIN();

    // CTRL-C abort path.
    if (variables_const().readByte(kAS_INPUT_BUFFER) == 0x03u) {
        AS_CONTROL_C_TYPED();
        return;
    }

    // Falls through to AS_INPUT_FLAG_ZERO in ROM.
    AS_INPUT_FLAG_ZERO(kAS_INPUT_BUFFER_MINUS_1);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_READ (inclusive) .. AS_INPUT_FLAG_ZERO (exclusive)
// Name normalization: none (assembler label AS_READ kept verbatim).
void AS_READ() {
    const std::uint16_t input_ptr = variables_const().AS_DATPTR;

    // A=$98 then .byt $2c skips AS_INPUT_FLAG_ZERO's lda #0 in ROM.
    // Model direct entry into AS_PROCESS_INPUT_LIST with AS_READ mode flag.
    AS_PROCESS_INPUT_LIST(input_ptr, 0x98u);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPUT_FLAG_ZERO (inclusive) .. AS_PROCESS_INPUT_LIST (exclusive)
// Name normalization: none (assembler label AS_INPUT_FLAG_ZERO kept verbatim).
void AS_INPUT_FLAG_ZERO(std::uint16_t input_ptr) {
    AS_PROCESS_INPUT_LIST(input_ptr, 0x00u);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_PROCESS_INPUT_LIST (inclusive) .. AS_NEXT (exclusive)
// Name normalization: none (assembler label AS_PROCESS_INPUT_LIST kept verbatim).
void AS_PROCESS_INPUT_LIST(std::uint16_t input_ptr, std::uint8_t input_flag) {
    // sta AS_INPUTFLG / stx AS_INPTR / sty AS_INPTR+1
    variables().AS_INPUTFLG = input_flag;
    variables().AS_INPTR = input_ptr;

    while (true) {
        // AS_PROCESS_INPUT_ITEM: jsr AS_PTRGET / sta AS_FORPNT / sty AS_FORPNT+1
        variables().AS_FORPNT = AS_PTRGET();

        // Save current program AS_TXTPTR and switch AS_TXTPTR to input source.
        variables().AS_TXPSV = variables_const().AS_TXTPTR;
        variables().AS_TXTPTR = variables_const().AS_INPTR;

        // jsr AS_CHRGOT
        std::uint8_t current = AS_CHRGOT();
        if (current == 0u) {
            // Empty input source branch.
            if ((variables_const().AS_INPUTFLG & 0x40u) != 0u) {
                // AS_GET mode: fetch single key and treat as one-char input buffer.
                const std::uint8_t ch = static_cast<std::uint8_t>(MON_RDKEY() & 0x7fu);
                variables().writeByte(kAS_INPUT_BUFFER, ch);
                variables().AS_TXTPTR = kAS_INPUT_BUFFER_MINUS_1;
            } else if ((variables_const().AS_INPUTFLG & 0x80u) != 0u) {
                // AS_READ mode: scan for next AS_DATA item.
                AS_FINDATA();
                variables().AS_TXTPTR = variables_const().AS_INPTR;
            } else {
                // AS_INPUT mode: prompt and read another line.
                AS_OUTQUES();
                AS_NXIN();
                variables().AS_TXTPTR = kAS_INPUT_BUFFER_MINUS_1;
            }
        }

        // AS_INSTART: advance to first token/char in source.
        current = AS_CHRGET_INPUT();
        (void)current;

        // String vs numeric assignment paths.
        if ((variables_const().AS_VALTYP & 0x80u) != 0u) {
            parseStringInputAndStore();
        } else {
            parseNumericInputAndStore();
        }

        // AS_INPUT_MORE: expect EOL/colon or comma separator.
        const std::uint8_t sep = AS_CHRGOT();
        if (sep != 0u && sep != static_cast<std::uint8_t>(',' & 0x7fu)) {
            AS_INPUTERR();
            return;
        }

        // Save updated source pointer and restore program pointer.
        variables().AS_INPTR = variables_const().AS_TXTPTR;
        variables().AS_TXTPTR = variables_const().AS_TXPSV;

        // Next program token: end of statement, comma, or syntax issue.
        const std::uint8_t next_program_char = AS_CHRGOT();
        if (next_program_char == 0u) {
            // AS_INPDONE path.
            if ((variables_const().AS_INPUTFLG & 0x80u) != 0u) {
                // AS_READ: store advanced data pointer.
                AS_SETDA(variables_const().AS_INPTR);
            } else {
                // AS_INPUT: if trailing chars remain, print "?EXTRA IGNORED".
                if (variables_const().pointer(variables_const().AS_INPTR).read() != 0u) {
                    AS_STROUT("?EXTRA IGNORED\r");
                }
            }
            return;
        }

        AS_CHKCOM();
        // Continue to AS_PROCESS_INPUT_ITEM for the next variable.
    }
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_ERLIN (inclusive) .. AS_INPERR (exclusive)
// Name normalization: none (assembler label AS_ERLIN kept verbatim).
void AS_ERLIN(std::uint16_t lineNumber) {
    variables().AS_CURLIN = lineNumber;
    AS_SYNERR();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_READERR (inclusive) .. AS_ERLIN (exclusive)
// Name normalization: none (assembler label AS_READERR kept verbatim).
void AS_READERR() {
    AS_ERLIN(variables_const().AS_DATLIN);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_RESPERR (inclusive) .. AS_GET (exclusive)
// Name normalization: none (assembler label AS_RESPERR kept verbatim).
void AS_RESPERR() {
    if ((variables_const().AS_ERRFLG & 0x80u) != 0u) {
        // ROM AS_RESPERR sets X=254 before jumping to AS_HANDLERR.
        SetPendingErrorCode(0xfeu);
        AS_HANDLERR();
        return;
    }

    AS_STROUT("?REENTER\r");
    variables().AS_TXTPTR = variables_const().AS_OLDTEXT;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPERR (inclusive) .. AS_RESPERR (exclusive)
// Name normalization: none (assembler label AS_INPERR kept verbatim).
void AS_INPERR() {
    (void)theStack().popByte();
    AS_RESPERR();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPUTERR (inclusive) .. AS_READERR (exclusive)
// Name normalization: none (assembler label AS_INPUTERR kept verbatim).
void AS_INPUTERR() {
    const std::uint8_t input_flag = variables_const().AS_INPUTFLG;
    if (input_flag == 0u) {
        AS_RESPERR();
        return;
    }

    if ((input_flag & 0x80u) != 0u) {
        AS_READERR();
        return;
    }

    AS_ERLIN(0xffffu);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_GET (inclusive) .. AS_INPUT (exclusive)
// Name normalization: none (assembler label AS_GET kept verbatim).
void AS_GET() {
    AS_ERRDIR();

    // Simulate AS_GET input source at AS_INPUT_BUFFER+1 containing 0 terminator.
    variables().writeByte(kAS_INPUT_BUFFER_PLUS_1, 0u);

    // Route directly to AS_PROCESS_INPUT_LIST with AS_GET mode ($40).
    AS_PROCESS_INPUT_LIST(kAS_INPUT_BUFFER_PLUS_1, 0x40u);
}

} // namespace applesoft::asm_port
