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
bool ISLETC();
void AYINT();
void PTRGET4();
std::uint8_t COMBYTE();
void GETADR();
void IQERR();
void SETDA(std::uint16_t dataPointer);

void SYNCHR(std::uint8_t expected);
void CHKCLS();
void CHKOPN();

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
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PARCHK (inclusive) .. CHKCLS (exclusive)
    // Name normalization: none (assembler label PARCHK kept verbatim).
    //
    // Validates and evaluates a parenthesized expression at TXTPTR:
    //   jsr CHKOPN  — require '(' at current position
    //   jsr FRMEVL  — evaluate the enclosed expression into FAC
    //   fall-through into CHKCLS — require ')' at current position

    CHKOPN();
    FRMEVL();
    CHKCLS();
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

void QINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: QINT (inclusive) .. INT (exclusive)
    // Name normalization: none (assembler label QINT kept verbatim).
    //
    // QINT converts the unpacked FAC mantissa into a signed 32-bit integer
    // in FAC+1..FAC+4 by arithmetic right-shifting according to the exponent.
    // The five-byte FAC is treated as one logical fixed-point value here rather
    // than re-porting the ROM's bytewise shifter helpers separately.

    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kFAC_SIGN = ApplesoftVariables::ZP_FAC_SIGN;

    const std::uint8_t exponent = ReadZeroPageByte(kFAC);
    if (exponent == 0u) {
        WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 1u), 0u);
        WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 2u), 0u);
        WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 3u), 0u);
        WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 4u), 0u);
        return;
    }

    const std::uint32_t mantissa =
        (static_cast<std::uint32_t>(ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 1u))) << 24u) |
        (static_cast<std::uint32_t>(ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 2u))) << 16u) |
        (static_cast<std::uint32_t>(ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 3u))) << 8u) |
        static_cast<std::uint32_t>(ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 4u)));

    const bool isNegative = (ReadZeroPageByte(kFAC_SIGN) & 0x80u) != 0u;
    std::int64_t signedMantissa = static_cast<std::int64_t>(mantissa);
    if (isNegative) {
        signedMantissa = -signedMantissa;
        WriteZeroPageByte(ApplesoftVariables::ZP_SHIFT_SIGN_EXT, 0xffu);
    }

    const std::uint8_t shiftCount = static_cast<std::uint8_t>(0xa0u - exponent);
    std::int32_t integerValue;
    if (shiftCount >= 32u) {
        integerValue = isNegative ? static_cast<std::int32_t>(-1) : static_cast<std::int32_t>(0);
    } else {
        integerValue = static_cast<std::int32_t>(signedMantissa >> shiftCount);
    }

    const std::uint32_t packedInteger = static_cast<std::uint32_t>(integerValue);
    WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 1u), static_cast<std::uint8_t>(packedInteger >> 24u));
    WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 2u), static_cast<std::uint8_t>(packedInteger >> 16u));
    WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 3u), static_cast<std::uint8_t>(packedInteger >> 8u));
    WriteZeroPageByte(static_cast<std::uint8_t>(kFAC + 4u), static_cast<std::uint8_t>(packedInteger & 0xffu));

    WriteZeroPageByte(ApplesoftVariables::ZP_SHIFT_SIGN_EXT, 0u);
}

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

bool CHKVAL(std::uint8_t savedValTyp) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKVAL (inclusive) .. FRMEVL (exclusive)
    // Name normalization: none (assembler label CHKVAL kept verbatim).

    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;

    const bool facIsString = (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u;
    const bool expectedString = (savedValTyp & 0x80u) != 0u;

    if (facIsString != expectedString) {
        ERROR(ERR_BADTYPE);
        return false;
    }

    return facIsString;
}

void NEG32768() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEG32768 (inclusive) .. MAKINT (exclusive)
    // Name normalization: none (assembler label NEG32768 kept verbatim).

    constexpr std::uint8_t kResult = ApplesoftVariables::ZP_RESULT;

    WriteZeroPageByte(kResult, kNEG32768Data[0]);
    WriteZeroPageByte(static_cast<std::uint8_t>(kResult + 1u), kNEG32768Data[1]);
    WriteZeroPageByte(static_cast<std::uint8_t>(kResult + 2u), kNEG32768Data[2]);
    WriteZeroPageByte(static_cast<std::uint8_t>(kResult + 3u), kNEG32768Data[3]);
}

void MAKINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKINT (inclusive) .. MKINT (exclusive)
    // Name normalization: none (assembler label MAKINT kept verbatim).

    CHRGET();
    FRMNUM();
    MKINT();
}

void MI1() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MI1 (inclusive) .. MI2 (exclusive)
    // Name normalization: none (assembler label MI1 kept verbatim).

    IQERR();
}

void MI2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MI2 (inclusive) .. ARRAY (exclusive)
    // Name normalization: none (assembler label MI2 kept verbatim).

    QINT();
}

bool ISLETC() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ISLETC (inclusive) .. NAME_NOT_FOUND (exclusive)
    // Name normalization: none (assembler label ISLETC kept verbatim).

    const std::uint8_t ch = CHRGOT();
    return ch >= static_cast<std::uint8_t>('A') && ch <= static_cast<std::uint8_t>('Z');
}

void MKINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MKINT (inclusive) .. AYINT (exclusive)
    // Name normalization: none (assembler label MKINT kept verbatim).

    if ((ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN) & 0x80u) != 0u) {
        MI1();
        return;
    }

    AYINT();
}

void ROUND_FAC() {
    // TODO(asm-port): port ROUND_FAC label.
}

void NAMOK() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NAMOK (inclusive) .. BASIC (exclusive)
    // Name normalization: none (assembler label NAMOK kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP, 0u); // VALTYP
    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP_PLUS_1, 0u); // VALTYP+1
    PTRGET4();
}

}  // namespace applesoft::asm_port