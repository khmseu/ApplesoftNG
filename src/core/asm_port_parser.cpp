#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_error_messages.hpp"

#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t ReadZeroPageByte(std::uint8_t address);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
std::uint8_t CHRGOT();
std::uint8_t CHRGET();
std::uint8_t ReadProgramByte(std::uint16_t address);
void SYNERR();
void FRMEVL();
void FRMNUM();
void CONINT();
void MKINT();
std::uint8_t COMBYTE();
void GETADR();
void IQERR();
void SETDA(std::uint16_t dataPointer);

void SYNCHR(std::uint8_t expected) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SYNCHR (inclusive) .. CHKNUM (exclusive)
    // Name normalization: none (assembler label SYNCHR kept verbatim).
    //
    // Require a specific statement token from the parsed input.
    // Read current character from TXTPTR, compare with expected, advance if match, error if not.

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    const std::uint16_t txtPtr = ReadZeroPageWord(kTXTPTR);
    const std::uint8_t current = variables_const().pointer(txtPtr).read(0u);

    if (current != expected) {
        ERROR(ERR_SYNTAX);
    }

    // Advance TXTPTR by 1.
    WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(txtPtr + 1u));
}

void CHKNUM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKNUM (inclusive) .. CHKSTR (exclusive)
    // Name normalization: none (assembler label CHKNUM kept verbatim).

    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;
    const bool facIsString = (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u;
    if (facIsString) {
        ERROR(ERR_BADTYPE);
    }
}

void CHKSTR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKSTR (inclusive) .. CHKVAL (exclusive)
    // Name normalization: none (assembler label CHKSTR kept verbatim).

    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;
    const bool facIsString = (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u;
    if (!facIsString) {
        ERROR(ERR_BADTYPE);
    }
}

void CHKCOM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKCOM (inclusive) .. SYNCHR (exclusive)
    // Name normalization: none (assembler label CHKCOM kept verbatim).

    SYNCHR(static_cast<std::uint8_t>(','));
}

void FRMNUM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FRMNUM (inclusive) .. FRMEVL (exclusive)
    // Name normalization: none (assembler label FRMNUM kept verbatim).
    //
    // FRMNUM does JSR FRMEVL and falls through into CHKNUM in ROM.

    FRMEVL();
    CHKNUM();
}

void PARCHK() {
    // TODO(asm-port): parse "(expression)" - validate open paren, evaluate, validate close.
}

void STORE_FACDB_YX_ROUNDED() {
    // TODO(asm-port): store 5-byte FAC to address in Y,X with rounding.
}

void CHKCLS() {
    // Check for ')' at current position.
    SYNCHR(static_cast<std::uint8_t>(')'));
}

void CHKOPN() {
    // Check for '(' at current position.
    SYNCHR(static_cast<std::uint8_t>('('));
}

void LINGET() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LINGET (inclusive) .. LET (exclusive)
    // Name normalization: none (assembler label LINGET kept verbatim).

    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kINDEX = ApplesoftVariables::ZP_INDEX;
    constexpr std::uint8_t kCHARAC = ApplesoftVariables::ZP_CHARAC;

    auto is_digit = [](std::uint8_t ch) {
        return ch >= static_cast<std::uint8_t>('0') && ch <= static_cast<std::uint8_t>('9');
    };

    WriteZeroPageWord(kLINNUM, 0);

    std::uint8_t current = CHRGOT();
    while (is_digit(current)) {
        const std::uint8_t digit = static_cast<std::uint8_t>(current - static_cast<std::uint8_t>('0'));
        WriteZeroPageByte(kCHARAC, digit);

        const std::uint8_t lineHigh = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1u));
        WriteZeroPageByte(kINDEX, lineHigh);

        // Preserve ROM overflow guard (line number exceeds 63999).
        if (lineHigh >= 0x19u) {
            SYNERR();
            return;
        }

        const std::uint16_t value = ReadZeroPageWord(kLINNUM);
        WriteZeroPageWord(kLINNUM, static_cast<std::uint16_t>(value * 10u + digit));

        current = CHRGET();
    }
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GTBYTC (inclusive) .. GETBYT (exclusive)
// Name normalization: none (assembler label GTBYTC kept verbatim).
//
// Advance text pointer one character, then evaluate as byte (0-255) in FAC+4.
//   jsr CHRGET     -- advance TXTPTR
//   fall-through to GETBYT
void GTBYTC() {
    CHRGET();
    (void)GETBYT();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GETBYT (inclusive) .. CONINT (exclusive)
// Name normalization: none (assembler label GETBYT kept verbatim).
//
// Evaluate numeric expression at TXTPTR into FAC, convert to byte (0-255) in FAC+4.
//   jsr FRMNUM     -- evaluate numeric expression
//   fall-through to CONINT
std::uint8_t GETBYT() {
    FRMNUM();
    CONINT();
    return ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 4u));
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CONINT (inclusive) .. VAL (exclusive)
// Name normalization: none (assembler label CONINT kept verbatim).
//
// Convert FAC to a single-byte integer (0-255) in FAC+4.
//   jsr MKINT      -- truncate FAC to integer
//   ldx FAC+3      -- high byte must be zero (else >255)
//   bne GOIQ       -- illegal quantity
//   ldx FAC+4      -- result byte in X (FAC+4 = low byte)
//   jmp CHRGOT     -- refresh A with current input char and return
void CONINT() {
    MKINT();
    const std::uint8_t facHi = ReadZeroPageByte(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 3u));
    if (facHi != 0u) {
        IQERR();  // GOIQ: jmp IQERR -- value > 255
        return;
    }
    // FAC+4 already holds the result byte after MKINT.
    (void)CHRGOT();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GTNUM (inclusive) .. COMBYTE (exclusive)
// Name normalization: none (assembler label GTNUM kept verbatim).
std::uint8_t GTNUM() {
    FRMNUM();
    GETADR();
    return COMBYTE();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: COMBYTE (inclusive) .. GETADR (exclusive)
// Name normalization: none (assembler label COMBYTE kept verbatim).
std::uint8_t COMBYTE() {
    CHKCOM();
    return GETBYT();
}

// TODO(asm-port): port QINT label.
void QINT() {}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GENERIC_CHRGET (inclusive) .. GENERIC_END (exclusive)
// Name normalization: none (assembler label GENERIC_CHRGET kept verbatim).
std::uint8_t GENERIC_CHRGET() {
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    // Generic CHRGET increments TXTPTR first, then examines the current character.
    const std::uint16_t next = static_cast<std::uint16_t>(ReadZeroPageWord(kTXTPTR) + 1u);
    WriteZeroPageWord(kTXTPTR, next);

    std::uint8_t current = variables_const().pointer(next).read();
    if (current >= static_cast<std::uint8_t>(':')) {
        return current;
    }

    if (current == static_cast<std::uint8_t>(' ')) {
        return GENERIC_CHRGET();
    }

    // Preserve the ROM arithmetic side effect used by numeric parsing.
    current = static_cast<std::uint8_t>(current - static_cast<std::uint8_t>('0'));
    current = static_cast<std::uint8_t>(current - 0xd0u);
    return current;
}

std::uint8_t CHRGET() {
    return GENERIC_CHRGET();
}

std::uint8_t CHRGOT() {
    return variables_const().pointer(ReadZeroPageWord(ApplesoftVariables::ZP_TXTPTR)).read();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GETADR (inclusive) .. PEEK (exclusive)
// Name normalization: none (assembler label GETADR kept verbatim).
void GETADR() {
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;

    if (ReadZeroPageByte(kFAC) >= 0x91u) {
        IQERR();
        return;
    }

    QINT();

    const std::uint8_t lo = ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 4u));
    const std::uint8_t hi = ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 3u));
    WriteZeroPageByte(kLINNUM, lo);
    WriteZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1u), hi);
}

std::uint8_t CurrentStatementChar() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: EXECUTE_STATEMENT (inclusive) .. EXECUTE_STATEMENT_1 (exclusive)
    // Name normalization: helper name chosen for the inline EXECUTE_STATEMENT load.
    // EXECUTE_STATEMENT uses `ldy #0` then `lda (TXTPTR),Y`.
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    return ReadProgramByte(ReadZeroPageWord(kTXTPTR));
}

bool IsStatementEndOfParsedInput() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: END2 (inclusive) .. SAVE (exclusive)
    // Name normalization: helper name chosen for the inline `bne RTS_4` guard.
    // STOP/END/CONT continue only when parser is at end-of-statement; model the
    // zero-flag check via the current parsed character at TXTPTR.
    return CHRGOT() == 0u;
}

void RESTORE() {
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    const std::uint16_t textTable = ReadZeroPageWord(kTXTTAB);
    const std::uint16_t dataPointer = static_cast<std::uint16_t>(textTable - 1u);
    SETDA(dataPointer);
}

void SETDA(std::uint16_t dataPointer) {
    constexpr std::uint8_t kDATPTR = ApplesoftVariables::ZP_DATPTR;
    WriteZeroPageWord(kDATPTR, dataPointer);
}

void SYNERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SYNERR (inclusive) .. ERROR (exclusive)
    // Name normalization: none (assembler label SYNERR kept verbatim).
    // ROM sequence is `ldx #ERR_SYNTAX` followed by an unconditional transfer
    // into ERROR. In C++, dispatch directly with the syntax error code.
    ERROR(ERR_SYNTAX);
}

}  // namespace applesoft::asm_port