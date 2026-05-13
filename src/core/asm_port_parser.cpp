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
void SYNERR();
void FRMNUM();
void CONINT();
void GETADR();
std::uint8_t COMBYTE();

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

}  // namespace applesoft::asm_port