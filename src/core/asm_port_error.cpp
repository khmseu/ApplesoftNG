#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/asm_port_qt_error.hpp"
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
    // TODO(asm-port): update the TXTPTR pointer into the input buffer.
    (void)lo;
    (void)hi;
}

void ClearErrFlag() {
    // TODO(asm-port): clear the Applesoft ERRFLG state.
}

void MarkDirectMode() {
    // TODO(asm-port): record the current execution mode as direct.
}

void LINGET() {
    // TODO(asm-port): consume the line number prefix from the input buffer.
}

std::uint8_t ReadZeroPageByte(std::uint8_t address) {
    // TODO(asm-port): read a byte from zero-page storage.
    (void)address;
    return 0;
}

void WriteZeroPageByte(std::uint8_t address, std::uint8_t value) {
    // TODO(asm-port): write a byte to zero-page storage.
    (void)address;
    (void)value;
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
    // TODO(asm-port): read a byte from INPUT_BUFFER + index in the runtime input buffer.
    (void)index;
    return 0;
}

void write_INPUT_BUFFER_minus_5(std::uint8_t index, std::uint8_t value) {
    // TODO(asm-port): write a byte to INPUT_BUFFER-5 + index in the runtime output buffer.
    (void)index;
    (void)value;
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

void TRACE_() {
    // TODO(asm-port): execute the parsed input line or trace it in the interpreter.
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
    // TODO(asm-port): detect whether a Control-C interrupt was typed.
    return false;
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
