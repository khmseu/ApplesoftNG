#include "core/asm_port_error.hpp"
#include "core/asm_port_print.hpp"
#include "platform/asm_port_outdo.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/asm_port_reason.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_token_address_table.hpp"
#include "core/asm_port_token_name_table.hpp"

#include <array>
#include <optional>
#include <string_view>

namespace applesoft::asm_port {

constexpr std::uint8_t RESTART_PROMPT = ']' | 0x80u;
constexpr std::size_t kTokenCount = 107;
constexpr std::uint8_t kTokenBase = 0x80u;

bool isDigit(std::uint8_t ch) {
    return ch >= '0' && ch <= '9';
}

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void SetStackPointer(std::uint8_t value);
void PushByteToStack(std::uint8_t value);

void SCRTCH();
void RESTART();
void CRDO();

// TODO(asm-port): port NORMAL statement behavior (currently display-mode init stub).
void NORMAL() {}

constexpr std::array<std::uint8_t, 29> kGenericCHRGETImage = {
    0xe6, 0xb8, 0xd0, 0x02, 0xe6, 0xb9, 0xad, 0x60, 0xea, 0xc9,
    0x3a, 0xb0, 0x0a, 0xc9, 0x20, 0xf0, 0xef, 0x38, 0xe9, 0x30,
    0x38, 0xe9, 0xd0, 0x60, 0x80, 0x4f, 0xc7, 0x52, 0x58,
};

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

void COLD_START();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GENERIC_END (inclusive) .. COLD_START (exclusive)
// Name normalization: none (assembler label GENERIC_END kept verbatim).
void GENERIC_END() {
    // Label-only range in ROM: this address immediately falls into COLD_START.
    COLD_START();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: COLD_START (inclusive) .. CALL (exclusive)
// Name normalization: none (assembler label COLD_START kept verbatim).
void COLD_START() {
    constexpr std::uint8_t kCURLIN_HI = static_cast<std::uint8_t>(ApplesoftVariables::ZP_CURLIN + 1u);
    constexpr std::uint8_t kJmpOpcode = 0x4cu;
    constexpr std::uint16_t kColdStartROM = 0x2128u;
    constexpr std::uint16_t kStroUTROM = 0x0b3au;
    constexpr std::uint16_t kRestartROM = 0x043cu;
    constexpr std::uint16_t kIqErrROM = 0x1199u;
    constexpr std::uint16_t kChrgetRuntime = 0x00b1u;
    constexpr std::uint16_t kProgramStart = 0x0800u;

    WriteZeroPageByte(kCURLIN_HI, 0xffu);
    SetStackPointer(0xfbu);

    WriteZeroPageWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_GOWARM + 1u), kColdStartROM);
    WriteZeroPageWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_GOSTROUT + 1u), kColdStartROM);

    NORMAL();

    WriteZeroPageByte(ApplesoftVariables::ZP_GOWARM, kJmpOpcode);
    WriteZeroPageByte(ApplesoftVariables::ZP_GOSTROUT, kJmpOpcode);
    WriteZeroPageByte(ApplesoftVariables::ZP_JMPADRS, kJmpOpcode);
    WriteZeroPageByte(ApplesoftVariables::ZP_USR, kJmpOpcode);
    WriteZeroPageWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_USR + 1u), kIqErrROM);

    // Preserve the ROM copy bug: the final random-seed byte is not copied.
    for (std::uint8_t x = static_cast<std::uint8_t>(kGenericCHRGETImage.size() - 1u); x != 0u; --x) {
        variables().pointer(kChrgetRuntime).write(kGenericCHRGETImage[x - 1u], static_cast<std::uint16_t>(x - 1u));
        WriteZeroPageByte(ApplesoftVariables::ZP_SPEEDZ, x);
    }

    WriteZeroPageByte(ApplesoftVariables::ZP_TRCFLG, 0u);
    WriteZeroPageByte(ApplesoftVariables::ZP_SHIFT_SIGN_EXT, 0u);
    WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_LASTPT + 1u), 0u);
    PushByteToStack(0u);
    WriteZeroPageByte(ApplesoftVariables::ZP_DSCLEN, 3u);

    CRDO();

    variables().writeByte(ApplesoftVariables::ADDR_INPUT_BUFFER_MINUS_3, 1u);
    variables().writeByte(ApplesoftVariables::ADDR_INPUT_BUFFER_MINUS_4, 1u);
    WriteZeroPageByte(ApplesoftVariables::ZP_TEMPPT, ApplesoftVariables::ZP_TEMPST);

    // Unified RAM probe pointer lifted from LINNUM low/high carry-chain in ROM.
    std::uint16_t ramProbe = kProgramStart;
    while ((ramProbe & 0xff00u) < 0xc000u) {
        ramProbe = static_cast<std::uint16_t>(ramProbe + 0x0100u);
    }

    const std::uint16_t memoryTop = static_cast<std::uint16_t>(ramProbe & 0xf000u);
    WriteZeroPageWord(ApplesoftVariables::ZP_MEMSIZ, memoryTop);
    WriteZeroPageWord(ApplesoftVariables::ZP_FRETOP, memoryTop);

    WriteZeroPageWord(ApplesoftVariables::ZP_TXTTAB, kProgramStart);
    WriteZeroPageByte(ApplesoftVariables::ZP_LOCK, 0u);
    variables().pointer(kProgramStart).write(0u);
    WriteZeroPageWord(ApplesoftVariables::ZP_TXTTAB, static_cast<std::uint16_t>(kProgramStart + 1u));

    REASONState reasonState{};
    const std::uint16_t txttab = ReadZeroPageWord(ApplesoftVariables::ZP_TXTTAB);
    const std::uint16_t fretop = ReadZeroPageWord(ApplesoftVariables::ZP_FRETOP);
    reasonState.a = ApplesoftVariables::lowByte(txttab);
    reasonState.y = ApplesoftVariables::highByte(txttab);
    reasonState.fretopLo = ApplesoftVariables::lowByte(fretop);
    reasonState.fretopHi = ApplesoftVariables::highByte(fretop);
    const REASONResult reasonResult = REASON(reasonState);
    if (!reasonResult.ok) {
        return;
    }

    SCRTCH();

    WriteZeroPageWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_GOSTROUT + 1u), kStroUTROM);
    WriteZeroPageWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_GOWARM + 1u), kRestartROM);

    // ROM uses JMP (GOWARM+1), which resolves to RESTART after vectors are installed.
    RESTART();
}

void SetTextPointer(std::uint16_t address) {
    variables().writeWord(ApplesoftVariables::ZP_TXTPTR, address);
}

void ClearErrFlag() {
    variables().writeByte(ApplesoftVariables::ZP_ERRFLG, 0);
}

void MarkDirectMode() {
    variables().writeByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_CURLIN + 1u), 0xffu);
}

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs);
std::uint8_t CHRGOT();
void SYNERR();

void LINGET() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LINGET (inclusive) .. LET (exclusive)
    // Name normalization: none (assembler label LINGET kept verbatim).

    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kINDEX = ApplesoftVariables::ZP_INDEX;
    constexpr std::uint8_t kCHARAC = ApplesoftVariables::ZP_CHARAC;

    WriteZeroPageWord(kLINNUM, 0);

    std::uint8_t current = CHRGOT();
    while (isDigit(current)) {
        const std::uint8_t digit = static_cast<std::uint8_t>(current - static_cast<std::uint8_t>('0'));
        WriteZeroPageByte(kCHARAC, digit);

        const std::uint8_t lineHigh = ReadZeroPageByte(add_u8(kLINNUM, 1u));
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

constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
    return static_cast<std::uint8_t>(lhs + rhs);
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

std::uint8_t ReadProgramByte(std::uint16_t address);

void WriteProgramByte(std::uint16_t address, std::uint8_t value) {
    // TODO(asm-port): write a byte into program memory at the given address.
    (void)address;
    (void)value;
}

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
bool FRETMS(std::uint16_t descriptorAddress);
std::uint8_t FRETMP(std::uint16_t descriptorAddress);
void GARBAG();
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
void CHKOPN();
void CHKCLS();
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
void FNC_();
void PARCHK();
void STORE_FACDB_YX_ROUNDED();
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
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kLOCK = ApplesoftVariables::ZP_LOCK;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kPRGEND = ApplesoftVariables::ZP_PRGEND;
    constexpr std::uint8_t kARYTAB = ApplesoftVariables::ZP_ARYTAB;
    constexpr std::uint8_t kSTREND = ApplesoftVariables::ZP_STREND;
    constexpr std::uint8_t kMEMSIZ = ApplesoftVariables::ZP_MEMSIZ;
    constexpr std::uint8_t kFRETOP = ApplesoftVariables::ZP_FRETOP;

    const ProgramPointer programStart{ReadZeroPageWord(kTXTTAB)};
    WriteZeroPageByte(kLOCK, 0);
    programStart.write(0);
    programStart.write(0, 1u);

    const std::uint16_t nextFree = programStart.advanced(2u).address;
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

void FOR() {
    constexpr std::uint8_t kSUBFLG = ApplesoftVariables::ZP_SUBFLG;
    constexpr std::uint8_t kTOKEN_TO = 0x00; // TODO(asm-port): actual Applesoft "TO" token value.

    WriteZeroPageByte(kSUBFLG, 0x80);
    LET();

    GTFORPNTState gtforpntState{};
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

// TODO(asm-port): port FLOAT label.
void FLOAT() {}

// TODO(asm-port): port CONINT label.
void CONINT() {}

// TODO(asm-port): port MON_PREAD monitor paddle reader.
std::uint8_t MON_PREAD() {
    return 0;
}

void GIVAYF(std::int16_t value);

// TODO(asm-port): port QINT label.
void QINT() {}

// TODO(asm-port): port FLOAT_1 label.
void FLOAT_1(std::uint8_t exponent) {
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exponent);
}

std::uint8_t gJerErrorCode = ERR_SYNTAX;
constexpr std::uint8_t kNEG32768Data[4] = {0x90u, 0x80u, 0x00u, 0x00u};
constexpr std::uint8_t kCZeroData[2] = {0x00u, 0x00u};

void SNGFLT(std::uint8_t value) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SNGFLT (inclusive) .. ERRDIR (exclusive)
    // Name normalization: none (assembler label SNGFLT kept verbatim).

    GIVAYF(static_cast<std::int16_t>(value));
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
    constexpr std::uint8_t kREMSTK = ApplesoftVariables::ZP_REMSTK;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;

    WriteZeroPageByte(kREMSTK, ReadStackPointer());

    if (ISCNTC()) {
        return;
    }

    if (ReadZeroPageByte(add_u8(kCURLIN, 1u)) != 0xffu) {
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
    constexpr std::uint8_t kTRCFLG = ApplesoftVariables::ZP_TRCFLG;

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

bool IsEndOfProgram(ProgramPointer currentPtr);
ProgramPointer AdvanceToNextLine(ProgramPointer currentPtr);

std::uint8_t CHRGOT();
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

    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;

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

        std::uint8_t tokenCode = add_u8(kTokenBase, static_cast<std::uint8_t>(i));
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
    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kMON_CH = ApplesoftVariables::ZP_MON_CH;

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

    LineNumber endRange{ReadZeroPageByte(kLINNUM), ReadZeroPageByte(add_u8(kLINNUM, 1u))};
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

ProgramPointer GetTextTablePointer() {
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    return ProgramPointer{ReadZeroPageWord(kTXTTAB)};
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

void WriteForwardPointer(ProgramPointer currentPtr, ProgramPointer nextPtr) {
    currentPtr.write(ApplesoftVariables::lowByte(nextPtr.address), 0u);
    currentPtr.write(ApplesoftVariables::highByte(nextPtr.address), 1u);
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
    // code.  It is distinct from STROUT(std::uint16_t address) in asm_port_print.
    (void)text;
}

void INPRT() {
    // TODO(asm-port): print the current line number when running a program.
}

void STKINI() {
    constexpr std::uint8_t kTEMPPT = ApplesoftVariables::ZP_TEMPPT;
    constexpr std::uint8_t kTEMPST = ApplesoftVariables::ZP_TEMPST;
    constexpr std::uint8_t kOLDTEXT_plus_1 = add_u8(ApplesoftVariables::ZP_OLDTEXT, 1u);
    constexpr std::uint8_t kSUBFLG = ApplesoftVariables::ZP_SUBFLG;

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
    constexpr std::uint8_t kCTRL_C_CODE = 0x83;

    if (variables_const().readByte(ApplesoftVariables::ADDR_KEYBOARD) != kCTRL_C_CODE) {
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
    
    // Advance TXTPTR by 1
    WriteZeroPageWord(kTXTPTR, txtPtr + 1u);
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

void ADDON(std::uint8_t offset) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ADDON (inclusive) .. DATAN (exclusive)
    // Name normalization: none (assembler label ADDON kept verbatim).

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    const ProgramPointer textPtr{ReadZeroPageWord(kTXTPTR)};
    WriteZeroPageWord(kTXTPTR, textPtr.advanced(offset).address);
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
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    const std::uint16_t textTable = ReadZeroPageWord(kTXTTAB);
    const std::uint16_t dataPointer = static_cast<std::uint16_t>(textTable - 1u);
    SETDA(dataPointer);
}

void SETDA(std::uint16_t dataPointer) {
    constexpr std::uint8_t kDATPTR = ApplesoftVariables::ZP_DATPTR;
    WriteZeroPageWord(kDATPTR, dataPointer);
}

void CONTROL_C_TYPED() {
    constexpr std::uint8_t kERRFLG = ApplesoftVariables::ZP_ERRFLG;
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

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;
    constexpr std::uint8_t kOLDLIN = ApplesoftVariables::ZP_OLDLIN;

    const std::uint16_t textPointer = ReadZeroPageWord(kTXTPTR);
    const std::uint16_t currentLine = ReadZeroPageWord(kCURLIN);
    const std::uint8_t currentPageHi = ApplesoftVariables::highByte(currentLine);

    if (add_u8(currentPageHi, 1u) != 0u) {
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

    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;
    constexpr std::uint8_t kOLDTEXT_plus_1 = add_u8(ApplesoftVariables::ZP_OLDTEXT, 1u);
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

    const ProgramPointer textTablePtr{ReadZeroPageWord(kTXTTAB)};
    const std::uint16_t programLength = ReadZeroPageWord(kLINNUM);
    WriteZeroPageWord(kVARTAB, textTablePtr.advanced(programLength).address);

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
    constexpr std::uint8_t kMON_A1H = add_u8(ApplesoftVariables::ZP_MON_A1, 1u);
    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_A2H = add_u8(ApplesoftVariables::ZP_MON_A2, 1u);

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
    constexpr std::uint8_t kMON_A1H = add_u8(ApplesoftVariables::ZP_MON_A1, 1u);
    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_A2H = add_u8(ApplesoftVariables::ZP_MON_A2, 1u);

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

    constexpr std::uint8_t kCURLIN_hi = add_u8(ApplesoftVariables::ZP_CURLIN, 1u);

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

    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;

    LINGET();
    const std::uint8_t remnOffset = REMN();

    const std::uint8_t currentPage = ReadZeroPageByte(add_u8(kCURLIN, 1u));
    const std::uint8_t targetPage = ReadZeroPageByte(add_u8(kLINNUM, 1u));

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
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: POP (inclusive) .. RETURN (exclusive)
    // Name normalization: none (assembler label POP kept verbatim).

    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
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

    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

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

    constexpr std::uint8_t kLOWTR = ApplesoftVariables::ZP_LOWTR;
    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;

    const std::uint8_t targetLo = ReadZeroPageByte(kLINNUM);
    const std::uint8_t targetHi = ReadZeroPageByte(add_u8(kLINNUM, 1u));

    ProgramPointer currentPtr{startAddress};

    while (true) {
        WriteZeroPageWord(kLOWTR, currentPtr.address);

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
        currentPtr = ProgramPointer{ApplesoftVariables::makeWord(nextLo, nextHi)};
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
    constexpr std::uint8_t kFAC_PLUS_4 = add_u8(ApplesoftVariables::ZP_FAC, 4u);

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
    constexpr std::uint8_t kFORPNT = ApplesoftVariables::ZP_FORPNT;
    PushByteToStack(ReadZeroPageByte(add_u8(kFORPNT, 1u)));
    PushByteToStack(ReadZeroPageByte(kFORPNT));
    PushTokenTo(TOKEN_FOR);
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
    const std::uint8_t savedValTypPlus1 = ReadZeroPageByte(add_u8(kVALTYP, 1u));

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
    constexpr std::uint8_t kFAC_PLUS_3 = add_u8(ApplesoftVariables::ZP_FAC, 3u);
    constexpr std::uint8_t kFAC_PLUS_4 = add_u8(ApplesoftVariables::ZP_FAC, 4u);

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

    constexpr std::uint8_t kFAC_PLUS_3 = add_u8(ApplesoftVariables::ZP_FAC, 3u);
    constexpr std::uint8_t kFAC_PLUS_4 = add_u8(ApplesoftVariables::ZP_FAC, 4u);
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
    const std::uint8_t fretopHigh = ReadZeroPageByte(add_u8(kFRETOP, 1u));

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
    WriteZeroPageByte(ApplesoftVariables::ZP_DIMFLG, 0u); // DIMFLG
    PTRGET3();
    return ReadZeroPageWord(ApplesoftVariables::ZP_VARPNT); // VARPNT
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

void SCREEN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SCREEN (inclusive) .. UNARY (exclusive)
    // Name normalization: none (assembler label SCREEN kept verbatim).

    constexpr std::uint8_t kFIRST = ApplesoftVariables::ZP_FIRST;

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

    constexpr std::uint8_t kARG = ApplesoftVariables::ZP_ARG;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;

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

    constexpr std::uint8_t kARG = ApplesoftVariables::ZP_ARG;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;

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

    constexpr std::uint8_t kCPRTYP = ApplesoftVariables::ZP_CPRTYP;
    constexpr std::uint16_t kARG = ApplesoftVariables::ZP_ARG;

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

    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;
    constexpr std::uint8_t kCPRTYP = ApplesoftVariables::ZP_CPRTYP;

    WriteZeroPageByte(kVALTYP, 0u);
    WriteZeroPageByte(kCPRTYP, static_cast<std::uint8_t>(ReadZeroPageByte(kCPRTYP) - 1u));

    FREFAC();
    (void)FRETMP(ReadZeroPageWord(ApplesoftVariables::ZP_DSCPTR));

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

    constexpr std::uint8_t kCPRMASK = ApplesoftVariables::ZP_CPRMASK;

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

    WriteZeroPageByte(ApplesoftVariables::ZP_DIMFLG, 1u); // DIMFLG non-zero when called from DIM.
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

    WriteZeroPageByte(ApplesoftVariables::ZP_VARNAM, CHRGOT()); // VARNAM low byte
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

    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP, 0u); // VALTYP
    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP_PLUS_1, 0u); // VALTYP+1
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
        WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP, 0xffu); // VALTYP string
        current = CHRGET();
    } else if (current == static_cast<std::uint8_t>('%')) {
        if ((ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) & 0x80u) != 0u) {
            BADNAM();
            return;
        }

        WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP_PLUS_1, 0x80u); // integer mode
        WriteZeroPageByte(ApplesoftVariables::ZP_VARNAM, static_cast<std::uint8_t>(ReadZeroPageByte(ApplesoftVariables::ZP_VARNAM) | 0x80u));
        secondChar = static_cast<std::uint8_t>(secondChar | 0x80u);
        current = CHRGET();
    }

    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_VARNAM, 1u), secondChar); // VARNAM+1

    const std::uint8_t subflg = ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG);
    if (subflg == 0u && current == static_cast<std::uint8_t>('(')) {
        ARRAY();
        return;
    }

    WriteZeroPageByte(ApplesoftVariables::ZP_SUBFLG, 0u); // clear SUBFLG
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

    WriteZeroPageByte(ApplesoftVariables::ZP_RESULT, kCZeroData[0]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 1u), kCZeroData[1]);
}

void MAKE_NEW_VARIABLE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKE_NEW_VARIABLE (inclusive) .. SET_VARPNT_AND_YA (exclusive)
    // Name normalization: none (assembler label MAKE_NEW_VARIABLE kept verbatim).

    const std::uint16_t arytab = ReadZeroPageWord(ApplesoftVariables::ZP_ARYTAB);
    WriteZeroPageWord(ApplesoftVariables::ZP_LOWTR, arytab); // LOWTR <- ARYTAB

    // TODO(asm-port): port BLTU movement of array block.
    SET_VARPNT_AND_YA();
}

void SET_VARPNT_AND_YA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SET_VARPNT_AND_YA (inclusive) .. GETARY (exclusive)
    // Name normalization: none (assembler label SET_VARPNT_AND_YA kept verbatim).

    const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    WriteZeroPageWord(ApplesoftVariables::ZP_VARPNT, lowtr.advanced(2u).address); // VARPNT
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

    const std::uint8_t numDim = ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM);
    const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    const std::uint16_t arypntOffset = static_cast<std::uint16_t>(numDim * 2u) + 5u;
    WriteZeroPageWord(ApplesoftVariables::ZP_ARYPNT, lowtr.advanced(arypntOffset).address);
}

void NEG32768() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEG32768 (inclusive) .. MAKINT (exclusive)
    // Name normalization: none (assembler label NEG32768 kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_RESULT, kNEG32768Data[0]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 1u), kNEG32768Data[1]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 2u), kNEG32768Data[2]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 3u), kNEG32768Data[3]);
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

    if ((ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN) & 0x80u) != 0u) {
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

    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC) < 0x90u) {
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

    if (ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) != 0u) {
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

    if (ReadZeroPageByte(ApplesoftVariables::ZP_DIMFLG) != 0u) {
        gJerErrorCode = ERR_REDIMD;
        JER();
        return;
    }

    if (ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) == 0u) {
        GETARY();
        FIND_ARRAY_ELEMENT();
    }
}

void MAKE_NEW_ARRAY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKE_NEW_ARRAY (inclusive) .. FIND_ARRAY_ELEMENT (exclusive)
    // Name normalization: none (assembler label MAKE_NEW_ARRAY kept verbatim).

    if (ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) != 0u) {
        ERROR(ERR_NODATA);
        return;
    }

    GETARY();

    // TODO(asm-port): complete dynamic allocation, descriptor population, and zeroing.
    if (ReadZeroPageByte(ApplesoftVariables::ZP_DIMFLG) == 0u) {
        FIND_ARRAY_ELEMENT();
    }
}

void FIND_ARRAY_ELEMENT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIND_ARRAY_ELEMENT (inclusive) .. FAE_1 (exclusive)
    // Name normalization: none (assembler label FIND_ARRAY_ELEMENT kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_NUMDIM, ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM)); // TODO(asm-port): fetch #dims from descriptor pointer.
    WriteZeroPageWord(ApplesoftVariables::ZP_STRNG2, 0u); // STRNG2 accumulator
    FAE_1();
}

void FAE_1() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FAE_1 (inclusive) .. GSE (exclusive)
    // Name normalization: none (assembler label FAE_1 kept verbatim).

    if (ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM) == 0u) {
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

std::uint16_t MULTIPLY_SUBS_1(std::uint8_t multiplierHigh);

std::uint16_t MULTIPLY_SUBSCRIPT(std::uint8_t descriptorOffset) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MULTIPLY_SUBSCRIPT (inclusive) .. MULTIPLY_SUBS_1 (exclusive)
    // Name normalization: none (assembler label MULTIPLY_SUBSCRIPT kept verbatim).
    // Load the 16-bit array-dimension multiplier from the LOWTR descriptor pointer.

    WriteZeroPageByte(ApplesoftVariables::ZP_INDEX, descriptorOffset);

    const ProgramPointer descriptor{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    WriteZeroPageByte(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_RESULT + 2u),
        descriptor.read(descriptorOffset));

    return MULTIPLY_SUBS_1(descriptor.read(static_cast<std::uint16_t>(descriptorOffset - 1u)));
}

std::uint16_t MULTIPLY_SUBS_1(std::uint8_t multiplierHigh) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MULTIPLY_SUBS_1 (inclusive) .. FRE (exclusive)
    // Name normalization: none (assembler label MULTIPLY_SUBS_1 kept verbatim).
    // STRNG2 is dual-use elsewhere, but in this slice it is the 16-bit multiplicand.

    WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_RESULT + 3u), multiplierHigh);
    WriteZeroPageByte(ApplesoftVariables::ZP_INDX, 16u);

    const std::uint16_t multiplier = ApplesoftVariables::makeWord(
        ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_RESULT + 2u)),
        multiplierHigh);

    std::uint16_t multiplicand = ReadZeroPageWord(ApplesoftVariables::ZP_STRNG2);
    std::uint16_t product = 0u;

    for (std::uint8_t bitsRemaining = 16u; bitsRemaining > 0u; --bitsRemaining) {
        if ((product & 0x8000u) != 0u) {
            GME();
            return product;
        }

        product = static_cast<std::uint16_t>(product << 1u);

        const bool nextBitSet = (multiplicand & 0x8000u) != 0u;
        multiplicand = static_cast<std::uint16_t>(multiplicand << 1u);
        WriteZeroPageWord(ApplesoftVariables::ZP_STRNG2, multiplicand);

        if (!nextBitSet) {
            continue;
        }

        if (product > static_cast<std::uint16_t>(0xffffu - multiplier)) {
            GME();
            return product;
        }

        product = static_cast<std::uint16_t>(product + multiplier);
    }

    WriteZeroPageByte(ApplesoftVariables::ZP_INDX, 0u);
    return product;
}

void FRE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FRE (inclusive) .. GIVAYF (exclusive)
    // Name normalization: none (assembler label FRE kept verbatim).
    // FRETOP and STREND are one logical address pair each; model the subtraction
    // as one 16-bit free-space computation before floating the signed result.

    if (ReadZeroPageByte(ApplesoftVariables::ZP_VALTYP) != 0u) {
        (void)FREFAC();
    }

    GARBAG();

    const std::uint16_t fretop = ReadZeroPageWord(ApplesoftVariables::ZP_FRETOP);
    const std::uint16_t strend = ReadZeroPageWord(ApplesoftVariables::ZP_STREND);
    const std::uint16_t freeSpace = static_cast<std::uint16_t>(fretop - strend);
    GIVAYF(static_cast<std::int16_t>(freeSpace));
}

namespace {

void GIVAYF(std::int16_t value) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GIVAYF (inclusive) .. POS (exclusive)
    // Name normalization: none (assembler label GIVAYF kept verbatim).
    // The A/Y pair is one signed 16-bit integer on entry; represent it as one
    // C++ value instead of split low/high byte locals.

    const std::uint16_t rawValue = static_cast<std::uint16_t>(value);

    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP, 0u);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 1u), ApplesoftVariables::lowByte(rawValue));
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 2u), ApplesoftVariables::highByte(rawValue));
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 3u), 0u);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 4u), 0u);

    FLOAT_1(0x90u);
}

} // namespace

void POS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: POS (inclusive) .. SNGFLT (exclusive)
    // Name normalization: none (assembler label POS kept verbatim).

    SNGFLT(ReadZeroPageByte(ApplesoftVariables::ZP_MON_CH));
}

void ERRDIR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ERRDIR (inclusive) .. DEF (exclusive)
    // Name normalization: none (assembler label ERRDIR kept verbatim).

    if (!IsDirectMode()) {
        return;
    }

    ERROR(ERR_ILLDIR);
}

void UNDFNC() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: UNDFNC (inclusive) .. DEF (exclusive)
    // Name normalization: none (assembler label UNDFNC kept verbatim).

    ERROR(ERR_UNDEFFUNC);
}

void CHKCLS() {
    // Check for ')' at current position
    SYNCHR(static_cast<std::uint8_t>(')'));
}

void CHKOPN() {
    // Check for '(' at current position
    SYNCHR(static_cast<std::uint8_t>('('));
}

void DEF() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DEF (inclusive) .. FNC_ (exclusive)
    // Name normalization: none (assembler label DEF kept verbatim).
    //
    // "DEF" STATEMENT
    // Parse: DEF FN name (arg) = expression
    // Stacks VARPNT, TXTPTR, and 5 bytes of FAC, then jumps to FNCDATA to store.

    // Parse "FN name"
    FNC_();
    
    // Error if in direct mode
    ERRDIR();
    
    // Require "("
    CHKOPN();
    
    // Set SUBFLG to flag DEF context for PTRGET
    constexpr std::uint8_t kSUBFLG = ApplesoftVariables::ZP_SUBFLG;
    WriteZeroPageByte(kSUBFLG, 0x80u);
    
    // Get pointer to argument variable
    PTRGET();
    
    // Argument must be numeric
    CHKNUM();
    
    // Require ")"
    CHKCLS();
    
    // Require "=" and advance past it
    SYNCHR(static_cast<std::uint8_t>(0xd0u));  // TOKEN_EQUAL = 0xd0
    
    // Stack the argument variable pointer (VARPNT)
    constexpr std::uint8_t kVARPNT = ApplesoftVariables::ZP_VARPNT;
    WriteZeroPageByte(kVARPNT + 1u, ReadZeroPageByte(kVARPNT + 1u));
    WriteZeroPageByte(kVARPNT, ReadZeroPageByte(kVARPNT));
    
    // Stack the text pointer (TXTPTR)
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    const std::uint16_t txtPtr = ReadZeroPageWord(kTXTPTR);
    
    // Scan to next statement
    DATA();
    
    // Fall through to FNCDATA to store 5-byte FAC
}

void FNC_() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNC_ (inclusive) .. FUNCT (exclusive)
    // Name normalization: none (assembler label FNC_ kept verbatim).
    //
    // Common routine for "DEF" and "FN" - parse FN token and function name.
    // Requires "FN" token, sets SUBFLG high bit, parses name to FNCNAM.

    // Require "FN" token
    SYNCHR(static_cast<std::uint8_t>(0xc2u));  // TOKEN_FN = 0xc2
    
    // Set high bit in SUBFLG to signal this is from DEF/FN context
    constexpr std::uint8_t kSUBFLG = ApplesoftVariables::ZP_SUBFLG;
    const std::uint8_t subflg = ReadZeroPageByte(kSUBFLG);
    WriteZeroPageByte(kSUBFLG, subflg | 0x80u);
    
    // Parse function name via PTRGET3
    PTRGET3();
    
    // PTRGET3 leaves A=name_lo, Y=name_hi
    // Store to FNCNAM
    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    const std::uint8_t nameA = ReadZeroPageByte(ApplesoftVariables::ZP_STRNG1);  // Temp storage from PTRGET3
    const std::uint8_t nameY = ReadZeroPageByte(ApplesoftVariables::ZP_STRNG1 + 1u);
    WriteZeroPageByte(kFNCNAM, nameA);
    WriteZeroPageByte(kFNCNAM + 1u, nameY);
    
    // Jump to CHKNUM to validate numeric type
    CHKNUM();
}

void FUNCT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FUNCT (inclusive) .. FNCDATA (exclusive)
    // Name normalization: none (assembler label FUNCT kept verbatim).
    //
    // "FN" FUNCTION CALL - invoke user-defined function
    // Parse FN name, save old argument value, evaluate expression with new value,
    // restore old value via FNCDATA.

    // Parse "FN name"
    FNC_();
    
    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    constexpr std::uint8_t kVARPNT = ApplesoftVariables::ZP_VARPNT;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    
    // Stack function address for nested FN calls
    const std::uint16_t fncAddr = ReadZeroPageWord(kFNCNAM);
    
    // Parse "(expression)" and evaluate
    PARCHK();
    
    // Result in FAC - must be numeric
    CHKNUM();
    
    // Get argument variable pointer from FNCNAM+2,+3
    const std::uint16_t argVarAddr = fncAddr + 2u;
    WriteZeroPageWord(kVARPNT, argVarAddr);
    
    // Save old value of argument variable (5 bytes) to stack
    for (std::uint8_t i = 4u; i >= 0u && i <= 4u; --i) {
        const std::uint8_t byte = variables_const().pointer(argVarAddr).read(i);
        // TODO(asm-port): push byte to stack
    }
    
    // Store FAC to argument variable (rounded)
    STORE_FACDB_YX_ROUNDED();
    
    // Save current TXTPTR
    const std::uint16_t savedTxtPtr = ReadZeroPageWord(kTXTPTR);
    
    // Load function definition address from FNCNAM+0,+1
    const std::uint16_t defAddr = fncAddr;  // Will read via pointer arithmetic
    WriteZeroPageWord(kTXTPTR, defAddr);
    
    // Stack argument variable address for later
    
    // Evaluate function expression
    FRMNUM();
    
    // Validate at ":" or EOL
    if (CHRGOT() != 0u && CHRGOT() != static_cast<std::uint8_t>(':')) {
        SYNERR();
    }
    
    // Restore TXTPTR
    WriteZeroPageWord(kTXTPTR, savedTxtPtr);
    
    // Stack now contains 5 saved bytes - fall through to FNCDATA to restore
}

void FNCDATA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNCDATA (inclusive) .. STR (exclusive)
    // Name normalization: none (assembler label FNCDATA kept verbatim).
    //
    // STORE FIVE BYTES FROM STACK AT (FNCNAM)
    // Pop 5 stack bytes and store to (FNCNAM),Y with Y incrementing.

    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    const std::uint16_t fncnampnt = ReadZeroPageWord(kFNCNAM);
    
    // Loop 5 times: pop stack and store
    for (std::uint8_t y = 0u; y < 5u; ++y) {
        // TODO(asm-port): pop stack byte
        // Store to (fncnampnt + y)
    }
}

void PARCHK() {
    // TODO(asm-port): parse "(expression)" - validate open paren, evaluate, validate close.
}

void STORE_FACDB_YX_ROUNDED() {
    // TODO(asm-port): store 5-byte FAC to address in Y,X with rounding.
}

void SETFOR() {
    // TODO(asm-port): port SETFOR label.
}

void HANDLERR() {
    // TODO(asm-port): transfer control to the ON ERR handler.
}

bool IsOnErr() {
    // TODO(asm-port): inspect the Applesoft ERRFLG state.
    return false;
}

bool IsDirectMode() {
    return ReadZeroPageByte(add_u8(ApplesoftVariables::ZP_CURLIN, 1u)) == 0xffu;
}

} // namespace applesoft::asm_port
