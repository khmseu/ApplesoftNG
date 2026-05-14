#include "core/asm_port_error.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/asm_port_token_address_table.hpp"
#include "core/io_ports.hpp"

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
void FRMNUM();
void FRMEVL();
void CHKNUM();
void PushForPntFrame();
void PushTextPointerAddress();
void PushCurrentLineNumber();
void PushTokenTo(std::uint8_t token);
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
std::uint16_t PTRGET();
void PULL3();
std::uint8_t REMN();
std::uint8_t DATAN();
bool FL1(std::uint16_t startAddress);
void HANDLERR();
void SetPendingErrorCode(std::uint8_t errorCode);
std::uint8_t INCHR();
void SetTextPointer(std::uint16_t address);
void ClearErrFlag();
void MarkDirectMode();
void PARSE_INPUT_LINE();
void HandleNumberedLine();
void CRDO();
void SETFOR();
void ROUND_FAC();

constexpr std::uint8_t kTokenBase = 0x80u;
constexpr std::uint8_t RESTART_PROMPT = ']' | 0x80u;

void STOP_impl(bool shouldPrintBreak);
void ENDX_impl(bool shouldPrintBreak);

namespace {

constexpr std::uint16_t kStepLabelAddress = 0x07afu;

std::uint8_t readStackByteAt(std::uint8_t x, std::uint8_t plus) {
    const std::uint8_t offset = static_cast<std::uint8_t>(x + plus);
    return ReadProgramByte(static_cast<std::uint16_t>(0x0100u + offset));
}

void ApplyFacSign() {
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kFAC_SIGN = ApplesoftVariables::ZP_FAC_SIGN;

    const std::uint8_t facSign = ReadZeroPageByte(kFAC_SIGN);
    const std::uint8_t facMantissaHigh = ReadZeroPageByte(kFAC + 1u);
    // ROM sequence at $079C: LDA FAC_SIGN / ORA #$7F / AND FAC+1 / STA FAC+1.
    // This clears bit 7 for positive values and preserves FAC+1 when FAC_SIGN is negative.
    const std::uint8_t signedMantissaHigh =
        static_cast<std::uint8_t>(facMantissaHigh & (facSign | 0x7fu));
    WriteZeroPageByte(kFAC + 1u, signedMantissaHigh);
}

void SetBranchTargetToSTEP() {
    constexpr std::uint8_t kINDEX = ApplesoftVariables::ZP_INDEX;

    WriteZeroPageWord(kINDEX, kStepLabelAddress);
}

void LOAD_FAC_FROM_YA() {
    // TODO(asm-port): load the constant 1.0 into FAC from the Y,A pointer.
}

std::int8_t SIGN2(std::uint8_t sign) {
    // Labels: SIGN2 (inclusive) .. SGN (exclusive)
    // MSBIT to carry, then return -1 if carry set, +1 if carry clear.
    if ((sign & 0x80u) != 0u) {
        return -1;
    }
    return 1;
}

std::int8_t SIGN1() {
    // Labels: SIGN1 (inclusive) .. SIGN2 (exclusive)
    return SIGN2(ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN));
}

std::int8_t SIGN() {
    // Labels: SIGN (inclusive) .. SIGN2 (exclusive)
    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC) == 0u) {
        return 0; // Numbers are effectively zero
    }
    return SIGN1();
}

void FCOMP2() {
    // TODO(asm-port): move implementation to return int8 once caller is updated.
    // Labels: FCOMP2 (inclusive) .. L_FCOMP2_1 (exclusive)
    // Target branches to L_FCOMP2_1 or RTS depending on comparison.
}

void FRM_STACK_2(std::uint8_t signByte) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FRM_STACK_2 (inclusive) .. FRM_STACK_3 (exclusive)
    // Name normalization: none (assembler label FRM_STACK_2 kept verbatim).
    constexpr std::uint8_t kINDEXZeroPageAddress = ApplesoftVariables::ZP_INDEX;
    constexpr std::uint8_t kINDEXHighByteAddress =
        static_cast<std::uint8_t>(kINDEXZeroPageAddress + 1u);

    const std::uint8_t returnAddressLow = PopByteFromStack();
    // Net effect of ROM sequence PLA / STA INDEX / INC INDEX:
    // store the low return-address byte plus one as an 8-bit value so INDEX
    // points at the byte immediately after the JSR call-site return location.
    // The original routine explicitly assumes no page-boundary carry into INDEX+1.
    WriteZeroPageByte(kINDEXZeroPageAddress, static_cast<std::uint8_t>(returnAddressLow + 1u));
    WriteZeroPageByte(kINDEXHighByteAddress, PopByteFromStack());

    PushByteToStack(signByte);
}

void FRM_STACK_3() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FRM_STACK_3 (inclusive) .. NOTMATH (exclusive)
    // Name normalization: none (assembler label FRM_STACK_3 kept verbatim).
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kINDEX = ApplesoftVariables::ZP_INDEX;

    ROUND_FAC();

    PushByteToStack(ReadZeroPageByte(kFAC + 4u));
    PushByteToStack(ReadZeroPageByte(kFAC + 3u));
    PushByteToStack(ReadZeroPageByte(kFAC + 2u));
    PushByteToStack(ReadZeroPageByte(kFAC + 1u));
    PushByteToStack(ReadZeroPageByte(kFAC));

    const std::uint16_t branchTarget = ReadZeroPageWord(kINDEX);
    if (branchTarget == kStepLabelAddress) {
        STEP();
    }
    // Other indirect targets used by FRM_STACK_3 are not ported yet; return to caller.
}

constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
    return static_cast<std::uint8_t>(lhs + rhs);
}

std::uint16_t readStackWordAt(std::uint8_t x, std::uint8_t lowOffset, std::uint8_t highOffset) {
    return ApplesoftVariables::makeWord(readStackByteAt(x, lowOffset), readStackByteAt(x, highOffset));
}

// TODO(asm-port): port FADD label.
void FADD() {}

// TODO(asm-port): decide branch condition after comparing FOR value with end value.
bool NEXT_shouldTerminateLoop() {
    return false;
}

std::uint8_t ScanAheadOffset(std::uint8_t terminator) {
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kCHARAC = ApplesoftVariables::ZP_CHARAC;
    constexpr std::uint8_t kENDCHR = ApplesoftVariables::ZP_ENDCHR;

    WriteZeroPageByte(kCHARAC, terminator);
    std::uint8_t offset = 0;
    WriteZeroPageByte(kENDCHR, 0);

    while (true) {
        const std::uint8_t previousEnd = ReadZeroPageByte(kENDCHR);
        const std::uint8_t previousCharac = ReadZeroPageByte(kCHARAC);
        WriteZeroPageByte(kCHARAC, previousEnd);
        WriteZeroPageByte(kENDCHR, previousCharac);

        while (true) {
            const ProgramPointer textPtr{ReadZeroPageWord(kTXTPTR)};
            const std::uint8_t ch = textPtr.read(offset);
            if (ch == 0 || ch == ReadZeroPageByte(kENDCHR)) {
                return offset;
            }

            ++offset;
            if (ch == static_cast<std::uint8_t>('"')) {
                break;
            }
        }
    }
}

bool isDigit(std::uint8_t ch) {
    return ch >= '0' && ch <= '9';
}

}  // namespace

void SETFOR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SETFOR (inclusive) .. COPY_ARG_TO_FAC (exclusive)
    // Name normalization: none (assembler label SETFOR kept verbatim).
    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kFAC_SIGN = ApplesoftVariables::ZP_FAC_SIGN;
    constexpr std::uint8_t kFAC_EXTENSION = ApplesoftVariables::ZP_FAC_EXTENSION;

    ROUND_FAC();

    const ProgramPointer forVariablePtr{ReadZeroPageWord(kFORPNT)};
    forVariablePtr.write(ReadZeroPageByte(kFAC), 0u);

    const std::uint8_t facMantissaHigh = ReadZeroPageByte(add_u8(kFAC, 1u));
    const std::uint8_t facSign = ReadZeroPageByte(kFAC_SIGN);
    // ROM sequence: (FAC+1) is masked by (FAC_SIGN | $7F), preserving mantissa
    // low 7 bits while applying sign-bit packing semantics.
    const std::uint8_t packedMantissaHigh = static_cast<std::uint8_t>(
        facMantissaHigh & static_cast<std::uint8_t>(facSign | 0x7fu));
    forVariablePtr.write(packedMantissaHigh, 1u);
    forVariablePtr.write(ReadZeroPageByte(add_u8(kFAC, 2u)), 2u);
    forVariablePtr.write(ReadZeroPageByte(add_u8(kFAC, 3u)), 3u);
    forVariablePtr.write(ReadZeroPageByte(add_u8(kFAC, 4u)), 4u);

    WriteZeroPageByte(kFAC_EXTENSION, 0u);
}

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

bool ISCNTC() {
    constexpr std::uint8_t kCTRL_C_CODE = 0x83;

    if (ioPorts_const().readByte(IOPorts::ADDR_KEYBOARD) != kCTRL_C_CODE) {
        return false;
    }

    INCHR();
    CONTROL_C_TYPED();
    return true;
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

void RESTART() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RESTART (inclusive)
    // Name normalization: none (assembler label RESTART kept verbatim).

    CRDO();
    const Inlin2Result inlin2 = INLIN2(RESTART_PROMPT);
    SetTextPointer(inlin2.address());
    ClearErrFlag();

    const std::uint8_t firstChar = CHRGET();
    if (firstChar == 0) {
        RESTART();
        return;
    }

    MarkDirectMode();

    if (isDigit(firstChar)) {
        HandleNumberedLine();
        return;
    }

    PARSE_INPUT_LINE();
    TRACE_();
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

void RESUME() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RESUME (inclusive) .. JSYN (exclusive)
    // Name normalization: none (assembler label RESUME kept verbatim).
    constexpr std::uint8_t kERRLIN = ApplesoftVariables::ZP_ERRLIN;
    constexpr std::uint8_t kERRPOS = ApplesoftVariables::ZP_ERRPOS;
    constexpr std::uint8_t kERRSTK = ApplesoftVariables::ZP_ERRSTK;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    WriteZeroPageWord(kCURLIN, ReadZeroPageWord(kERRLIN));
    WriteZeroPageWord(kTXTPTR, ReadZeroPageWord(kERRPOS));
    SetStackPointer(ReadZeroPageByte(kERRSTK));
    NEWSTT();
}

void ONERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ONERR (inclusive) .. HANDLERR (exclusive)
    // Name normalization: none (assembler label ONERR kept verbatim).
    constexpr std::uint8_t kTOKEN_GOTO = 0xabu;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kTXTPSV = ApplesoftVariables::ZP_TXTPSV;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kCURLSV = ApplesoftVariables::ZP_CURLSV;
    constexpr std::uint8_t kERRFLG = ApplesoftVariables::ZP_ERRFLG;

    SYNCHR(kTOKEN_GOTO);
    WriteZeroPageWord(kTXTPSV, ReadZeroPageWord(kTXTPTR));

    const std::uint8_t errflg = ReadZeroPageByte(kERRFLG);
    WriteZeroPageByte(kERRFLG, static_cast<std::uint8_t>((errflg >> 1u) | 0x80u));

    WriteZeroPageWord(kCURLSV, ReadZeroPageWord(kCURLIN));
    ADDON(REMN());
}

void RTS_5() {
    // Shared RTS target for GOTO/POP in ROM.
}

void PULL3() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PULL3 (inclusive) .. IF (exclusive)
    // Name normalization: none (assembler label PULL3 kept verbatim).

    (void)PopByteFromStack();
    (void)PopByteFromStack();
    (void)PopByteFromStack();
}

std::uint8_t REMN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: REMN (inclusive) .. PULL3 (exclusive)
    // Name normalization: none (assembler label REMN kept verbatim).

    return ScanAheadOffset(0);
}

void PushForPntFrame() {
    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    PushByteToStack(ReadZeroPageByte(add_u8(kFORPNT, 1u)));
    PushByteToStack(ReadZeroPageByte(kFORPNT));
    PushTokenTo(TOKEN_FOR);
}

void FOR() {
    constexpr std::uint8_t kSUBFLG = ApplesoftVariables::ZP_SUBFLG;
    constexpr std::uint8_t kTOKEN_TO = 0xc1u;

    WriteZeroPageByte(kSUBFLG, 0x80);
    LET();

    GTFORPNTState gtforpntState{};
    for (std::size_t i = 0; i < gtforpntState.stackPage.size(); ++i) {
        gtforpntState.stackPage[i] =
            ReadProgramByte(static_cast<std::uint16_t>(0x0100u + i));
    }
    const auto gtforpntResult = GTFORPNT(ReadStackPointer(), gtforpntState);
    if (gtforpntResult.found) {
        SetStackPointer(add_u8(gtforpntResult.x, 15u));
    }

    PopReturnAddress();
    PopReturnAddress();

    CHKMEMState chkmemState{};
    chkmemState.a = 9;
    chkmemState.stackPointer = ReadStackPointer();
    const auto chkmemResult = CHKMEM(chkmemState);
    if (!chkmemResult.ok) {
        return;
    }

    DATAN();
    PushTextPointerAddress();
    PushCurrentLineNumber();
    PushTokenTo(kTOKEN_TO);
    SYNCHR(kTOKEN_TO);
    CHKNUM();
    FRMNUM();
    ApplyFacSign();
    SetBranchTargetToSTEP();
    FRM_STACK_3();
}

void NEXT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEXT (inclusive) .. FRMNUM (exclusive)
    // Name normalization: none (assembler label NEXT kept verbatim).

    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    // d0 04 / NEXT_1 jsr PTRGET / NEXT_2 sta FORPNT, sty FORPNT+1
    // No-variable NEXT case is represented by FORPNT+1 = 0.
    if (CHRGOT() == 0u) {
        WriteZeroPageByte(add_u8(kFORPNT, 1u), 0u);
    } else {
        const std::uint16_t varPtr = PTRGET();
        WriteZeroPageWord(kFORPNT, varPtr);
    }

    // jsr GTFORPNT
    GTFORPNTState gtforpntState{};
    gtforpntState.forpntLo = ReadZeroPageByte(kFORPNT);
    gtforpntState.forpntHi = ReadZeroPageByte(add_u8(kFORPNT, 1u));
    for (std::size_t i = 0; i < gtforpntState.stackPage.size(); ++i) {
        gtforpntState.stackPage[i] =
            ReadProgramByte(static_cast<std::uint16_t>(0x0100u + i));
    }

    const auto gtforpntResult = GTFORPNT(ReadStackPointer(), gtforpntState);
    if (!gtforpntResult.found) {
        // Ldx #ERR_NOFOR / jmp ERROR via GERR/JERROR path.
        ERROR(ERR_NOFOR);
        return;
    }

    // NEXT_3: txs
    SetStackPointer(gtforpntResult.x);

    // STEP arithmetic path (LOAD_FAC_FROM_YA / FADD / SETFOR / FCOMP2).
    // Stack offsets follow ROM comments; helpers are placeholders until stack
    // memory and FAC math ports are fully wired.
    LOAD_FAC_FROM_YA();
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, readStackByteAt(gtforpntResult.x, 9u)); // FAC_SIGN
    WriteZeroPageWord(kFORPNT, ReadZeroPageWord(kFORPNT));
    FADD();
    SETFOR();
    FCOMP2();

    if (!NEXT_shouldTerminateLoop()) {
        // Restore line/TXTPTR from FOR frame and jump NEWSTT.
        const std::uint16_t restoredLine = readStackWordAt(gtforpntResult.x, 15u, 16u);
        const std::uint16_t restoredTextPointer = readStackWordAt(gtforpntResult.x, 18u, 17u);
        WriteZeroPageWord(kCURLIN, restoredLine);
        WriteZeroPageWord(kTXTPTR, restoredTextPointer);
        NEWSTT();
        return;
    }

    // L_NEXT_3_2: pop FOR frame, then continue NEWSTT unless another variable
    // follows in NEXT var-list (NEXT I,J,...).
    SetStackPointer(add_u8(gtforpntResult.x, 18u));

    if (CHRGOT() != static_cast<std::uint8_t>(',')) {
        NEWSTT();
        return;
    }

    CHRGET();
    // jsr NEXT_1 (does not return in ROM when comma-separated variables remain).
    NEXT();
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

    const std::int8_t stepSign = SIGN();
    FRM_STACK_2(static_cast<std::uint8_t>(stepSign));
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
