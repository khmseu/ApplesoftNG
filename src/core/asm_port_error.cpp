#include "core/asm_port_error.hpp"
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

void SetTextPointer(std::uint8_t lo, std::uint8_t hi) {
    variables().writeWord(0x00b8u, static_cast<std::uint16_t>((static_cast<std::uint16_t>(hi) << 8) | lo));
}

void ClearErrFlag() {
    variables().writeByte(0x00d8u, 0);
}

void MarkDirectMode() {
    variables().writeByte(0x0076u, 0xffu);
}

void LINGET() {
    // TODO(asm-port): consume the line number prefix from the input buffer.
}

std::uint8_t ReadZeroPageByte(std::uint8_t address) {
    return variables_const().readByte(address);
}

void WriteZeroPageByte(std::uint8_t address, std::uint8_t value) {
    variables().writeByte(address, value);
}

void WriteZeroPageWord(std::uint8_t address, std::uint16_t value) {
    WriteZeroPageByte(address, static_cast<std::uint8_t>(value & 0xffu));
    WriteZeroPageByte(static_cast<std::uint8_t>(address + 1), static_cast<std::uint8_t>(value >> 8));
}

std::uint16_t ReadZeroPageWord(std::uint8_t address) {
    const std::uint8_t low = ReadZeroPageByte(address);
    const std::uint8_t high = ReadZeroPageByte(static_cast<std::uint8_t>(address + 1));
    return static_cast<std::uint16_t>(high) << 8 | low;
}

void RESTORE();
void SETDA(std::uint16_t dataPointer);
void CONTROL_C_TYPED();
void STOP();
void ENDX();
void CONT();
void SAVE();
void LOAD();
void VARTIO();
void PROGIO();
void STOP_impl(bool shouldPrintBreak);
void ENDX_impl(bool shouldPrintBreak);
void EXECUTE_STATEMENT_1();
std::uint8_t CurrentStatementChar();
void SYNERR();

void PRINT_ERROR_LINNUM(std::string_view prefix = QT_ERROR(QT_ERROR_INDEX));

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
void FRMNUM();
void SIGN();
void FRM_STACK_2();
void FRM_STACK_3();
void MON_WRITE();
void MON_READ();
void DATAN();
void GOEND();
bool IsEndOfLineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadLineNumberFromTextPointer();
void AdvanceTextPointerToNextLine();
bool IsRunningMode();
bool IsTraceEnabled();
void OUTSP();
void EXECUTE_STATEMENT();
void PushForPntFrame();

std::uint8_t CHRGOT();
bool ISCNTC();
void LINPRT();
void OUTDO();

void LET();
void FOR();
void STEP();
void NEWSTT();
void TRACE_();

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
            OUTDO();
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

struct LineAddress;
std::uint8_t ReadProgramByte(LineAddress base, std::uint8_t offset);
std::uint16_t ToWord(LineAddress address);
bool IsEndOfProgram(LineAddress current);
LineAddress AdvanceToNextLine(LineAddress current);

std::uint8_t CHRGOT();
struct LineAddress {
    std::uint8_t lo = 0;
    std::uint8_t hi = 0;
};

LineAddress FromWord(std::uint16_t value);

bool ISCNTC();
void LINPRT();
void OUTDO();
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

LineNumber ReadProgramLineNumber(LineAddress current) {
    return LineNumber{ReadProgramByte(current, 2), ReadProgramByte(current, 3)};
}

void PrintListLine(LineAddress current) {
    std::uint8_t offset = 4;
    while (true) {
        const std::uint8_t ch = ReadProgramByte(current, offset);
        if (ch == 0) {
            break;
        }

        // TODO(asm-port): reproduce LIST token/keyword conversion and output
        // behavior from the original Applesoft source.
        OUTDO();
        ++offset;
    }
}

bool FNDLIN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNDLIN (inclusive) .. NEW (exclusive)
    // Name normalization: none (assembler label FNDLIN kept verbatim).

    constexpr std::uint8_t kTXTTAB = 0x67;
    constexpr std::uint8_t kLOWTR = 0x9b;
    constexpr std::uint8_t kLINNUM = 0x50;

    const std::uint16_t targetLow = ReadZeroPageByte(kLINNUM);
    const std::uint16_t targetHigh = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1));
    std::uint16_t current = ReadZeroPageWord(kTXTTAB);

    while (true) {
        WriteZeroPageWord(kLOWTR, current);

        const std::uint8_t nextHigh = ReadProgramByte(current + 1);
        if (nextHigh == 0) {
            return false;
        }

        const std::uint8_t currentHigh = ReadProgramByte(current + 3);
        if (targetHigh < currentHigh) {
            return false;
        }

        if (targetHigh == currentHigh) {
            const std::uint8_t currentLow = ReadProgramByte(current + 2);
            if (targetLow < currentLow) {
                return false;
            }
            if (targetLow == currentLow) {
                return true;
            }
        }

        const std::uint8_t nextLow = ReadProgramByte(current);
        current = static_cast<std::uint16_t>(nextHigh) << 8 | nextLow;
    }
}

void DeleteExistingLine() {
    // TODO(asm-port): delete the existing numbered line and shift later lines down.
}

void InsertNewLine() {
    // TODO(asm-port): make room and copy the new numbered line into the program listing.
}

std::uint8_t read_INPUT_BUFFER(std::uint8_t index) {
    return variables_const().readByte(static_cast<std::uint16_t>(0x0200u + index));
}

void write_INPUT_BUFFER_minus_5(std::uint8_t index, std::uint8_t value) {
    variables().writeByte(static_cast<std::uint16_t>(0x01fbu + index), value);
}

void SetTextPointerToInputBufferMinus1() {
    // TODO(asm-port): compute the actual INPUT_BUFFER-1 address in the runtime model.
    SetTextPointer(0xffu, 0x01u);
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

    LineAddress current = FromWord(ReadZeroPageWord(kLOWTR));
    while (!IsEndOfProgram(current)) {
        if (ISCNTC()) {
            break;
        }

        CRDO();
        const LineNumber currentLine = ReadProgramLineNumber(current);
        if (IsLineNumberGreater(currentLine, endRange)) {
            break;
        }

        LINPRT();
        WriteZeroPageByte(kMON_CH, 5);
        PrintListLine(current);

        current = AdvanceToNextLine(current);
        WriteZeroPageWord(kLOWTR, ToWord(current));
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
    SetTextPointer(inlin2.x, inlin2.y);
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
    return static_cast<std::uint16_t>(address.hi) << 8 | address.lo;
}

LineAddress FromWord(std::uint16_t value) {
    return LineAddress{static_cast<std::uint8_t>(value & 0xff), static_cast<std::uint8_t>(value >> 8)};
}

LineAddress GetTextTableAddress() {
    constexpr std::uint8_t kTXTTAB = 0x67;
    return FromWord(ReadZeroPageWord(kTXTTAB));
}

bool IsEndOfProgram(LineAddress current) {
    // The original FIX_LINKS loop terminates by jumping to RESTART after processing
    // the final program line. A zero address is the safest sentinel for now.
    return current.lo == 0 && current.hi == 0;
}

std::uint8_t ReadProgramByte(LineAddress base, std::uint8_t offset) {
    // TODO(asm-port): read a byte from the program memory buffer at base+offset.
    (void)base;
    (void)offset;
    return 0;
}

LineAddress AdvanceToNextLine(LineAddress current) {
    // The original FIX_LINKS routine scans from the current line until it finds the
    // end-of-line marker, then computes the address of the next line.
    std::uint8_t offset = 4;
    while (ReadProgramByte(current, offset) != 0) {
        ++offset;
    }

    const std::uint16_t nextAddress = ToWord(current) + static_cast<std::uint16_t>(offset) + 1u;
    return FromWord(nextAddress);
}

void WriteForwardPointer(LineAddress current, LineAddress next) {
    // TODO(asm-port): write the low/high bytes of 'next' into the current line's forward-pointer header.
    (void)current;
    (void)next;
}

void FIX_LINKS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIX_LINKS (inclusive) .. INLIN (exclusive)
    // Name normalization: none (assembler label FIX_LINKS kept verbatim).

    SETPTRS();

    LineAddress current = GetTextTableAddress();
    while (true) {
        if (IsEndOfProgram(current)) {
            RESTART();
            return;
        }

        const LineAddress next = AdvanceToNextLine(current);
        WriteForwardPointer(current, next);
        current = next;
    }
}

void CRDO() {
    // TODO(asm-port): print a carriage return or perform monitor return behavior.
}

void STROUT(std::string_view text) {
    // TODO(asm-port): print the given string to the Applesoft console.
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

void OUTDO() {
    // TODO(asm-port): write the current output character from the Applesoft line
    // printer to the console.
}

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
    // TODO(asm-port): enforce numeric semantics for the current expression.
}

void FRMNUM() {
    // TODO(asm-port): convert the parsed numeric expression into FAC format.
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

void DATAN() {
    // TODO(asm-port): scan ahead from the current statement to the next token.
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

void OUTSP() {
    // TODO(asm-port): output the trace-space separator after the line number.
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
    constexpr std::uint8_t kTXTPTR_plus_1 = 0xb9;
    constexpr std::uint8_t kCURLIN = 0x75;
    constexpr std::uint8_t kCURLIN_plus_1 = 0x76;
    constexpr std::uint8_t kOLDTEXT = 0x79;
    constexpr std::uint8_t kOLDTEXT_plus_1 = 0x7a;
    constexpr std::uint8_t kOLDLIN = 0x77;
    constexpr std::uint8_t kOLDLIN_plus_1 = 0x78;

    const std::uint8_t txPtrLo = ReadZeroPageByte(kTXTPTR);
    const std::uint8_t txPtrHi = ReadZeroPageByte(kTXTPTR_plus_1);
    const std::uint8_t currentPageHi = ReadZeroPageByte(kCURLIN_plus_1);

    if (static_cast<std::uint8_t>(currentPageHi + 1u) != 0u) {
        WriteZeroPageByte(kOLDTEXT, txPtrLo);
        WriteZeroPageByte(kOLDTEXT_plus_1, txPtrHi);
        WriteZeroPageByte(kOLDLIN, ReadZeroPageByte(kCURLIN));
        WriteZeroPageByte(kOLDLIN_plus_1, currentPageHi);
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
    constexpr std::uint8_t kOLDLIN_plus_1 = 0x78;
    constexpr std::uint8_t kTXTPTR = 0xb8;
    constexpr std::uint8_t kTXTPTR_plus_1 = 0xb9;
    constexpr std::uint8_t kCURLIN = 0x75;
    constexpr std::uint8_t kCURLIN_plus_1 = 0x76;

    if (ReadZeroPageByte(kOLDTEXT_plus_1) == 0) {
        ERROR(ERR_CANTCONT);
        return;
    }

    WriteZeroPageByte(kTXTPTR, ReadZeroPageByte(kOLDTEXT));
    WriteZeroPageByte(kTXTPTR_plus_1, ReadZeroPageByte(kOLDTEXT_plus_1));
    WriteZeroPageByte(kCURLIN, ReadZeroPageByte(kOLDLIN));
    WriteZeroPageByte(kCURLIN_plus_1, ReadZeroPageByte(kOLDLIN_plus_1));
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

void GOTO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GOTO (inclusive) .. GOSUB (exclusive)
    // Name normalization: none (assembler label GOTO kept verbatim).
    //
    // Parses a line number and searches for it in the program.
    // Sets TXTPTR to the start of the found line, or raises error if not found.

    // TODO(asm-port): Parse the line number from current TXTPTR using LINGET.
    // TODO(asm-port): Determine search direction based on current line number.
    // TODO(asm-port): Call FL1 or FNDLIN to search for the target line.
    // TODO(asm-port): Set TXTPTR to the found line address, or error if not found.
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
    constexpr std::uint8_t kCURLIN_lo = 0x75;
    
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
    
    // TODO(asm-port): Parse line number and search for it (shared with GOTO)
    // For now, call GOTO directly since it handles the line number parsing and search.
    // After GOTO returns, TXTPTR points to the start of the target line.
    GOTO();
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

    constexpr std::uint8_t kTXTPTR_lo = 0xb8;
    constexpr std::uint8_t kTXTPTR_hi = 0xb9;
    constexpr std::uint8_t kCURLIN_lo = 0x75;
    constexpr std::uint8_t kCURLIN_hi = 0x76;
    constexpr std::uint8_t kTOKEN_GOSUB = 0xb0;
    
    // TODO(asm-port): Check stack space using CHKMEM (6502: lda #3, jsr CHKMEM).
    // TODO(asm-port): Push return frame onto 6502 stack in reverse order:
    //   pha (TXTPTR+1)
    //   pha (TXTPTR+0)
    //   pha (CURLIN+1)
    //   pha (CURLIN+0)
    //   pha (TOKEN_GOSUB = 0xB0)
    //
    // This will save the current execution state so that a RETURN statement
    // can restore it after executing the subroutine.
    
    // Call GOTO to parse the line number and set TXTPTR to the target line
    GOTO();
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
    // TODO(asm-port): evaluate a variable assignment and store its address into FORPNT.
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

void OUTQUES() {
    // TODO(asm-port): print the question mark prompt after an error.
}

} // namespace applesoft::asm_port
