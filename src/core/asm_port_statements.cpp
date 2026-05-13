#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_token_name_table.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace applesoft::asm_port {

constexpr std::size_t kTokenCount = 107;
constexpr std::uint8_t kTokenBase = 0x80u;

std::uint16_t ReadZeroPageWord(std::uint8_t address);
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void SetTextPointer(std::uint16_t address);
void RESTART();
void MON_WRITE();
void MON_READ();
bool SETPTRS();
void FIX_LINKS();
void VARTIO();
void PROGIO();
std::uint16_t PTRGET();
void SYNCHR(std::uint8_t expected);
void FRMEVL();
bool CHKVAL(std::uint8_t savedValTyp);
void ROUND_FAC();
void AYINT();
void SETFOR();
void STRINI(std::uint8_t length);
void MOVINS();
bool FRETMS(std::uint16_t descriptorAddress);
void LET2(std::uint8_t savedValTypPlus1);
void PUTSTR();
void LINGET();
bool FNDLIN();
std::uint8_t CHRGOT();
std::uint8_t CHRGET();
bool IsStatementEndOfParsedInput();
bool ISCNTC();
void CRDO();
void LINPRT();
void OUTDO(std::uint8_t value);
bool FL1(std::uint16_t startAddress);
void RESTORE();
void STKINI();
std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);
bool NEW_impl();
void SCRTCH_impl();
bool SETPTRS_impl();
bool CLEAR_impl();
void CLEARC_impl();
void STXTPT_impl();
void GO_TO_LINE();

bool NEW_impl() {
    if (!IsStatementEndOfParsedInput()) {
        return false;
    }

    SCRTCH_impl();
    return true;
}

void SCRTCH_impl() {
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kLOCK = ApplesoftVariables::ZP_LOCK;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kPRGEND = ApplesoftVariables::ZP_PRGEND;
    constexpr std::uint8_t kARYTAB = ApplesoftVariables::ZP_ARYTAB;
    constexpr std::uint8_t kSTREND = ApplesoftVariables::ZP_STREND;
    constexpr std::uint8_t kMEMSIZ = ApplesoftVariables::ZP_MEMSIZ;
    constexpr std::uint8_t kFRETOP = ApplesoftVariables::ZP_FRETOP;

    const std::uint16_t txtTabAddr = ReadZeroPageWord(kTXTTAB);
    WriteZeroPageByte(kLOCK, 0);
    WriteProgramByte(txtTabAddr, 0);
    WriteProgramByte(static_cast<std::uint16_t>(txtTabAddr + 1u), 0);

    const std::uint16_t nextFree = static_cast<std::uint16_t>(txtTabAddr + 2u);
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
    constexpr std::uint8_t kMEMSIZ = ApplesoftVariables::ZP_MEMSIZ;
    constexpr std::uint8_t kFRETOP = ApplesoftVariables::ZP_FRETOP;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kARYTAB = ApplesoftVariables::ZP_ARYTAB;
    constexpr std::uint8_t kSTREND = ApplesoftVariables::ZP_STREND;

    WriteZeroPageWord(kFRETOP, ReadZeroPageWord(kMEMSIZ));
    WriteZeroPageWord(kARYTAB, ReadZeroPageWord(kVARTAB));
    WriteZeroPageWord(kSTREND, ReadZeroPageWord(kVARTAB));
    RESTORE();
    STKINI();
}

void STXTPT_impl() {
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    const std::uint16_t textTable = ReadZeroPageWord(kTXTTAB);
    WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(textTable - 1u));
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

void RUN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RUN (inclusive) .. GOSUB (exclusive)
    // Name normalization: none (assembler label RUN kept verbatim).
    constexpr std::uint8_t kCURLIN_hi = static_cast<std::uint8_t>(ApplesoftVariables::ZP_CURLIN + 1u);

    std::uint8_t curlinHi = ReadZeroPageByte(kCURLIN_hi);
    WriteZeroPageByte(kCURLIN_hi, static_cast<std::uint8_t>(curlinHi - 1));

    const std::uint8_t currentChar = CHRGOT();
    if (currentChar == 0) {
        SETPTRS();
        return;
    }

    CLEARC();
    GO_TO_LINE();
}


struct TokenMatch {
    std::uint8_t code;
    std::uint8_t length;
    std::string_view name;
};

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

        const std::uint8_t tokenCode = static_cast<std::uint8_t>(kTokenBase + static_cast<std::uint8_t>(i));
        if (!best || token.size() > best->length) {
            best = TokenMatch{tokenCode,
                              static_cast<std::uint8_t>(token.size()),
                              token};
        }
    }

    return best;
}

std::uint8_t ScanAheadOffsetForData(std::uint8_t terminator) {
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
            const std::uint16_t textPtr = ReadZeroPageWord(kTXTPTR);
            const std::uint8_t ch = variables_const().pointer(textPtr).read(offset);
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

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    const std::uint16_t textPtr = ReadZeroPageWord(kTXTPTR);
    WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(textPtr + offset));
}

std::uint8_t DATAN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DATAN (inclusive) .. REMN (exclusive)
    // Name normalization: none (assembler label DATAN kept verbatim).

    return ScanAheadOffsetForData(static_cast<std::uint8_t>(':'));
}

void DATA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DATA (inclusive) .. ADDON (exclusive)
    // Name normalization: none (assembler label DATA kept verbatim).

    const std::uint8_t offset = DATAN();
    ADDON(offset);
}

void LET() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LET (inclusive) .. LET2 (exclusive)
    // Name normalization: none (assembler label LET kept verbatim).

    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    constexpr std::uint8_t kTOKEN_EQUAL = 0xd0;
    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;

    const std::uint16_t variablePtr = PTRGET();
    WriteZeroPageWord(kFORPNT, variablePtr);

    SYNCHR(kTOKEN_EQUAL);

    const std::uint8_t savedValTyp = ReadZeroPageByte(kVALTYP);
    const std::uint8_t savedValTypPlus1 = ReadZeroPageByte(static_cast<std::uint8_t>(kVALTYP + 1u));

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

    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    constexpr std::uint8_t kFAC_PLUS_3 = static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 3u);
    constexpr std::uint8_t kFAC_PLUS_4 = static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 4u);

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

    constexpr std::uint8_t kFAC_PLUS_3 = static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 3u);
    constexpr std::uint8_t kDSCPTR = ApplesoftVariables::ZP_DSCPTR;
    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    constexpr std::uint8_t kFRETOP = ApplesoftVariables::ZP_FRETOP;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kSTRNG1 = ApplesoftVariables::ZP_STRNG1;

    const std::uint16_t facDescriptor = ReadZeroPageWord(kFAC_PLUS_3);
    const auto facDescriptorPtr = variables_const().pointer(facDescriptor);
    auto readDescriptorByte = [&](std::uint8_t offset) {
        return facDescriptorPtr.read(offset);
    };

    std::uint16_t descriptorPointer = ReadZeroPageWord(kFAC_PLUS_3);

    const std::uint8_t descDataHigh = readDescriptorByte(2);
    const std::uint8_t fretopHigh = ReadZeroPageByte(static_cast<std::uint8_t>(kFRETOP + 1u));

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

    (void)FRETMS(descriptorPointer);

    const std::uint16_t source = ReadZeroPageWord(kDSCPTR);
    const std::uint16_t dest = ReadZeroPageWord(kFORPNT);
    const auto sourcePtr = variables_const().pointer(source);
    auto destPtr = variables().pointer(dest);
    for (std::uint8_t i = 0; i < 3; ++i) {
        destPtr.write(sourcePtr.read(i), i);
    }
}

void DeleteExistingLine() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NUMBERED_LINE delete block (inclusive) .. PUT_NEW_LINE (exclusive)
    // Name normalization: C++ helper; corresponds to the inline delete block inside
    // NUMBERED_LINE (T:0471–T:04b5) in the assembler listing.

    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;

    const std::uint16_t lowtr = ReadZeroPageWord(kLOWTR);
    const std::uint16_t nextLine = ApplesoftVariables::makeWord(
        variables_const().readByte(lowtr),
        variables_const().readByte(static_cast<std::uint16_t>(lowtr + 1u)));

    const std::uint16_t lineSize = static_cast<std::uint16_t>(nextLine - lowtr);
    const std::uint16_t vartab = ReadZeroPageWord(kVARTAB);

    const std::uint16_t moveCount = static_cast<std::uint16_t>(vartab - nextLine);
    for (std::uint16_t i = 0; i < moveCount; ++i) {
        variables().writeByte(
            static_cast<std::uint16_t>(lowtr + i),
            variables_const().readByte(static_cast<std::uint16_t>(nextLine + i)));
    }

    WriteZeroPageWord(kVARTAB, static_cast<std::uint16_t>(vartab - lineSize));
}

void InsertNewLine() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PUT_NEW_LINE (inclusive) .. FIX_LINKS (exclusive)
    // Name normalization: C++ helper; corresponds to PUT_NEW_LINE (T:04b5–T:04f2).

    constexpr std::uint16_t kTokenBuf =
        static_cast<std::uint16_t>(ApplesoftVariables::ADDR_INPUT_BUFFER - 5u);
    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kSTREND = ApplesoftVariables::ZP_STREND;
    constexpr std::uint8_t kFRETOP = ApplesoftVariables::ZP_FRETOP;
    constexpr std::uint8_t kMEMSIZ = ApplesoftVariables::ZP_MEMSIZ;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;

    if (variables_const().readByte(ApplesoftVariables::ADDR_INPUT_BUFFER) == 0u) {
        return;
    }

    WriteZeroPageWord(kFRETOP, ReadZeroPageWord(kMEMSIZ));

    std::uint16_t tokenLen = 0;
    while (variables_const().readByte(static_cast<std::uint16_t>(kTokenBuf + tokenLen)) != 0u) {
        ++tokenLen;
    }

    const std::uint16_t lineSize = static_cast<std::uint16_t>(tokenLen + 5u);
    const std::uint16_t lowtr = ReadZeroPageWord(kLOWTR);
    const std::uint16_t vartab = ReadZeroPageWord(kVARTAB);

    for (std::uint16_t i = vartab; i > lowtr; --i) {
        const std::uint16_t src = static_cast<std::uint16_t>(i - 1u);
        variables().writeByte(
            static_cast<std::uint16_t>(src + lineSize),
            variables_const().readByte(src));
    }

    const std::uint16_t linnum = ReadZeroPageWord(kLINNUM);
    variables().writeByte(lowtr, 0u);
    variables().writeByte(static_cast<std::uint16_t>(lowtr + 1u), 0u);
    variables().writeByte(static_cast<std::uint16_t>(lowtr + 2u), ApplesoftVariables::lowByte(linnum));
    variables().writeByte(static_cast<std::uint16_t>(lowtr + 3u), ApplesoftVariables::highByte(linnum));

    for (std::uint16_t i = 0; i <= tokenLen; ++i) {
        variables().writeByte(
            static_cast<std::uint16_t>(lowtr + 4u + i),
            variables_const().readByte(static_cast<std::uint16_t>(kTokenBuf + i)));
    }

    const std::uint16_t newVartab = static_cast<std::uint16_t>(vartab + lineSize);
    WriteZeroPageWord(kVARTAB, newVartab);
    WriteZeroPageWord(kSTREND, newVartab);
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
        if (ch == 0u) {
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

        if (ch == static_cast<std::uint8_t>(0x22u)) {
            write_INPUT_BUFFER_minus_5(outputIndex++, ch);
            ++inputIndex;
            while (true) {
                const std::uint8_t quoteChar = read_INPUT_BUFFER(inputIndex);
                if (quoteChar == 0u) {
                    break;
                }
                write_INPUT_BUFFER_minus_5(outputIndex++, quoteChar);
                ++inputIndex;
                if (quoteChar == static_cast<std::uint8_t>(0x22u)) {
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

    write_INPUT_BUFFER_minus_5(outputIndex, 0u);
    SetTextPointerToInputBufferMinus1();
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

std::uint16_t GetTextTablePointer() {
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    return ReadZeroPageWord(kTXTTAB);
}

bool IsEndOfProgram(std::uint16_t current) {
    return current == 0u;
}

std::uint16_t AdvanceToNextLine(std::uint16_t current) {
    // The original FIX_LINKS routine scans from the current line until it finds the
    // end-of-line marker, then computes the address of the next line.
    std::uint16_t offset = 4u;
    while (variables_const().readByte(static_cast<std::uint16_t>(current + offset)) != 0u) {
        ++offset;
    }

    return static_cast<std::uint16_t>(current + offset + 1u);
}

void WriteForwardPointer(std::uint16_t current, std::uint16_t next) {
    variables().writeByte(current, ApplesoftVariables::lowByte(next));
    variables().writeByte(static_cast<std::uint16_t>(current + 1u), ApplesoftVariables::highByte(next));
}

void FIX_LINKS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIX_LINKS (inclusive) .. INLIN (exclusive)
    // Name normalization: none (assembler label FIX_LINKS kept verbatim).

    SETPTRS();

    std::uint16_t current = GetTextTablePointer();
    while (true) {
        if (IsEndOfProgram(current)) {
            RESTART();
            return;
        }

        const std::uint16_t next = AdvanceToNextLine(current);
        WriteForwardPointer(current, next);
        current = next;
    }
}

bool FNDLIN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNDLIN (inclusive) .. FL1 (exclusive)
    // Name normalization: none (assembler label FNDLIN kept verbatim).

    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;

    // Assembler falls through from FNDLIN directly into FL1 with A=TXTTAB, X=TXTTAB+1.
    return FL1(ReadZeroPageWord(kTXTTAB));
}

bool FL1(std::uint16_t startAddress) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FL1 (inclusive) .. NEW (exclusive)
    // Name normalization: none (assembler label FL1 kept verbatim).

    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;

    const std::uint8_t targetLo = ReadZeroPageByte(kLINNUM);
    const std::uint8_t targetHi = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1u));

    std::uint16_t current = startAddress;

    while (true) {
        WriteZeroPageWord(kLOWTR, current);

        const std::uint8_t nextHi = variables_const().readByte(static_cast<std::uint16_t>(current + 1u));
        if (nextHi == 0u) {
            return false;
        }

        const std::uint8_t lineHi = variables_const().readByte(static_cast<std::uint16_t>(current + 3u));
        if (targetHi < lineHi) {
            return false;
        }

        if (targetHi == lineHi) {
            const std::uint8_t lineLo = variables_const().readByte(static_cast<std::uint16_t>(current + 2u));
            if (targetLo < lineLo) {
                return false;
            }
            if (targetLo == lineLo) {
                return true;
            }
        }

        const std::uint8_t nextLo = variables_const().readByte(current);
        current = ApplesoftVariables::makeWord(nextLo, nextHi);
    }
}

bool FL1(std::uint8_t startLo, std::uint8_t startHi) {
    return FL1(ApplesoftVariables::makeWord(startLo, startHi));
}

void DEL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DEL (inclusive) .. GR (exclusive)
    // Name normalization: none (assembler label DEL kept verbatim).

    constexpr std::uint8_t kPRGEND = ApplesoftVariables::ZP_PRGEND;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kDEST = ApplesoftVariables::ZP_DEST;
    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;

    const std::uint16_t prgend = ReadZeroPageWord(kPRGEND);
    WriteZeroPageWord(kVARTAB, prgend);

    LINGET();
    FNDLIN();
    WriteZeroPageWord(kDEST, ReadZeroPageWord(kLOWTR));

    SYNCHR(static_cast<std::uint8_t>(','));
    LINGET();

    std::uint8_t linnumLo = ReadZeroPageByte(kLINNUM);
    if (linnumLo == 0xffu) {
        WriteZeroPageByte(kLINNUM, 0u);
        const std::uint8_t linnumHi = ReadZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1u));
        WriteZeroPageByte(static_cast<std::uint8_t>(kLINNUM + 1u), static_cast<std::uint8_t>(linnumHi + 1u));
    } else {
        WriteZeroPageByte(kLINNUM, static_cast<std::uint8_t>(linnumLo + 1u));
    }

    FNDLIN();

    const std::uint16_t lowtr = ReadZeroPageWord(kLOWTR);
    const std::uint16_t dest = ReadZeroPageWord(kDEST);
    if (lowtr < dest) {
        return;
    }

    const std::uint16_t vartab = ReadZeroPageWord(kVARTAB);
    std::uint16_t source = lowtr;
    std::uint16_t destination = dest;
    while (source < vartab) {
        const std::uint8_t byteVal = variables_const().readByte(source);
        variables().writeByte(destination, byteVal);
        ++source;
        ++destination;
    }

    const std::uint16_t deletedSize = static_cast<std::uint16_t>(lowtr - dest);
    const std::uint16_t newVartab = static_cast<std::uint16_t>(vartab - deletedSize);
    WriteZeroPageWord(kVARTAB, newVartab);

    FIX_LINKS();
}

void LIST() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LIST (inclusive) .. AUTO (exclusive)
    // Name normalization: none (assembler label LIST kept verbatim).

    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kMON_CH = ApplesoftVariables::ZP_MON_CH;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;

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

    std::uint16_t endRange = ReadZeroPageWord(kLINNUM);
    if (endRange == 0u) {
        endRange = 0xffffu;
    }

    std::uint16_t current = ReadZeroPageWord(kLOWTR);
    while (current != 0u) {
        if (ISCNTC()) {
            break;
        }

        CRDO();

        const std::uint16_t currentLine = ApplesoftVariables::makeWord(
            variables_const().readByte(static_cast<std::uint16_t>(current + 2u)),
            variables_const().readByte(static_cast<std::uint16_t>(current + 3u)));
        if (currentLine > endRange) {
            break;
        }

        WriteZeroPageWord(kCURLIN, currentLine);
        LINPRT();
        WriteZeroPageByte(kMON_CH, 5u);

        std::uint16_t offset = 4u;
        while (true) {
            const std::uint8_t ch = variables_const().readByte(static_cast<std::uint16_t>(current + offset));
            if (ch == 0u) {
                break;
            }
            OUTDO(static_cast<std::uint8_t>(ch & 0x7fu));
            ++offset;
        }

        current = static_cast<std::uint16_t>(current + offset + 1u);
        WriteZeroPageWord(kLOWTR, current);
    }

    CRDO();
}

void SAVE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SAVE (inclusive) .. LOAD (exclusive)
    // Name normalization: none (assembler label SAVE kept verbatim).

    constexpr std::uint8_t kPRGEND = ApplesoftVariables::ZP_PRGEND;
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;

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

    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kTEMPPT = ApplesoftVariables::ZP_TEMPPT;
    constexpr std::uint8_t kLOCK = ApplesoftVariables::ZP_LOCK;

    VARTIO();
    MON_READ();

    const std::uint16_t textTable = ReadZeroPageWord(kTXTTAB);
    const std::uint16_t programLength = ReadZeroPageWord(kLINNUM);
    WriteZeroPageWord(kVARTAB, static_cast<std::uint16_t>(textTable + programLength));

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

    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kTEMPPT = ApplesoftVariables::ZP_TEMPPT;
    constexpr std::uint8_t kLOCK = ApplesoftVariables::ZP_LOCK;
    constexpr std::uint8_t kMON_A1L = ApplesoftVariables::ZP_MON_A1;
    constexpr std::uint8_t kMON_A1H = static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A1 + 1u);
    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_A2H = static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A2 + 1u);

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

    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kMON_A1L = ApplesoftVariables::ZP_MON_A1;
    constexpr std::uint8_t kMON_A1H = static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A1 + 1u);
    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_A2H = static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A2 + 1u);

    WriteZeroPageWord(kMON_A1L, ReadZeroPageWord(kTXTTAB));
    WriteZeroPageWord(kMON_A2L, ReadZeroPageWord(kVARTAB));

    // Keep symbolic names visible for monitor register parity.
    (void)kMON_A1H;
    (void)kMON_A2H;
}

void MON_WRITE() {
    // TODO(asm-port): port monitor tape write handler used by SAVE.
}

void MON_READ() {
    // TODO(asm-port): port monitor tape read handler used by LOAD.
}

}  // namespace applesoft::asm_port