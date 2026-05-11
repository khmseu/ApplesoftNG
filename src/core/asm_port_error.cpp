#include "core/asm_port_error.hpp"
#include "core/asm_port_print.hpp"
#include "platform/asm_port_outdo.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_token_address_table.hpp"
#include "core/asm_port_token_name_table.hpp"

#include <optional>
#include <string_view>

namespace applesoft::asm_port {

constexpr std::uint8_t RESTART_PROMPT = ']' | 0x80u;
constexpr std::size_t kTokenCount = 107;
constexpr std::uint8_t kTokenBase = 0x80u;

bool isDigit(std::uint8_t ch) {
    return ch >= '0' && ch <= '9';
}

std::uint8_t CHRGET() {
    // TODO(asm-port): read the next character from the current input buffer.
    return 0;
}

void SetTextPointer(std::uint16_t address) {
    variables().writeWord(0x00b8u, address);
}

void SetTextPointer(std::uint8_t lo, std::uint8_t hi) {
    SetTextPointer(ApplesoftVariables::makeWord(lo, hi));
}

void ClearErrFlag() {
    variables().writeByte(0x00d8u, 0);
}

void MarkDirectMode() {
    variables().writeByte(0x0076u, 0xffu);
}

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
std::uint8_t CHRGOT();
void SYNERR();

void LINGET() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LINGET (inclusive) .. LET (exclusive)
    // Name normalization: none (assembler label LINGET kept verbatim).

    constexpr std::uint8_t kLINNUM = 0x50;
    constexpr std::uint8_t kINDEX = 0x5e;
    constexpr std::uint8_t kCHARAC = 0x0d;

    WriteZeroPageWord(kLINNUM, 0);

    std::uint8_t current = CHRGOT();
    while (isDigit(current)) {
        const std::uint8_t digit = static_cast<std::uint8_t>(current - static_cast<std::uint8_t>('0'));
        WriteZeroPageByte(kCHARAC, digit);

        const std::uint8_t lineHigh = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1));
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

std::uint8_t ReadZeroPageByte(std::uint8_t address) {
    return variables_const().readByte(address);
}

void WriteZeroPageByte(std::uint8_t address, std::uint8_t value) {
    variables().writeByte(address, value);
}

void WriteZeroPageWord(std::uint8_t address, std::uint16_t value) {
    variables().writeWord(address, value);
}

std::uint16_t ReadZeroPageWord(std::uint8_t address) {
    return variables_const().readWord(address);
}

void RESTORE();
void SETDA(std::uint16_t dataPointer);
void CONTROL_C_TYPED();
void STOP();
void ENDX();
void CONT();
void SAVE();
void LOAD();
void RUN();
void GOSUB();
void GO_TO_LINE();
void GOTO();
void POP();
void RETURN();
void VARTIO();
void PROGIO();
void STOP_impl(bool shouldPrintBreak);
void ENDX_impl(bool shouldPrintBreak);
void EXECUTE_STATEMENT_1();
std::uint8_t CurrentStatementChar();
void SYNERR();

void PRINT_ERROR_LINNUM();
void PRINT_ERROR_LINNUM(std::string_view prefix);

void WriteProgramByte(std::uint16_t address, std::uint8_t value) {
    // TODO(asm-port): write a byte into program memory at the given address.
    (void)address;
    (void)value;
}

void SetStackPointer(std::uint8_t value) {
    // TODO(asm-port): set the 6502 stack pointer.
    (void)value;
}

bool IsStatementEndOfParsedInput() {
    // TODO(asm-port): determine whether the current statement has no trailing text.
    return true;
}

bool NEW_impl();
void SCRTCH_impl();
bool SETPTRS_impl();
bool CLEAR_impl();
void CLEARC_impl();
void STXTPT_impl();

std::uint8_t ReadStackPointer();
void PopReturnAddress();
void PushByteToStack(std::uint8_t value);
void PushTextPointerAddress();
void PushCurrentLineNumber();
void PushTokenTo(std::uint8_t token);
void ApplyFacSign();
void SetBranchTargetToSTEP();
void LOAD_FAC_FROM_YA();
void SYNCHR(std::uint8_t expected);
void CHKNUM();
void CHKSTR();
void FRMNUM();
void SIGN();
void FRM_STACK_2();
void FRM_STACK_3();
void MON_WRITE();
void MON_READ();
void ADDON(std::uint8_t offset);
std::uint8_t DATAN();
void GOEND();
bool IsEndOfLineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadLineNumberFromTextPointer();
void AdvanceTextPointerToNextLine();
bool IsRunningMode();
bool IsTraceEnabled();
std::uint8_t REMN();
bool FL1(std::uint16_t startAddress);
bool FL1(std::uint8_t startLo, std::uint8_t startHi);
std::uint8_t PopByteFromStack();
bool ReturnWasFromPOPContext();
std::uint8_t PeekTopControlTokenAfterGTFORPNT();
void PULL3();
void RTS_5();
void OUTSP();
void EXECUTE_STATEMENT();
void PushForPntFrame();

std::uint8_t CHRGOT();
bool ISCNTC();
void LINPRT();
// void OUTDO();
std::uint8_t MEMERR();

void LET();
void LET2(std::uint8_t savedValTypPlus1);
void PUTSTR();
void IF();
void REM();
void IF_TRUE();
void ONGOTO();
void FOR();
void NEXT();
void STEP();
void NEWSTT();
void TRACE_();
void FRMEVL();
std::uint8_t GETBYT();
// std::uint16_t PTRGET();
bool CHKVAL(std::uint8_t savedValTyp);
void ROUND_FAC();
void AYINT();
void SETFOR();
void STRINI(std::uint8_t length);
void MOVINS();
void FRETMS();
void SCREEN();
void UNARY();
void OR();
void ANDOP();
void FALSE();
void TRUE();
void RELOPS();
void STRCMP();
void NUMCMP();
void CMPDONE();
void CHKCOM();
void PDL();
void NXDIM();
void DIM();
void PTRGET2();
void PTRGET3();
void BADNAM();
void NAMOK();
void BASIC();
void BASIC2();
void PTRGET4();
bool ISLETC();
void NAME_NOT_FOUND();
void C_ZERO();
void MAKE_NEW_VARIABLE();
void SET_VARPNT_AND_YA();
void GETARY();
void GETARY2();
void NEG32768();
void MAKINT();
void MKINT();
void MI1();
void MI2();
void ARRAY();
void SUBERR();
void IQERR();
void JER();
void USE_OLD_ARRAY();
void MAKE_NEW_ARRAY();
void FIND_ARRAY_ELEMENT();
void FAE_1();
void GSE();
void GME();

bool NEW_impl() {
    if (!IsStatementEndOfParsedInput()) {
        return false;
    }

    SCRTCH_impl();
    return true;
}

void SCRTCH_impl() {
    constexpr std::uint8_t kTXTTAB = 0x67;
    constexpr std::uint8_t kLOCK = 0xd6;
    constexpr std::uint8_t kVARTAB = 0x69;
    constexpr std::uint8_t kPRGEND = 0xaf;
    constexpr std::uint8_t kARYTAB = 0x6b;
    constexpr std::uint8_t kSTREND = 0x6d;
    constexpr std::uint8_t kMEMSIZ = 0x73;
    constexpr std::uint8_t kFRETOP = 0x6f;

    const std::uint16_t programStart = ReadZeroPageWord(kTXTTAB);
    WriteZeroPageByte(kLOCK, 0);
    WriteProgramByte(programStart, 0);
    WriteProgramByte(static_cast<std::uint16_t>(programStart + 1), 0);

    const std::uint16_t nextFree = static_cast<std::uint16_t>(programStart + 2);
    WriteZeroPageWord(kVARTAB, nextFree);
    WriteZeroPageWord(kPRGEND, nextFree);
    WriteZeroPageWord(kFRETOP, ReadZeroPageWord(kMEMSIZ));
    WriteZeroPageWord(kARYTAB, ReadZeroPageWord(kVARTAB));
    WriteZeroPageWord(kSTREND, ReadZeroPageWord(kVARTAB));

    SETPTRS_impl();
}

bool SETPTRS_impl() {
    STXTPT_impl();
    return CLEAR_impl();
}

bool CLEAR_impl() {
    if (!IsStatementEndOfParsedInput()) {
        return false;
    }

    CLEARC_impl();
    return true;
}

void CLEARC_impl() {
    constexpr std::uint8_t kMEMSIZ = 0x73;
    constexpr std::uint8_t kFRETOP = 0x6f;
    constexpr std::uint8_t kVARTAB = 0x69;
    constexpr std::uint8_t kARYTAB = 0x6b;
    constexpr std::uint8_t kSTREND = 0x6d;

    WriteZeroPageWord(kFRETOP, ReadZeroPageWord(kMEMSIZ));
    WriteZeroPageWord(kARYTAB, ReadZeroPageWord(kVARTAB));
    WriteZeroPageWord(kSTREND, ReadZeroPageWord(kVARTAB));
    RESTORE();
    STKINI();
}

void STXTPT_impl() {
    constexpr std::uint8_t kTXTTAB = 0x67;
    constexpr std::uint8_t kTXTPTR = 0xb8;

    const std::uint16_t textTable = ReadZeroPageWord(kTXTTAB);
    WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(textTable - 1u));
}

void FOR() {
    constexpr std::uint8_t kSUBFLG = 0x14;
    constexpr std::uint8_t kTOKEN_TO = 0x00; // TODO(asm-port): actual Applesoft "TO" token value.

    WriteZeroPageByte(kSUBFLG, 0x80);
    LET();

    GTFORPNTState gtforpntState{};
    const auto gtforpntResult = GTFORPNT(ReadStackPointer(), gtforpntState);
    if (gtforpntResult.found) {
        SetStackPointer(static_cast<std::uint8_t>(gtforpntResult.x + 15));
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

namespace {

// TODO(asm-port): provide stack-page reads wired to the 6502 runtime stack.
std::uint8_t readStackByteAt(std::uint8_t /*x*/, std::uint8_t /*plus*/) {
    return 0;
}

std::uint16_t readStackWordAt(std::uint8_t x, std::uint8_t lowOffset, std::uint8_t highOffset) {
    return ApplesoftVariables::makeWord(readStackByteAt(x, lowOffset), readStackByteAt(x, highOffset));
}

// TODO(asm-port): port FADD label.
void FADD() {}

// TODO(asm-port): port FCOMP2 label.
void FCOMP2() {}

// TODO(asm-port): decide branch condition after comparing FOR value with end value.
bool NEXT_shouldTerminateLoop() {
    return false;
}

std::int8_t gNumericCompareResult = 0;
bool gNumericCompareCarry = false;
std::uint8_t gFloatInput = 0;

// TODO(asm-port): port PLOTFNS label.
void PLOTFNS() {}

// TODO(asm-port): port MON_SCRN monitor handler.
std::uint8_t MON_SCRN(std::uint8_t /*row*/, std::uint8_t /*column*/) {
    return 0;
}

// TODO(asm-port): port FCOMP label.
std::int8_t FCOMP(std::uint16_t /*argAddress*/) {
    return 0;
}

// TODO(asm-port): port FREFAC label.
// void FREFAC() {}

// TODO(asm-port): port FRETMP label.
void FRETMP() {}

// TODO(asm-port): port FLOAT label.
void FLOAT() {}

// TODO(asm-port): port CONINT label.
void CONINT() {}

// TODO(asm-port): port MON_PREAD monitor paddle reader.
std::uint8_t MON_PREAD() {
    return 0;
}

// TODO(asm-port): port QINT label.
void QINT() {}

// TODO(asm-port): port COLD_START label.
void COLD_START() {}

std::uint8_t gJerErrorCode = ERR_SYNTAX;
constexpr std::uint8_t kNEG32768Data[4] = {0x90u, 0x80u, 0x00u, 0x00u};
constexpr std::uint8_t kCZeroData[2] = {0x00u, 0x00u};

void SNGFLT(std::uint8_t value) {
    // TODO(asm-port): replace with true SNGFLT conversion routine.
    WriteZeroPageByte(0x9du, value);
    WriteZeroPageByte(0x9eu, 0u);
    WriteZeroPageByte(0x9fu, 0u);
    WriteZeroPageByte(0xa0u, 0u);
    WriteZeroPageByte(0xa1u, 0u);
}

// TODO(asm-port): compare temporary ARG and FAC strings and return -1/0/1.
std::int8_t CompareArgAndFacStrings() {
    return 0;
}

} // namespace

void NEXT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEXT (inclusive) .. FRMNUM (exclusive)
    // Name normalization: none (assembler label NEXT kept verbatim).

    constexpr std::uint8_t kFORPNT = 0x85;
    constexpr std::uint8_t kCURLIN = 0x75;
    constexpr std::uint8_t kTXTPTR = 0xb8;

    // d0 04 / NEXT_1 jsr PTRGET / NEXT_2 sta FORPNT, sty FORPNT+1
    // No-variable NEXT case is represented by FORPNT+1 = 0.
    if (CHRGOT() == 0u) {
        WriteZeroPageByte(static_cast<std::uint8_t>(kFORPNT + 1), 0u);
    } else {
        const std::uint16_t varPtr = PTRGET();
        WriteZeroPageWord(kFORPNT, varPtr);
    }

    // jsr GTFORPNT
    GTFORPNTState gtforpntState{};
    gtforpntState.forpntLo = ReadZeroPageByte(kFORPNT);
    gtforpntState.forpntHi = ReadZeroPageByte(static_cast<std::uint8_t>(kFORPNT + 1));
    // TODO(asm-port): populate gtforpntState.stackPage from runtime stack memory.

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
    WriteZeroPageByte(0xa2u, readStackByteAt(gtforpntResult.x, 9u)); // FAC_SIGN
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
    SetStackPointer(static_cast<std::uint8_t>(gtforpntResult.x + 18u));

    if (CHRGOT() != static_cast<std::uint8_t>(',')) {
        NEWSTT();
        return;
    }

    CHRGET();
    // jsr NEXT_1 (does not return in ROM when comma-separated variables remain).
    NEXT();
}

void STEP() {
    constexpr std::uint8_t kTOKEN_STEP = 0x00; // TODO(asm-port): actual Applesoft "STEP" token value.

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
    constexpr std::uint8_t kREMSTK = 0xf8;
    constexpr std::uint8_t kTXTPTR = 0xb8;
    constexpr std::uint8_t kCURLIN = 0x75;
    constexpr std::uint8_t kOLDTEXT_lo = 0x79;

    WriteZeroPageByte(kREMSTK, ReadStackPointer());

    if (ISCNTC()) {
        return;
    }

    if (ReadZeroPageByte(static_cast<std::uint8_t>(kCURLIN + 1)) != 0xffu) {
        WriteZeroPageWord(kOLDTEXT_lo, ReadZeroPageWord(kTXTPTR));
    } else {
        WriteZeroPageWord(kOLDTEXT_lo, 0);
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
    constexpr std::uint8_t kTRCFLG = 0xf2;

    if ((ReadZeroPageByte(kTRCFLG) & 0x80u) != 0u) {
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

std::uint8_t ReadProgramByte(std::uint16_t address) {
    // TODO(asm-port): read a byte from the program memory buffer.
    (void)address;
    return 0;
}

struct ProgramPointer {
    std::uint16_t address = 0;

    std::uint8_t read(std::uint16_t offset = 0) const {
        return ReadProgramByte(static_cast<std::uint16_t>(address + offset));
    }

    ProgramPointer advanced(std::uint16_t offset) const {
        return ProgramPointer{static_cast<std::uint16_t>(address + offset)};
    }
};

struct LineAddress;
std::uint16_t ToWord(LineAddress address);
bool IsEndOfProgram(LineAddress current);
bool IsEndOfProgram(ProgramPointer currentPtr);
ProgramPointer AdvanceToNextLine(ProgramPointer currentPtr);

std::uint8_t CHRGOT();
struct LineAddress {
    std::uint8_t lo = 0;
    std::uint8_t hi = 0;
};

LineAddress FromWord(std::uint16_t value);

bool ISCNTC();
void LINPRT();
// void OUTDO();
std::uint8_t GETCHR();

struct LineNumber {
    std::uint8_t lo = 0;
    std::uint8_t hi = 0;
};

bool IsLineNumberGreater(LineNumber current, LineNumber limit) {
    if (current.hi != limit.hi) {
        return current.hi > limit.hi;
    }
    return current.lo > limit.lo;
}

LineNumber ReadProgramLineNumber(ProgramPointer currentPtr) {
    return LineNumber{currentPtr.read(2u), currentPtr.read(3u)};
}

void PrintListLine(ProgramPointer currentPtr) {
    std::uint8_t offset = 4;
    while (true) {
        const std::uint8_t ch = currentPtr.read(offset);
        if (ch == 0) {
            break;
        }

        // TODO(asm-port): reproduce LIST token/keyword conversion and output
        // behavior from the original Applesoft source.
        OUTDO(ch&0x7fu);
        ++offset;
    }
}

bool FNDLIN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNDLIN (inclusive) .. FL1 (exclusive)
    // Name normalization: none (assembler label FNDLIN kept verbatim).

    constexpr std::uint8_t kTXTTAB = 0x67;

    // Assembler falls through from FNDLIN directly into FL1 with A=TXTTAB, X=TXTTAB+1.
    return FL1(ReadZeroPageWord(kTXTTAB));
}

void DeleteExistingLine() {
    // TODO(asm-port): delete the existing numbered line and shift later lines down.
}

void InsertNewLine() {
    // TODO(asm-port): make room and copy the new numbered line into the program listing.
}

std::uint8_t read_INPUT_BUFFER(std::uint8_t index) {
    return variables_const().pointer(0x0200u).read(index);
}

void write_INPUT_BUFFER_minus_5(std::uint8_t index, std::uint8_t value) {
    variables().pointer(0x01fbu).write(value, index);
}

void SetTextPointerToInputBufferMinus1() {
    // TODO(asm-port): compute the actual INPUT_BUFFER-1 address in the runtime model.
    SetTextPointer(0x01ffu);
}

struct TokenMatch {
    std::uint8_t code;
    std::uint8_t length;
    std::string_view name;
};

std::optional<TokenMatch> MatchToken(std::uint8_t index) {
    std::optional<TokenMatch> best;

    for (std::size_t i = 0; i < kTokenCount; ++i) {
        const std::string_view token = TOKEN_NAME_TABLE(i);
        if (token.empty()) {
            continue;
        }

        std::uint8_t current = index;
        bool matched = true;
        for (char expected : token) {
            if (read_INPUT_BUFFER(current) != static_cast<std::uint8_t>(expected)) {
                matched = false;
                break;
            }
            ++current;
        }

        if (!matched) {
            continue;
        }

        if (token == "AT") {
            const std::uint8_t next = read_INPUT_BUFFER(current);
            if (next == static_cast<std::uint8_t>('N') || next == static_cast<std::uint8_t>('O')) {
                continue;
            }
        }

        std::uint8_t tokenCode = static_cast<std::uint8_t>(kTokenBase + i);
        if (!best || token.size() > best->length) {
            best = TokenMatch{tokenCode,
                              static_cast<std::uint8_t>(token.size()),
                              token};
        }
    }

    return best;
}

void PARSE_INPUT_LINE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PARSE_INPUT_LINE (inclusive) .. FNDLIN (exclusive)
    // Name normalization: none (assembler label PARSE_INPUT_LINE kept verbatim).

    std::uint8_t inputIndex = 0;
    std::uint8_t outputIndex = 0;
    bool inRem = false;

    while (true) {
        const std::uint8_t ch = read_INPUT_BUFFER(inputIndex);
        if (ch == 0) {
            break;
        }

        if (inRem) {
            write_INPUT_BUFFER_minus_5(outputIndex++, ch);
            ++inputIndex;
            continue;
        }

        if (ch == static_cast<std::uint8_t>(' ')) {
            ++inputIndex;
            continue;
        }

        if (ch == static_cast<std::uint8_t>(0x22)) { // '"'
            write_INPUT_BUFFER_minus_5(outputIndex++, ch);
            ++inputIndex;
            while (true) {
                const std::uint8_t quoteChar = read_INPUT_BUFFER(inputIndex);
                if (quoteChar == 0) {
                    break;
                }
                write_INPUT_BUFFER_minus_5(outputIndex++, quoteChar);
                ++inputIndex;
                if (quoteChar == static_cast<std::uint8_t>(0x22)) {
                    break;
                }
            }
            continue;
        }

        const auto token = MatchToken(inputIndex);
        if (token.has_value()) {
            const TokenMatch match = *token;
            write_INPUT_BUFFER_minus_5(outputIndex++, match.code);

            if (match.name == "REM") {
                inputIndex += match.length;
                inRem = true;
                continue;
            }

            inputIndex += match.length;
            continue;
        }

        write_INPUT_BUFFER_minus_5(outputIndex++, ch);
        ++inputIndex;
    }

    write_INPUT_BUFFER_minus_5(outputIndex, 0);
    SetTextPointerToInputBufferMinus1();
}

void LIST() {
    constexpr std::uint8_t kLOWTR = 0x9b;
    constexpr std::uint8_t kLINNUM = 0x50;
    constexpr std::uint8_t kMON_CH = 0x24;

    if (!IsStatementEndOfParsedInput()) {
        return;
    }

    LINGET();
    FNDLIN();

    const std::uint8_t rangeChar = CHRGOT();
    if (rangeChar == static_cast<std::uint8_t>('-') || rangeChar == static_cast<std::uint8_t>(',')) {
        CHRGET();
        LINGET();
    }

    LineNumber endRange{ReadZeroPageByte(kLINNUM), ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1))};
    if (endRange.lo == 0 && endRange.hi == 0) {
        endRange.lo = 0xff;
        endRange.hi = 0xff;
    }

    ProgramPointer currentPtr{ReadZeroPageWord(kLOWTR)};
    while (!IsEndOfProgram(currentPtr)) {
        if (ISCNTC()) {
            break;
        }

        CRDO();
        const LineNumber currentLine = ReadProgramLineNumber(currentPtr);
        if (IsLineNumberGreater(currentLine, endRange)) {
            break;
        }

        LINPRT();
        WriteZeroPageByte(kMON_CH, 5);
        PrintListLine(currentPtr);

        currentPtr = AdvanceToNextLine(currentPtr);
        WriteZeroPageWord(kLOWTR, currentPtr.address);
    }

    CRDO();
}

void HandleNumberedLine() {
    LINGET();
    PARSE_INPUT_LINE();

    if (FNDLIN()) {
        DeleteExistingLine();
    }

    InsertNewLine();
    FIX_LINKS();
}

bool NEW() {
    return NEW_impl();
}

bool SETPTRS() {
    return SETPTRS_impl();
}

bool CLEAR() {
    return CLEAR_impl();
}

void SCRTCH() {
    SCRTCH_impl();
}

void CLEARC() {
    CLEARC_impl();
}

void STXTPT() {
    STXTPT_impl();
}

void ERROR(std::uint8_t error_code_offset) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ERROR (inclusive) .. PRINT_ERROR_LINNUM (exclusive)
    // Name normalization: none (assembler label ERROR kept verbatim).

    if (IsOnErr()) {
        HANDLERR();
        return;
    }

    CRDO();
    OUTQUES();
    STROUT(ERROR_MESSAGES(error_code_offset));
    STKINI();
    PRINT_ERROR_LINNUM();
}

void PRINT_ERROR_LINNUM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PRINT_ERROR_LINNUM (inclusive) .. RESTART (exclusive)
    // Name normalization: none (assembler label PRINT_ERROR_LINNUM kept verbatim).

    STROUT(QT_ERROR(QT_ERROR_INDEX));

    if (IsDirectMode()) {
        RESTART();
        return;
    }

    INPRT();
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

std::uint16_t ToWord(LineAddress address) {
    return ApplesoftVariables::makeWord(address.lo, address.hi);
}

LineAddress FromWord(std::uint16_t value) {
    return LineAddress{ApplesoftVariables::lowByte(value), ApplesoftVariables::highByte(value)};
}

ProgramPointer GetTextTablePointer() {
    constexpr std::uint8_t kTXTTAB = 0x67;
    return ProgramPointer{ReadZeroPageWord(kTXTTAB)};
}

bool IsEndOfProgram(LineAddress current) {
    // The original FIX_LINKS loop terminates by jumping to RESTART after processing
    // the final program line. A zero address is the safest sentinel for now.
    return current.lo == 0 && current.hi == 0;
}

bool IsEndOfProgram(ProgramPointer currentPtr) {
    return currentPtr.address == 0;
}

ProgramPointer AdvanceToNextLine(ProgramPointer currentPtr) {
    // The original FIX_LINKS routine scans from the current line until it finds the
    // end-of-line marker, then computes the address of the next line.
    std::uint8_t offset = 4;
    while (currentPtr.read(offset) != 0) {
        ++offset;
    }

    return currentPtr.advanced(static_cast<std::uint16_t>(offset) + 1u);
}

void WriteForwardPointer(LineAddress current, LineAddress next) {
    // TODO(asm-port): write the low/high bytes of 'next' into the current line's forward-pointer header.
    (void)current;
    (void)next;
}

void WriteForwardPointer(ProgramPointer currentPtr, ProgramPointer nextPtr) {
    WriteForwardPointer(FromWord(currentPtr.address), FromWord(nextPtr.address));
}

void FIX_LINKS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIX_LINKS (inclusive) .. INLIN (exclusive)
    // Name normalization: none (assembler label FIX_LINKS kept verbatim).

    SETPTRS();

    ProgramPointer currentPtr = GetTextTablePointer();
    while (true) {
        if (IsEndOfProgram(currentPtr)) {
            RESTART();
            return;
        }

        const ProgramPointer nextPtr = AdvanceToNextLine(currentPtr);
        WriteForwardPointer(currentPtr, nextPtr);
        currentPtr = nextPtr;
    }
}

void STROUT(std::string_view text) {
    // TODO(asm-port): print the given string to the Applesoft console.
    // This overload is a high-level convenience bridge used by the error-printing
    // code.  It is distinct from STROUT(uint8_t a, uint8_t y) in asm_port_print.
    (void)text;
}

void INPRT() {
    // TODO(asm-port): print the current line number when running a program.
}

void STKINI() {
    constexpr std::uint8_t kTEMPPT = 0x52;
    constexpr std::uint8_t kTEMPST = 0x55;
    constexpr std::uint8_t kOLDTEXT_plus_1 = 0x7a;
    constexpr std::uint8_t kSUBFLG = 0x14;

    WriteZeroPageByte(kTEMPPT, kTEMPST);
    SetStackPointer(0xf8);
    WriteZeroPageByte(kOLDTEXT_plus_1, 0);
    WriteZeroPageByte(kSUBFLG, 0);
}

// void OUTDO() {
//     // TODO(asm-port): write the current output character from the Applesoft line
//     // printer to the console.
// }

void LINPRT() {
    // TODO(asm-port): print the current Applesoft line number during LIST.
}

std::uint8_t GETCHR() {
    // TODO(asm-port): fetch the next keyword character from the Applesoft token
    // table during LIST output.
    return 0;
}

bool ISCNTC() {
    constexpr std::uint8_t kKEYBOARD = 0xc0; // TODO(asm-port): actual KEYBOARD zero-page address.
    constexpr std::uint8_t kCTRL_C_CODE = 0x83;

    if (ReadZeroPageByte(kKEYBOARD) != kCTRL_C_CODE) {
        return false;
    }

    INCHR();
    CONTROL_C_TYPED();
    return true;
}

std::uint8_t ReadStackPointer() {
    // TODO(asm-port): return the current 6502 stack pointer value.
    return 0;
}

void PopReturnAddress() {
    // TODO(asm-port): discard the most recently pushed return address bytes.
}

void PushByteToStack(std::uint8_t /*value*/) {
    // TODO(asm-port): push a byte onto the Applesoft 6502 stack.
}

void PushWordToStack(std::uint16_t value) {
    // Push word big-endian (hi first) per 6502 stack convention.
    PushByteToStack(ApplesoftVariables::highByte(value));
    PushByteToStack(ApplesoftVariables::lowByte(value));
}

std::uint16_t PopWordFromStack() {
    // Pop word in reverse order: lo byte popped first, then hi.
    const std::uint8_t lo = PopByteFromStack();
    const std::uint8_t hi = PopByteFromStack();
    return ApplesoftVariables::makeWord(lo, hi);
}

void PushTextPointerAddress() {
    // TODO(asm-port): push the current TXTPTR address on the Applesoft stack.
}

void PushCurrentLineNumber() {
    // TODO(asm-port): push CURLIN and CURLIN+1 onto the Applesoft stack.
}

void PushTokenTo(std::uint8_t /*token*/) {
    // TODO(asm-port): push a statement token value onto the Applesoft stack.
}

void ApplyFacSign() {
    // TODO(asm-port): update FAC+1 with the signed value produced by FRMNUM.
}

void SetBranchTargetToSTEP() {
    // TODO(asm-port): set the indirect jump target used by FRM_STACK_3 to STEP.
}

void LOAD_FAC_FROM_YA() {
    // TODO(asm-port): load the constant 1.0 into FAC from the Y,A pointer.
}

void SYNCHR(std::uint8_t /*expected*/) {
    // TODO(asm-port): require a specific statement token from the parsed input.
}

void CHKNUM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKNUM (inclusive) .. CHKSTR (exclusive)
    // Name normalization: none (assembler label CHKNUM kept verbatim).

    constexpr std::uint8_t kVALTYP = 0x11;
    const bool facIsString = (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u;
    if (facIsString) {
        ERROR(ERR_BADTYPE);
    }
}

void CHKSTR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKSTR (inclusive) .. CHKVAL (exclusive)
    // Name normalization: none (assembler label CHKSTR kept verbatim).

    constexpr std::uint8_t kVALTYP = 0x11;
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

void SIGN() {
    // TODO(asm-port): normalize the sign of the current FAC value.
}

void FRM_STACK_2() {
    // TODO(asm-port): prepare FOR frame storage on the Applesoft stack.
}

void FRM_STACK_3() {
    // TODO(asm-port): consume the current frame data and continue at STEP.
}

std::uint8_t ScanAheadOffset(std::uint8_t terminator) {
    constexpr std::uint8_t kTXTPTR = 0xb8;
    constexpr std::uint8_t kCHARAC = 0x0d;
    constexpr std::uint8_t kENDCHR = 0x0e;

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

void ADDON(std::uint8_t offset) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ADDON (inclusive) .. DATAN (exclusive)
    // Name normalization: none (assembler label ADDON kept verbatim).

    constexpr std::uint8_t kTXTPTR = 0xb8;

    const std::uint16_t advanced = static_cast<std::uint16_t>(ReadZeroPageWord(kTXTPTR) + offset);
    WriteZeroPageWord(kTXTPTR, advanced);
}

std::uint8_t DATAN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DATAN (inclusive) .. REMN (exclusive)
    // Name normalization: none (assembler label DATAN kept verbatim).

    return ScanAheadOffset(static_cast<std::uint8_t>(':'));
}

void GOEND() {
    // TODO(asm-port): handle end-of-program flow for NEXT statements.
}

bool IsEndOfLineAtTextPointer() {
    // TODO(asm-port): return true when TXTPTR is at the end of the current line.
    return false;
}

bool IsEndOfProgramAtTextPointer() {
    // TODO(asm-port): return true when there is no next line after TXTPTR.
    return false;
}

std::uint16_t ReadLineNumberFromTextPointer() {
    // TODO(asm-port): read the line number stored at the current TXTPTR.
    return 0;
}

void AdvanceTextPointerToNextLine() {
    // TODO(asm-port): advance TXTPTR to the start of the next BASIC line.
}

bool IsRunningMode() {
    // TODO(asm-port): determine whether the interpreter is currently running.
    return false;
}

bool IsTraceEnabled() {
    // TODO(asm-port): inspect the TRCFLG flag from zero page.
    return false;
}

std::uint8_t CurrentStatementChar() {
    // TODO(asm-port): return the current statement character at the parser cursor.
    return 0;
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
void RESTORE() {
    constexpr std::uint8_t kTXTTAB = 0x67;
    const std::uint16_t textTable = ReadZeroPageWord(kTXTTAB);
    const std::uint16_t dataPointer = static_cast<std::uint16_t>(textTable - 1u);
    SETDA(dataPointer);
}

void SETDA(std::uint16_t dataPointer) {
    constexpr std::uint8_t kDATPTR = 0x7d;
    WriteZeroPageWord(kDATPTR, dataPointer);
}

void CONTROL_C_TYPED() {
    constexpr std::uint8_t kERRFLG = 0xd8;
    const std::uint8_t errFlags = ReadZeroPageByte(kERRFLG);

    if ((errFlags & 0x80u) == 0u) {
        HANDLERR();
        return;
    }

    // Control-C attempts to fall through to the STOP/END handler with an
    // implicit "break" condition.
    STOP_impl(true);
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

    constexpr std::uint8_t kTXTPTR = 0xb8;
    constexpr std::uint8_t kCURLIN = 0x75;
    constexpr std::uint8_t kOLDTEXT = 0x79;
    constexpr std::uint8_t kOLDLIN = 0x77;

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

void PRINT_ERROR_LINNUM(std::string_view prefix) {
    STROUT(prefix);

    if (IsDirectMode()) {
        RESTART();
        return;
    }

    INPRT();
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

    constexpr std::uint8_t kOLDTEXT = 0x79;
    constexpr std::uint8_t kOLDTEXT_plus_1 = 0x7a;
    constexpr std::uint8_t kOLDLIN = 0x77;
    constexpr std::uint8_t kTXTPTR = 0xb8;
    constexpr std::uint8_t kCURLIN = 0x75;

    if (ReadZeroPageByte(kOLDTEXT_plus_1) == 0) {
        ERROR(ERR_CANTCONT);
        return;
    }

    WriteZeroPageWord(kTXTPTR, ReadZeroPageWord(kOLDTEXT));
    WriteZeroPageWord(kCURLIN, ReadZeroPageWord(kOLDLIN));
}

void SAVE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SAVE (inclusive) .. LOAD (exclusive)
    // Name normalization: none (assembler label SAVE kept verbatim).

    constexpr std::uint8_t kPRGEND = 0xaf;
    constexpr std::uint8_t kTXTTAB = 0x67;
    constexpr std::uint8_t kLINNUM = 0x50;

    const std::uint16_t programEnd = ReadZeroPageWord(kPRGEND);
    const std::uint16_t textTable = ReadZeroPageWord(kTXTTAB);
    const std::uint16_t programLength = static_cast<std::uint16_t>(programEnd - textTable);
    WriteZeroPageWord(kLINNUM, programLength);

    VARTIO();
    MON_WRITE();
    PROGIO();
    MON_WRITE();
}

void LOAD() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LOAD (inclusive) .. VARTIO (exclusive)
    // Name normalization: none (assembler label LOAD kept verbatim).

    constexpr std::uint8_t kLINNUM = 0x50;
    constexpr std::uint8_t kTXTTAB = 0x67;
    constexpr std::uint8_t kVARTAB = 0x69;
    constexpr std::uint8_t kTEMPPT = 0x52;
    constexpr std::uint8_t kLOCK = 0xd6;

    VARTIO();
    MON_READ();

    const std::uint16_t endAddress =
        static_cast<std::uint16_t>(ReadZeroPageWord(kTXTTAB) + ReadZeroPageWord(kLINNUM));
    WriteZeroPageWord(kVARTAB, endAddress);

    WriteZeroPageByte(kLOCK, ReadZeroPageByte(kTEMPPT));

    PROGIO();
    MON_READ();

    if ((ReadZeroPageByte(kLOCK) & 0x80u) != 0u) {
        (void)SETPTRS();
        return;
    }

    FIX_LINKS();
}

void VARTIO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: VARTIO (inclusive) .. PROGIO (exclusive)
    // Name normalization: none (assembler label VARTIO kept verbatim).

    constexpr std::uint8_t kLINNUM = 0x50;
    constexpr std::uint8_t kTEMPPT = 0x52;
    constexpr std::uint8_t kLOCK = 0xd6;
    constexpr std::uint8_t kMON_A1L = 0x3c;
    constexpr std::uint8_t kMON_A1H = 0x3d;
    constexpr std::uint8_t kMON_A2L = 0x3e;
    constexpr std::uint8_t kMON_A2H = 0x3f;

    WriteZeroPageByte(kMON_A1L, kLINNUM);
    WriteZeroPageByte(kMON_A1H, 0x00);
    WriteZeroPageByte(kMON_A2L, kTEMPPT);
    WriteZeroPageByte(kMON_A2H, 0x00);
    WriteZeroPageByte(kLOCK, 0x00);
}

void PROGIO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PROGIO (inclusive) .. RUN (exclusive)
    // Name normalization: none (assembler label PROGIO kept verbatim).

    constexpr std::uint8_t kTXTTAB = 0x67;
    constexpr std::uint8_t kVARTAB = 0x69;
    constexpr std::uint8_t kMON_A1L = 0x3c;
    constexpr std::uint8_t kMON_A1H = 0x3d;
    constexpr std::uint8_t kMON_A2L = 0x3e;
    constexpr std::uint8_t kMON_A2H = 0x3f;

    WriteZeroPageWord(kMON_A1L, ReadZeroPageWord(kTXTTAB));
    WriteZeroPageWord(kMON_A2L, ReadZeroPageWord(kVARTAB));

    // Keep symbolic names visible for monitor register parity.
    (void)kMON_A1H;
    (void)kMON_A2H;
}

void RUN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RUN (inclusive) .. GOSUB (exclusive)
    // Name normalization: none (assembler label RUN kept verbatim).
    //
    // Executes the "RUN" command:
    // - Modifies CURLIN+1 to mark running mode (converts $FF direct mode to $FE)
    // - If no line number specified: starts execution at beginning of program (SETPTRS)
    // - If line number specified: clears variables (CLEARC) then searches for and jumps to that line

    constexpr std::uint8_t kCURLIN_hi = 0x76;
    
    // Decrement CURLIN+1 to mark as running (6502: dec CURLIN+1)
    std::uint8_t curlinHi = ReadZeroPageByte(kCURLIN_hi);
    WriteZeroPageByte(kCURLIN_hi, static_cast<std::uint8_t>(curlinHi - 1));
    
    // Check if there's a line number argument following RUN
    // (CHRGET sets Z flag if no more input; CHRGOT returns current char)
    const std::uint8_t currentChar = CHRGOT();
    if (currentChar == 0) {
        // No line number: start at beginning of program
        SETPTRS();
        return;
    }
    
    // Line number specified: clear variables then go to that line
    CLEARC();
    
    GO_TO_LINE();
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

    constexpr std::uint8_t kTXTPTR = 0xb8;
    constexpr std::uint8_t kCURLIN = 0x75;
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

    // Fall-through in ROM from GOSUB to GO_TO_LINE.
    GO_TO_LINE();
}

void GO_TO_LINE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GO_TO_LINE (inclusive) .. GOTO (exclusive)
    // Name normalization: none (assembler label GO_TO_LINE kept verbatim).

    (void)CHRGOT();
    GOTO();
    NEWSTT();
}

void GOTO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GOTO (inclusive) .. POP (exclusive)
    // Name normalization: none (assembler label GOTO kept verbatim).

    constexpr std::uint8_t kCURLIN = 0x75;
    constexpr std::uint8_t kLINNUM = 0x50;
    constexpr std::uint8_t kTXTPTR = 0xb8;
    constexpr std::uint8_t kTXTTAB = 0x67;
    constexpr std::uint8_t kLOWTR = 0x9b;

    LINGET();
    const std::uint8_t remnOffset = REMN();

    const std::uint8_t currentPage = ReadZeroPageByte(static_cast<std::uint8_t>(kCURLIN + 1));
    const std::uint8_t targetPage = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1));

    std::uint16_t start = 0;
    if (currentPage >= targetPage) {
        start = ReadZeroPageWord(kTXTTAB);
    } else {
        start = static_cast<std::uint16_t>(ReadZeroPageWord(kTXTPTR) + remnOffset + 1u);
    }

    if (!FL1(start)) {
        ERROR(ERR_UNDEFSTAT);
        return;
    }

    const std::uint16_t destination = static_cast<std::uint16_t>(ReadZeroPageWord(kLOWTR) - 1u);
    WriteZeroPageWord(kTXTPTR, destination);
}

void POP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: POP (inclusive) .. RETURN (exclusive)
    // Name normalization: none (assembler label POP kept verbatim).

    constexpr std::uint8_t kFORPNT = 0x85;
    constexpr std::uint8_t kTOKEN_GOSUB = 0xb0;

    if (!IsStatementEndOfParsedInput()) {
        RTS_5();
        return;
    }

    // Preserve original ROM bug: writes $FF to FORPNT low byte, not FORPNT+1.
    WriteZeroPageByte(kFORPNT, 0xffu);

    GTFORPNTState gtforpntState{};
    const auto gtforpntResult = GTFORPNT(ReadStackPointer(), gtforpntState);
    SetStackPointer(gtforpntResult.x);

    if (PeekTopControlTokenAfterGTFORPNT() == kTOKEN_GOSUB) {
        // Fall-through in ROM from POP to RETURN when top frame is GOSUB.
        RETURN();
        return;
    }

    ERROR(ERR_NOGOSUB);
}

void RETURN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RETURN (inclusive) .. DATA (exclusive)
    // Name normalization: none (assembler label RETURN kept verbatim).

    constexpr std::uint8_t kCURLIN = 0x75;
    constexpr std::uint8_t kTXTPTR = 0xb8;

    (void)PopByteFromStack(); // discard GOSUB token
    const std::uint16_t currentLine = PopWordFromStack();

    if (ReturnWasFromPOPContext()) {
        PULL3();
        return;
    }

    const std::uint16_t textPointer = PopWordFromStack();

    WriteZeroPageWord(kCURLIN, currentLine);
    WriteZeroPageWord(kTXTPTR, textPointer);
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

bool FL1(std::uint16_t startAddress) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FL1 (inclusive) .. NEW (exclusive)
    // Name normalization: none (assembler label FL1 kept verbatim).

    constexpr std::uint8_t kLOWTR = 0x9b;
    constexpr std::uint8_t kLINNUM = 0x50;

    const std::uint8_t targetLo = ReadZeroPageByte(kLINNUM);
    const std::uint8_t targetHi = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1));

    std::uint16_t current = startAddress;

    while (true) {
        WriteZeroPageWord(kLOWTR, current);
        const ProgramPointer currentPtr{current};

        const std::uint8_t nextHi = currentPtr.read(1u);
        if (nextHi == 0) {
            return false;
        }

        const std::uint8_t lineHi = currentPtr.read(3u);
        if (targetHi < lineHi) {
            return false;
        }

        if (targetHi == lineHi) {
            const std::uint8_t lineLo = currentPtr.read(2u);
            if (targetLo < lineLo) {
                return false;
            }
            if (targetLo == lineLo) {
                return true;
            }
        }

        const std::uint8_t nextLo = currentPtr.read();
        current = ApplesoftVariables::makeWord(nextLo, nextHi);
    }
}

bool FL1(std::uint8_t startLo, std::uint8_t startHi) {
    return FL1(ApplesoftVariables::makeWord(startLo, startHi));
}

std::uint8_t PopByteFromStack() {
    // TODO(asm-port): pop and return one byte from the 6502 runtime stack.
    return 0;
}

bool ReturnWasFromPOPContext() {
    // TODO(asm-port): model CPY #<(TOKEN_POP*2) context check from RETURN.
    return false;
}

std::uint8_t PeekTopControlTokenAfterGTFORPNT() {
    // TODO(asm-port): recover A register/token result from GTFORPNT scan.
    return 0;
}

void DATA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DATA (inclusive) .. ADDON (exclusive)
    // Name normalization: none (assembler label DATA kept verbatim).

    const std::uint8_t offset = DATAN();
    ADDON(offset);
}

void IF() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: IF (inclusive) .. REM (exclusive)
    // Name normalization: none (assembler label IF kept verbatim).

    constexpr std::uint8_t kTOKEN_GOTO = 0xabu;
    constexpr std::uint8_t kTOKEN_THEN = 0xc4u;
    constexpr std::uint8_t kFAC = 0x9d;

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
    constexpr std::uint8_t kFAC_PLUS_4 = 0xa1;

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

void MON_WRITE() {
    // TODO(asm-port): port monitor tape write handler used by SAVE.
}

void MON_READ() {
    // TODO(asm-port): port monitor tape read handler used by LOAD.
}

void SYNERR() {
    // TODO(asm-port): handle a syntax error from the statement parser.
}

void PushForPntFrame() {
    constexpr std::uint8_t kFORPNT = 0x85;
    PushByteToStack(ReadZeroPageByte(static_cast<std::uint8_t>(kFORPNT + 1)));
    PushByteToStack(ReadZeroPageByte(kFORPNT));
    PushTokenTo(TOKEN_FOR);
}

void LET() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LET (inclusive) .. LET2 (exclusive)
    // Name normalization: none (assembler label LET kept verbatim).

    constexpr std::uint8_t kFORPNT = 0x85;
    constexpr std::uint8_t kTOKEN_EQUAL = 0xd0;
    constexpr std::uint8_t kVALTYP = 0x11;

    const std::uint16_t variablePtr = PTRGET();
    WriteZeroPageWord(kFORPNT, variablePtr);

    SYNCHR(kTOKEN_EQUAL);

    const std::uint8_t savedValTyp = ReadZeroPageByte(kVALTYP);
    const std::uint8_t savedValTypPlus1 = ReadZeroPageByte(static_cast<std::uint8_t>(kVALTYP + 1));

    FRMEVL();

    if (CHKVAL(savedValTyp)) {
        // LET_STRING branch falls through to PUTSTR in ROM.
        PUTSTR();
        return;
    }

    // Explicitly model LET -> LET2 fall-through.
    LET2(savedValTypPlus1);
}

void LET2(std::uint8_t savedValTypPlus1) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LET2 (inclusive) .. PUTSTR (exclusive)
    // Name normalization: none (assembler label LET2 kept verbatim).

    constexpr std::uint8_t kFORPNT = 0x85;
    constexpr std::uint8_t kFAC_PLUS_3 = 0xa0;
    constexpr std::uint8_t kFAC_PLUS_4 = 0xa1;

    // Positive means real variable; ROM jumps directly to SETFOR.
    if ((savedValTypPlus1 & 0x80u) == 0u) {
        SETFOR();
        return;
    }

    ROUND_FAC();
    AYINT();

    const std::uint16_t forPtr = ReadZeroPageWord(kFORPNT);
    auto forPtrByte = variables().pointer(forPtr);
    forPtrByte.write(ReadZeroPageByte(kFAC_PLUS_3));
    forPtrByte.write(ReadZeroPageByte(kFAC_PLUS_4), 1u);
}

void PUTSTR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PUTSTR (inclusive) .. PR_STRING (exclusive)
    // Name normalization: none (assembler label PUTSTR kept verbatim).

    constexpr std::uint8_t kFAC_PLUS_3 = 0xa0;
    constexpr std::uint8_t kFAC_PLUS_4 = 0xa1;
    constexpr std::uint8_t kDSCPTR = 0x8c;
    constexpr std::uint8_t kFORPNT = 0x85;
    constexpr std::uint8_t kFRETOP = 0x6f;
    constexpr std::uint8_t kVARTAB = 0x69;
    constexpr std::uint8_t kSTRNG1 = 0xab;

    const std::uint16_t facDescriptor = ReadZeroPageWord(kFAC_PLUS_3);
    const auto facDescriptorPtr = variables_const().pointer(facDescriptor);
    auto readDescriptorByte = [&](std::uint8_t offset) {
        return facDescriptorPtr.read(offset);
    };

    std::uint16_t descriptorPointer = ReadZeroPageWord(kFAC_PLUS_3);

    const std::uint8_t descDataHigh = readDescriptorByte(2);
    const std::uint8_t fretopHigh = ReadZeroPageByte(static_cast<std::uint8_t>(kFRETOP + 1));

    bool useExistingDescriptor = false;
    bool descriptorIsVariable = false;

    if (descDataHigh < fretopHigh) {
        useExistingDescriptor = true;
    } else if (descDataHigh == fretopHigh) {
        const std::uint8_t descDataLow = readDescriptorByte(1);
        if (descDataLow < ReadZeroPageByte(kFRETOP)) {
            useExistingDescriptor = true;
        }
    }

    if (!useExistingDescriptor) {
        if (descriptorPointer >= ReadZeroPageWord(kVARTAB)) {
            descriptorIsVariable = true;
        }
    }

    if (descriptorIsVariable) {
        STRINI(readDescriptorByte(0));
        WriteZeroPageWord(kSTRNG1, ReadZeroPageWord(kDSCPTR));
        MOVINS();
        descriptorPointer = kFAC_PLUS_3;
    }

    WriteZeroPageWord(kDSCPTR, descriptorPointer);

    FRETMS();

    const std::uint16_t source = ReadZeroPageWord(kDSCPTR);
    const std::uint16_t dest = ReadZeroPageWord(kFORPNT);
    const auto sourcePtr = variables_const().pointer(source);
    auto destPtr = variables().pointer(dest);
    for (std::uint8_t i = 0; i < 3; ++i) {
        destPtr.write(sourcePtr.read(i), i);
    }
}

void FRMEVL() {
    // TODO(asm-port): evaluate an expression into FAC.
}

std::uint8_t GETBYT() {
    // TODO(asm-port): parse byte argument and leave selector in FAC+4.
    return 0;
}

std::uint16_t PTRGET() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET (inclusive) .. PTRGET2 (exclusive)
    // Name normalization: none (assembler label PTRGET kept verbatim).

    CHRGOT();
    WriteZeroPageByte(0x10u, 0u); // DIMFLG
    PTRGET3();
    return ReadZeroPageWord(0x83u); // VARPNT
}

bool CHKVAL(std::uint8_t savedValTyp) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKVAL (inclusive) .. FRMEVL (exclusive)
    // Name normalization: none (assembler label CHKVAL kept verbatim).

    constexpr std::uint8_t kVALTYP = 0x11;

    const bool facIsString = (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u;
    const bool expectedString = (savedValTyp & 0x80u) != 0u;

    if (facIsString != expectedString) {
        ERROR(ERR_BADTYPE);
        return false;
    }

    return facIsString;
}

void SCREEN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SCREEN (inclusive) .. UNARY (exclusive)
    // Name normalization: none (assembler label SCREEN kept verbatim).

    constexpr std::uint8_t kFIRST = 0xf0;

    CHRGET();
    PLOTFNS();

    // PLOTFNS returns row in X and column in FIRST in ROM.
    const std::uint8_t row = ReadZeroPageByte(kFIRST);
    const std::uint8_t column = ReadZeroPageByte(kFIRST);
    const std::uint8_t color = MON_SCRN(row, column);

    SNGFLT(color);
    SYNCHR(static_cast<std::uint8_t>(')'));
}

void UNARY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: UNARY (inclusive) .. OR (exclusive)
    // Name normalization: none (assembler label UNARY kept verbatim).

    constexpr std::uint8_t kTOKEN_SCRN = 0xd7u;

    if (CHRGOT() == kTOKEN_SCRN) {
        // ROM branches back to SCREEN for SCRN(.
        SCREEN();
        return;
    }

    CHRGET();

    // TODO(asm-port): complete unary-function dispatch through UNFNC/JMPADRS.
    FRMEVL();
    CHKNUM();
}

void OR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: OR (inclusive) .. ANDOP (exclusive)
    // Name normalization: none (assembler label OR kept verbatim).

    constexpr std::uint8_t kARG = 0xa5;
    constexpr std::uint8_t kFAC = 0x9d;

    if ((ReadZeroPageByte(kARG) | ReadZeroPageByte(kFAC)) != 0u) {
        TRUE();
        return;
    }

    // Fall-through in ROM from OR to ANDOP.
    ANDOP();
}

void ANDOP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ANDOP (inclusive) .. FALSE (exclusive)
    // Name normalization: none (assembler label ANDOP kept verbatim).

    constexpr std::uint8_t kARG = 0xa5;
    constexpr std::uint8_t kFAC = 0x9d;

    if (ReadZeroPageByte(kARG) == 0u || ReadZeroPageByte(kFAC) == 0u) {
        FALSE();
        return;
    }

    // Fall-through in ROM from ANDOP to TRUE.
    TRUE();
}

void FALSE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FALSE (inclusive) .. TRUE (exclusive)
    // Name normalization: none (assembler label FALSE kept verbatim).

    SNGFLT(0u);
}

void TRUE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: TRUE (inclusive) .. RELOPS (exclusive)
    // Name normalization: none (assembler label TRUE kept verbatim).

    SNGFLT(1u);
}

void RELOPS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RELOPS (inclusive) .. STRCMP (exclusive)
    // Name normalization: none (assembler label RELOPS kept verbatim).

    constexpr std::uint8_t kCPRTYP = 0x89;
    constexpr std::uint16_t kARG = 0x00a5u;

    const std::uint8_t compareTypeFlags = ReadZeroPageByte(kCPRTYP);
    if (CHKVAL(compareTypeFlags)) {
        // Carry set in ROM indicates string compare path.
        STRCMP();
        return;
    }

    gNumericCompareResult = FCOMP(kARG);
    gNumericCompareCarry = gNumericCompareResult >= 0;
    NUMCMP();
}

void STRCMP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: STRCMP (inclusive) .. NUMCMP (exclusive)
    // Name normalization: none (assembler label STRCMP kept verbatim).

    constexpr std::uint8_t kVALTYP = 0x11;
    constexpr std::uint8_t kCPRTYP = 0x89;

    WriteZeroPageByte(kVALTYP, 0u);
    WriteZeroPageByte(kCPRTYP, static_cast<std::uint8_t>(ReadZeroPageByte(kCPRTYP) - 1u));

    FREFAC();
    FRETMP();

    gNumericCompareResult = CompareArgAndFacStrings();
    gNumericCompareCarry = gNumericCompareResult >= 0;
    NUMCMP();
}

void NUMCMP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NUMCMP (inclusive) .. CMPDONE (exclusive)
    // Name normalization: none (assembler label NUMCMP kept verbatim).

    // ROM reaches CMPDONE with C set only when compare result was negative.
    gNumericCompareCarry = (gNumericCompareResult < 0);
    CMPDONE();
}

void CMPDONE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CMPDONE (inclusive) .. PDL (exclusive)
    // Name normalization: none (assembler label CMPDONE kept verbatim).

    constexpr std::uint8_t kCPRMASK = 0x16;

    std::int16_t x = static_cast<std::int16_t>(gNumericCompareResult) + 1;
    if (x < 0) {
        x = 0;
    }

    std::uint8_t a = static_cast<std::uint8_t>(x & 0xff);
    a = static_cast<std::uint8_t>((a << 1) | (gNumericCompareCarry ? 1u : 0u));
    a = static_cast<std::uint8_t>(a & ReadZeroPageByte(kCPRMASK));

    gFloatInput = (a == 0u) ? 0u : 1u;
    SNGFLT(gFloatInput);
    FLOAT();
}

void PDL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PDL (inclusive) .. NXDIM (exclusive)
    // Name normalization: none (assembler label PDL kept verbatim).

    CONINT();
    SNGFLT(MON_PREAD());
}

void NXDIM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NXDIM (inclusive) .. DIM (exclusive)
    // Name normalization: none (assembler label NXDIM kept verbatim).

    CHKCOM();
    DIM();
}

void DIM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DIM (inclusive) .. PTRGET (exclusive)
    // Name normalization: none (assembler label DIM kept verbatim).

    WriteZeroPageByte(0x10u, 1u); // DIMFLG non-zero when called from DIM.
    PTRGET2();

    if (CHRGOT() != 0u) {
        NXDIM();
    }
}

void PTRGET2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET2 (inclusive) .. PTRGET3 (exclusive)
    // Name normalization: none (assembler label PTRGET2 kept verbatim).

    PTRGET3();
}

void PTRGET3() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET3 (inclusive) .. BADNAM (exclusive)
    // Name normalization: none (assembler label PTRGET3 kept verbatim).

    WriteZeroPageByte(0x81u, CHRGOT()); // VARNAM low byte
    CHRGOT();
    if (!ISLETC()) {
        BADNAM();
        return;
    }

    NAMOK();
}

void BADNAM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BADNAM (inclusive) .. NAMOK (exclusive)
    // Name normalization: none (assembler label BADNAM kept verbatim).

    SYNERR();
}

void NAMOK() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NAMOK (inclusive) .. BASIC (exclusive)
    // Name normalization: none (assembler label NAMOK kept verbatim).

    WriteZeroPageByte(0x11u, 0u); // VALTYP
    WriteZeroPageByte(0x12u, 0u); // VALTYP+1
    PTRGET4();
}

void BASIC() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BASIC (inclusive) .. BASIC2 (exclusive)
    // Name normalization: none (assembler label BASIC kept verbatim).

    COLD_START();
}

void BASIC2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BASIC2 (inclusive) .. PTRGET4 (exclusive)
    // Name normalization: none (assembler label BASIC2 kept verbatim).

    RESTART();
}

void PTRGET4() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET4 (inclusive) .. ISLETC (exclusive)
    // Name normalization: none (assembler label PTRGET4 kept verbatim).

    std::uint8_t current = CHRGET();
    std::uint8_t secondChar = 0u;

    if ((current >= '0') && (current <= '9')) {
        secondChar = current;
        do {
            current = CHRGET();
        } while ((current >= '0') && (current <= '9'));
    } else {
        if (ISLETC()) {
            secondChar = current;
            do {
                current = CHRGET();
            } while (((current >= '0') && (current <= '9')) || ISLETC());
        }
    }

    if (current == static_cast<std::uint8_t>('$')) {
        WriteZeroPageByte(0x11u, 0xffu); // VALTYP string
        current = CHRGET();
    } else if (current == static_cast<std::uint8_t>('%')) {
        if ((ReadZeroPageByte(0x14u) & 0x80u) != 0u) {
            BADNAM();
            return;
        }

        WriteZeroPageByte(0x12u, 0x80u); // integer mode
        WriteZeroPageByte(0x81u, static_cast<std::uint8_t>(ReadZeroPageByte(0x81u) | 0x80u));
        secondChar = static_cast<std::uint8_t>(secondChar | 0x80u);
        current = CHRGET();
    }

    WriteZeroPageByte(0x82u, secondChar); // VARNAM+1

    const std::uint8_t subflg = ReadZeroPageByte(0x14u);
    if (subflg == 0u && current == static_cast<std::uint8_t>('(')) {
        ARRAY();
        return;
    }

    WriteZeroPageByte(0x14u, 0u); // clear SUBFLG
    NAME_NOT_FOUND();
}

bool ISLETC() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ISLETC (inclusive) .. NAME_NOT_FOUND (exclusive)
    // Name normalization: none (assembler label ISLETC kept verbatim).

    const std::uint8_t ch = CHRGOT();
    return ch >= static_cast<std::uint8_t>('A') && ch <= static_cast<std::uint8_t>('Z');
}

void NAME_NOT_FOUND() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NAME_NOT_FOUND (inclusive) .. C_ZERO (exclusive)
    // Name normalization: none (assembler label NAME_NOT_FOUND kept verbatim).

    // TODO(asm-port): preserve FRM_VARIABLE_CALL return-address special case.
    MAKE_NEW_VARIABLE();
}

void C_ZERO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: C_ZERO (inclusive) .. MAKE_NEW_VARIABLE (exclusive)
    // Name normalization: none (assembler label C_ZERO kept verbatim).

    WriteZeroPageByte(0x62u, kCZeroData[0]);
    WriteZeroPageByte(0x63u, kCZeroData[1]);
}

void MAKE_NEW_VARIABLE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKE_NEW_VARIABLE (inclusive) .. SET_VARPNT_AND_YA (exclusive)
    // Name normalization: none (assembler label MAKE_NEW_VARIABLE kept verbatim).

    const std::uint16_t arytab = ReadZeroPageWord(0x6bu);
    WriteZeroPageWord(0x9bu, arytab); // LOWTR <- ARYTAB

    // TODO(asm-port): port BLTU movement of array block.
    SET_VARPNT_AND_YA();
}

void SET_VARPNT_AND_YA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SET_VARPNT_AND_YA (inclusive) .. GETARY (exclusive)
    // Name normalization: none (assembler label SET_VARPNT_AND_YA kept verbatim).

    const std::uint16_t valueAddress = static_cast<std::uint16_t>(ReadZeroPageWord(0x9bu) + 2u);
    WriteZeroPageWord(0x83u, valueAddress); // VARPNT
}

void GETARY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GETARY (inclusive) .. GETARY2 (exclusive)
    // Name normalization: none (assembler label GETARY kept verbatim).

    GETARY2();
}

void GETARY2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GETARY2 (inclusive) .. NEG32768 (exclusive)
    // Name normalization: none (assembler label GETARY2 kept verbatim).

    const std::uint8_t numDim = ReadZeroPageByte(0x0fu);
    const std::uint16_t lowtr = ReadZeroPageWord(0x9bu);
    const std::uint16_t arypnt = static_cast<std::uint16_t>(lowtr + static_cast<std::uint16_t>(numDim * 2u) + 5u);
    WriteZeroPageWord(0x94u, arypnt);
}

void NEG32768() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEG32768 (inclusive) .. MAKINT (exclusive)
    // Name normalization: none (assembler label NEG32768 kept verbatim).

    WriteZeroPageByte(0x62u, kNEG32768Data[0]);
    WriteZeroPageByte(0x63u, kNEG32768Data[1]);
    WriteZeroPageByte(0x64u, kNEG32768Data[2]);
    WriteZeroPageByte(0x65u, kNEG32768Data[3]);
}

void MAKINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKINT (inclusive) .. MKINT (exclusive)
    // Name normalization: none (assembler label MAKINT kept verbatim).

    CHRGET();
    FRMNUM();
    MKINT();
}

void MKINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MKINT (inclusive) .. AYINT (exclusive)
    // Name normalization: none (assembler label MKINT kept verbatim).

    if ((ReadZeroPageByte(0xa2u) & 0x80u) != 0u) {
        MI1();
        return;
    }

    AYINT();
}

void ROUND_FAC() {
    // TODO(asm-port): port ROUND_FAC label.
}

void AYINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: AYINT (inclusive) .. MI1 (exclusive)
    // Name normalization: none (assembler label AYINT kept verbatim).

    if (ReadZeroPageByte(0x9du) < 0x90u) {
        MI2();
        return;
    }

    NEG32768();
    if (FCOMP(0x0062u) != 0) {
        MI1();
        return;
    }

    MI2();
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

void ARRAY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ARRAY (inclusive) .. SUBERR (exclusive)
    // Name normalization: none (assembler label ARRAY kept verbatim).

    if (ReadZeroPageByte(0x14u) != 0u) {
        USE_OLD_ARRAY();
        return;
    }

    // TODO(asm-port): complete subscript-list parsing and array-table scan.
    MAKE_NEW_ARRAY();
}

void SUBERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SUBERR (inclusive) .. IQERR (exclusive)
    // Name normalization: none (assembler label SUBERR kept verbatim).

    gJerErrorCode = ERR_BADSUBS;
    JER();
}

void IQERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: IQERR (inclusive) .. JER (exclusive)
    // Name normalization: none (assembler label IQERR kept verbatim).

    gJerErrorCode = ERR_ILLQTY;
    JER();
}

void JER() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: JER (inclusive) .. USE_OLD_ARRAY (exclusive)
    // Name normalization: none (assembler label JER kept verbatim).

    ERROR(gJerErrorCode);
}

void USE_OLD_ARRAY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: USE_OLD_ARRAY (inclusive) .. MAKE_NEW_ARRAY (exclusive)
    // Name normalization: none (assembler label USE_OLD_ARRAY kept verbatim).

    if (ReadZeroPageByte(0x10u) != 0u) {
        gJerErrorCode = ERR_REDIMD;
        JER();
        return;
    }

    if (ReadZeroPageByte(0x14u) == 0u) {
        GETARY();
        FIND_ARRAY_ELEMENT();
    }
}

void MAKE_NEW_ARRAY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKE_NEW_ARRAY (inclusive) .. FIND_ARRAY_ELEMENT (exclusive)
    // Name normalization: none (assembler label MAKE_NEW_ARRAY kept verbatim).

    if (ReadZeroPageByte(0x14u) != 0u) {
        ERROR(ERR_NODATA);
        return;
    }

    GETARY();

    // TODO(asm-port): complete dynamic allocation, descriptor population, and zeroing.
    if (ReadZeroPageByte(0x10u) == 0u) {
        FIND_ARRAY_ELEMENT();
    }
}

void FIND_ARRAY_ELEMENT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIND_ARRAY_ELEMENT (inclusive) .. FAE_1 (exclusive)
    // Name normalization: none (assembler label FIND_ARRAY_ELEMENT kept verbatim).

    WriteZeroPageByte(0x0fu, ReadZeroPageByte(0x0fu)); // TODO(asm-port): fetch #dims from descriptor pointer.
    WriteZeroPageWord(0xadu, 0u); // STRNG2 accumulator
    FAE_1();
}

void FAE_1() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FAE_1 (inclusive) .. GSE (exclusive)
    // Name normalization: none (assembler label FAE_1 kept verbatim).

    if (ReadZeroPageByte(0x0fu) == 0u) {
        return;
    }

    // TODO(asm-port): complete per-dimension bounds and offset accumulation.
    GSE();
}

void GSE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GSE (inclusive) .. GME (exclusive)
    // Name normalization: none (assembler label GSE kept verbatim).

    SUBERR();
}

void GME() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GME (inclusive) .. MULTIPLY_SUBSCRIPT (exclusive)
    // Name normalization: none (assembler label GME kept verbatim).

    (void)MEMERR();
}

void SETFOR() {
    // TODO(asm-port): port SETFOR label.
}

void STRINI(std::uint8_t length) {
    // TODO(asm-port): port STRINI label.
    (void)length;
}

void MOVINS() {
    // TODO(asm-port): port MOVINS label.
}

void FRETMS() {
    // TODO(asm-port): port FRETMS label.
}

void HANDLERR() {
    // TODO(asm-port): transfer control to the ON ERR handler.
}

bool IsOnErr() {
    // TODO(asm-port): inspect the Applesoft ERRFLG state.
    return false;
}

bool IsDirectMode() {
    // TODO(asm-port): return true when the interpreter is in direct mode.
    return false;
}

} // namespace applesoft::asm_port
