#include "core/asm_port_error.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_token_address_table.hpp"

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
void SYNCHR(std::uint8_t expected);
void LET();
void IF();
void REM();
void IF_TRUE();
void ONGOTO();
void CONTROL_C_TYPED();
void STEP();
void TRACE_();
void LOAD_FAC_FROM_YA();
void FRMNUM();
void FRMEVL();
void SIGN();
void FRM_STACK_2();
void PushForPntFrame();
std::uint8_t GETBYT();
void ADDON(std::uint8_t offset);
bool ISCNTC();
void OUTSP();
void LINPRT();
void OUTDO(std::uint8_t value);
std::uint8_t CurrentStatementChar();
bool IsRunningMode();
bool IsTraceEnabled();
bool IsEndOfLineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadLineNumberFromTextPointer();
void AdvanceTextPointerToNextLine();
void GOEND();
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
void HANDLERR();
void SetPendingErrorCode(std::uint8_t errorCode);

constexpr std::uint8_t kTokenBase = 0x80u;

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

void CONTROL_C_TYPED() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CONTROL_C_TYPED (inclusive) .. STOP (exclusive)
    // Name normalization: none (assembler label CONTROL_C_TYPED kept verbatim).
    constexpr std::uint8_t kERRFLG = ApplesoftVariables::ZP_ERRFLG;
    const std::uint8_t errFlags = ReadZeroPageByte(kERRFLG);

    // `bit ERRFLG` / `bpl` in ROM: when sign bit is set, ON ERR is active and
    // CONTROL-C dispatches to HANDLERR with code $FF semantics.
    if ((errFlags & 0x80u) != 0u) {
        SetPendingErrorCode(0xffu);
        HANDLERR();
        return;
    }

    // Control-C attempts to fall through to the STOP/END handler with an
    // implicit "break" condition.
    STOP_impl(true);
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

void STEP() {
    constexpr std::uint8_t kTOKEN_STEP = 0xc7u;

    LOAD_FAC_FROM_YA();
    if (CHRGOT() == kTOKEN_STEP) {
        CHRGET();
        FRMNUM();
    }

    SIGN();
    FRM_STACK_2();
    PushForPntFrame();
    NEWSTT();
}

void NEWSTT() {
    constexpr std::uint8_t kREMSTK = ApplesoftVariables::ZP_REMSTK;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;

    WriteZeroPageByte(kREMSTK, ReadStackPointer());

    if (ISCNTC()) {
        return;
    }

    if (ReadZeroPageByte(static_cast<std::uint8_t>(kCURLIN + 1u)) != 0xffu) {
        WriteZeroPageWord(kOLDTEXT, ReadZeroPageWord(kTXTPTR));
    } else {
        WriteZeroPageWord(kOLDTEXT, 0);
    }

    if (IsEndOfLineAtTextPointer()) {
        if (IsEndOfProgramAtTextPointer()) {
            GOEND();
            return;
        }
    }

    WriteZeroPageWord(kCURLIN, ReadLineNumberFromTextPointer());
    AdvanceTextPointerToNextLine();
    TRACE_();
}

void TRACE_() {
    if (IsTraceEnabled()) {
        if (IsRunningMode()) {
            OUTDO('#'&0x7fu);
            LINPRT();
            OUTSP();
        }
    }

    CHRGET();
    EXECUTE_STATEMENT();
    NEWSTT();
}

void GOEND() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GOEND (inclusive) .. EXECUTE_STATEMENT (exclusive)
    // Name normalization: none (assembler label GOEND kept verbatim).
    // End-of-program path in NEWSTT jumps into END4 with carry clear, which
    // restarts without printing BREAK. Model that directly here.
    RESTART();
}

bool IsEndOfLineAtTextPointer() {
    // Source: NEWSTT inline — ldy #0 / lda (TXTPTR),Y: end-of-statement when byte is 0.
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    return ReadProgramByte(ReadZeroPageWord(kTXTPTR)) == 0u;
}

bool IsEndOfProgramAtTextPointer() {
    // Source: NEWSTT inline — ldy #2 / lda (TXTPTR),Y: next-line link high byte;
    // if zero the program has ended (null forward pointer).
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    return ReadProgramByte(static_cast<std::uint16_t>(ReadZeroPageWord(kTXTPTR) + 2u)) == 0u;
}

std::uint16_t ReadLineNumberFromTextPointer() {
    // Source: NEWSTT inline — reads CURLIN from (TXTPTR)+3 and (TXTPTR)+4.
    // Memory layout at TXTPTR when it sits on an EOL 0x00:
    //   [0] = 0x00 (EOL), [1] = link.lo, [2] = link.hi, [3] = lineno.lo, [4] = lineno.hi.
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    const std::uint16_t txtptr = ReadZeroPageWord(kTXTPTR);
    const std::uint8_t lo = ReadProgramByte(static_cast<std::uint16_t>(txtptr + 3u));
    const std::uint8_t hi = ReadProgramByte(static_cast<std::uint16_t>(txtptr + 4u));
    return ApplesoftVariables::makeWord(lo, hi);
}

void AdvanceTextPointerToNextLine() {
    // Source: NEWSTT inline — tya (A=4) + adc TXTPTR → TXTPTR += 4.
    // CHRGET called next by TRACE_ adds 1 more, landing on the first content byte.
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(ReadZeroPageWord(kTXTPTR) + 4u));
}

bool IsRunningMode() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: TRACE_ (inclusive) .. EXECUTE_STATEMENT (exclusive)
    // Name normalization: helper name chosen for the inline TRACE_ predicate.
    // TRACE_ checks CURLIN+1 and only traces when non-zero (running mode).
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    return ReadZeroPageByte(static_cast<std::uint8_t>(kCURLIN + 1u)) != 0u;
}

bool IsTraceEnabled() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: TRACE_ (inclusive) .. EXECUTE_STATEMENT (exclusive)
    // Name normalization: helper name chosen for the inline TRACE_ predicate.
    // `bit TRCFLG` + `bpl` means tracing is enabled when TRCFLG bit 7 is set.
    constexpr std::uint8_t kTRCFLG = ApplesoftVariables::ZP_TRCFLG;
    return (ReadZeroPageByte(kTRCFLG) & 0x80u) != 0u;
}

void EXECUTE_STATEMENT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: EXECUTE_STATEMENT (inclusive) .. EXECUTE_STATEMENT_1 (exclusive)
    // Name normalization: none (assembler label EXECUTE_STATEMENT kept verbatim).

    if (CurrentStatementChar() == 0) {
        // EMPTY STATEMENT: fall through to caller behavior.
        return;
    }

    EXECUTE_STATEMENT_1();
}

void EXECUTE_STATEMENT_1() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: EXECUTE_STATEMENT_1 (inclusive) .. COLON_ (exclusive)
    // Name normalization: none (assembler label EXECUTE_STATEMENT_1 kept verbatim).

    const std::uint8_t ch = CurrentStatementChar();
    if ((ch & 0x80u) == 0u) {
        LET();
        return;
    }

    const std::uint8_t tokenIndex = static_cast<std::uint8_t>(ch - kTokenBase);
    if (tokenIndex >= 0x40u) {
        SYNERR();
        return;
    }

    CHRGET();
    const TOKEN_ADDRESS_TABLE_fn handler = TOKEN_ADDRESS_TABLE(static_cast<std::size_t>(tokenIndex));
    handler();
}

void COLON_() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: COLON_ (inclusive) .. RESTORE (exclusive)
    // Name normalization: none (assembler label COLON_ kept verbatim).

    if (CurrentStatementChar() == static_cast<std::uint8_t>(':' )) {
        TRACE_();
        return;
    }

    SYNERR();
}

void IF() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: IF (inclusive) .. REM (exclusive)
    // Name normalization: none (assembler label IF kept verbatim).

    constexpr std::uint8_t kTOKEN_GOTO = 0xabu;
    constexpr std::uint8_t kTOKEN_THEN = 0xc4u;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;

    FRMEVL();
    if (CHRGOT() != kTOKEN_GOTO) {
        SYNCHR(kTOKEN_THEN);
    }

    if (ReadZeroPageByte(kFAC) != 0u) {
        IF_TRUE();
        return;
    }

    // False IF falls through to REM in ROM.
    REM();
}

void REM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: REM (inclusive) .. IF_TRUE (exclusive)
    // Name normalization: none (assembler label REM kept verbatim).

    const std::uint8_t offset = REMN();
    ADDON(offset);
}

void IF_TRUE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: IF_TRUE (inclusive) .. ONGOTO (exclusive)
    // Name normalization: none (assembler label IF_TRUE kept verbatim).

    if (CHRGOT() >= kTokenBase) {
        EXECUTE_STATEMENT();
        return;
    }

    GOTO();
}

void ONGOTO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ONGOTO (inclusive) .. LINGET (exclusive)
    // Name normalization: none (assembler label ONGOTO kept verbatim).

    constexpr std::uint8_t kTOKEN_GOSUB = 0xb0u;
    constexpr std::uint8_t kTOKEN_GOTO = 0xabu;
    constexpr std::uint8_t kFAC_PLUS_4 = static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 4u);

    const std::uint8_t token = GETBYT();
    if (token != kTOKEN_GOSUB && token != kTOKEN_GOTO) {
        SYNERR();
        return;
    }

    while (true) {
        const std::uint8_t selector = ReadZeroPageByte(kFAC_PLUS_4);
        WriteZeroPageByte(kFAC_PLUS_4, static_cast<std::uint8_t>(selector - 1u));

        if (selector == 1u) {
            EXECUTE_STATEMENT_1();
            return;
        }

        CHRGET();
        LINGET();
        if (CHRGOT() == static_cast<std::uint8_t>(',')) {
            continue;
        }

        return;
    }
}

}  // namespace applesoft::asm_port