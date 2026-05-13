#include "core/asm_port_error.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_error_messages.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

bool IsStatementEndOfParsedInput();
std::uint8_t ReadZeroPageByte(std::uint8_t address);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);
std::uint8_t ReadStackPointer();
void SetStackPointer(std::uint8_t value);
void PushWordToStack(std::uint16_t value);
void PushByteToStack(std::uint8_t value);
std::uint8_t PopByteFromStack();
void PopReturnAddress();
void PRINT_ERROR_LINNUM(std::string_view prefix);
std::uint8_t CHRGOT();
std::uint8_t CHRGET();
void LINGET();
void SYNERR();
void EXECUTE_STATEMENT();
void EXECUTE_STATEMENT_1();
void NEWSTT();
void RESTART();
void RTS_5();
void GOSUB();
void GO_TO_LINE();
void GOTO();
void POP();
void RETURN();
void PULL3();
std::uint8_t REMN();
bool FL1(std::uint16_t startAddress);

void STOP_impl(bool shouldPrintBreak);
void ENDX_impl(bool shouldPrintBreak);

namespace {

struct ProgramPointer {
    std::uint16_t address = 0;

    std::uint8_t read(std::uint16_t offset = 0) const {
        return ReadProgramByte(static_cast<std::uint16_t>(address + offset));
    }

    void write(std::uint8_t value, std::uint16_t offset = 0) const {
        WriteProgramByte(static_cast<std::uint16_t>(address + offset), value);
    }

    ProgramPointer advanced(std::uint16_t offset) const {
        return ProgramPointer{static_cast<std::uint16_t>(address + offset)};
    }
};

std::uint8_t readStackByteAt(std::uint8_t x, std::uint8_t plus) {
    const std::uint8_t offset = static_cast<std::uint8_t>(x + plus);
    return ReadProgramByte(static_cast<std::uint16_t>(0x0100u + offset));
}

}  // namespace

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);

bool gReturnFromPopContext = false;

bool ReturnWasFromPOPContext() {
    if (!gReturnFromPopContext) {
        return false;
    }

    gReturnFromPopContext = false;
    return true;
}

std::uint8_t PeekTopControlTokenAfterGTFORPNT() {
    return readStackByteAt(ReadStackPointer(), 1u);
}

void STOP() {
    STOP_impl(false);
}

void STOP_impl(bool shouldPrintBreak) {
    if (!IsStatementEndOfParsedInput()) {
        return;
    }

    ENDX_impl(shouldPrintBreak);
}

void ENDX() {
    ENDX_impl(false);
}

void ENDX_impl(bool shouldPrintBreak) {
    if (!IsStatementEndOfParsedInput()) {
        return;
    }

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;
    constexpr std::uint8_t kOLDLIN = ApplesoftVariables::ZP_OLDLIN;

    const std::uint16_t textPointer = ReadZeroPageWord(kTXTPTR);
    const std::uint16_t currentLine = ReadZeroPageWord(kCURLIN);
    const std::uint8_t currentPageHi = ApplesoftVariables::highByte(currentLine);

    if (static_cast<std::uint8_t>(currentPageHi + 1u) != 0u) {
        WriteZeroPageWord(kOLDTEXT, textPointer);
        WriteZeroPageWord(kOLDLIN, currentLine);
    }

    PopReturnAddress();
    PopReturnAddress();

    if (shouldPrintBreak) {
        PRINT_ERROR_LINNUM(QT_ERROR(QT_BREAK_INDEX));
        return;
    }

    RESTART();
}

void CONT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CONT (inclusive) .. SAVE (exclusive)
    // Name normalization: none (assembler label CONT kept verbatim).
    // Internal label mapping: "bne RTS_4" is modeled as an early return.

    if (!IsStatementEndOfParsedInput()) {
        return;
    }

    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;
    constexpr std::uint8_t kOLDTEXT_plus_1 = static_cast<std::uint8_t>(ApplesoftVariables::ZP_OLDTEXT + 1u);
    constexpr std::uint8_t kOLDLIN = ApplesoftVariables::ZP_OLDLIN;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;

    if (ReadZeroPageByte(kOLDTEXT_plus_1) == 0) {
        ERROR(ERR_CANTCONT);
        return;
    }

    WriteZeroPageWord(kTXTPTR, ReadZeroPageWord(kOLDTEXT));
    WriteZeroPageWord(kCURLIN, ReadZeroPageWord(kOLDLIN));
}

void GOSUB() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GOSUB (inclusive) .. GO_TO_LINE (exclusive)
    // Name normalization: none (assembler label GOSUB kept verbatim).
    //
    // Executes the "GOSUB" command:
    // - Checks stack space for the return frame (7 bytes)
    // - Pushes return frame containing: TXTPTR (2), CURLIN (2), TOKEN_GOSUB (1)
    // - Falls through to shared GO_TO_LINE logic to find and execute the target line
    // - On RETURN, restores execution state from the stack frame

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kTOKEN_GOSUB = 0xb0;

    CHKMEMState chkmemState{};
    chkmemState.a = 3;
    chkmemState.stackPointer = ReadStackPointer();
    const auto chkmemResult = CHKMEM(chkmemState);
    if (!chkmemResult.ok) {
        return;
    }

    const std::uint16_t textPointer = ReadZeroPageWord(kTXTPTR);
    const std::uint16_t currentLine = ReadZeroPageWord(kCURLIN);

    PushWordToStack(textPointer);
    PushWordToStack(currentLine);
    PushByteToStack(kTOKEN_GOSUB);

    GO_TO_LINE();
}

void GO_TO_LINE() {
    (void)CHRGOT();
    GOTO();
    NEWSTT();
}

void GOTO() {
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;

    LINGET();
    const std::uint8_t remnOffset = REMN();

    const std::uint8_t currentPage = ReadZeroPageByte(static_cast<std::uint8_t>(kCURLIN + 1u));
    const std::uint8_t targetPage = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1u));

    ProgramPointer start{};
    if (currentPage >= targetPage) {
        start = ProgramPointer{ReadZeroPageWord(kTXTTAB)};
    } else {
        const ProgramPointer textPtr{ReadZeroPageWord(kTXTPTR)};
        start = textPtr.advanced(static_cast<std::uint16_t>(remnOffset) + 1u);
    }

    if (!FL1(start.address)) {
        ERROR(ERR_UNDEFSTAT);
        return;
    }

    const std::uint16_t destination = static_cast<std::uint16_t>(ReadZeroPageWord(kLOWTR) - 1u);
    WriteZeroPageWord(kTXTPTR, destination);
}

void POP() {
    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    constexpr std::uint8_t kTOKEN_GOSUB = 0xb0;

    if (!IsStatementEndOfParsedInput()) {
        RTS_5();
        return;
    }

    WriteZeroPageByte(kFORPNT, 0xffu);

    GTFORPNTState gtforpntState{};
    const auto gtforpntResult = GTFORPNT(ReadStackPointer(), gtforpntState);
    SetStackPointer(gtforpntResult.x);

    if (PeekTopControlTokenAfterGTFORPNT() == kTOKEN_GOSUB) {
        gReturnFromPopContext = true;
        RETURN();
        return;
    }

    ERROR(ERR_NOGOSUB);
}

void RETURN() {
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    (void)PopByteFromStack();
    const std::uint8_t currentLineLo = PopByteFromStack();

    if (ReturnWasFromPOPContext()) {
        PULL3();
        return;
    }

    const std::uint8_t currentLineHi = PopByteFromStack();
    const std::uint8_t textPointerLo = PopByteFromStack();
    const std::uint8_t textPointerHi = PopByteFromStack();

    WriteZeroPageWord(kCURLIN, ApplesoftVariables::makeWord(currentLineLo, currentLineHi));
    WriteZeroPageWord(kTXTPTR, ApplesoftVariables::makeWord(textPointerLo, textPointerHi));
}

}  // namespace applesoft::asm_port