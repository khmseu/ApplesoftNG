#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_error_messages.hpp"

#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t ReadZeroPageByte(std::uint8_t address);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);

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

}  // namespace applesoft::asm_port