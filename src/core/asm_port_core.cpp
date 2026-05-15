#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_error_handling.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_reason.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_mathtbl.hpp"
#include "core/asm_port_unfnc.hpp"
#include "core/asm_port_stack.hpp"
#include "core/io_ports.hpp"
#include "platform/asm_port_outdo.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace applesoft::asm_port {

void SYNERR();
extern std::uint8_t gJerErrorCode;
void MON_RESET2();
void MON_REGDSP();
constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
    return static_cast<std::uint8_t>(lhs + rhs);
}
void GETARY();
void GETARY2();
void FIND_ARRAY_ELEMENT();
std::uint16_t MULTIPLY_SUBS_1(std::uint8_t multiplierHigh);
std::uint16_t MULTIPLY_SUBSCRIPT(std::uint8_t descriptorOffset);
void GIVAYF(std::int16_t value);
void SNGFLT(std::uint8_t value);
void FALSE();
void TRUE();
void ANDOP();
bool ISLETC();
void NAMOK();
void NXDIM();
void ARRAY();
void MI1();
void MI2();
void CMPDONE();
void NUMCMP();
void PLOTFNS();
void SYNCHR(std::uint8_t expected);
void CHKNUM();
void FRMNUM();
void CHKOPN();
void CHKCLS();
std::uint16_t PTRGET();
void DATA();
void FRMEVL();
void STRCMP();
void PARCHK();
void STORE_FACDB_YX_ROUNDED();
void ERRDIR();
std::int8_t FCOMP(std::uint16_t argAddress);
void FLOAT();
void FLOAT_1(std::uint8_t exponent);
bool CHKVAL(std::uint8_t savedValTyp);
std::uint8_t MON_SCRN(std::uint8_t row, std::uint8_t column);
std::uint8_t FREFAC();
std::uint8_t FRETMP(std::uint16_t descriptorAddress);
void GARBAG();
std::int8_t CompareArgAndFacStrings();
void GOTO();
void NEWSTT();
void PRINT_ERROR_LINNUM();
void MON_INPORT(std::uint8_t slot);
void MON_OUTPORT(std::uint8_t slot);
void MON_SETTXT();
void MON_HOME();
void CAT();
void CHKSTR();
void CHKCOM();
std::uint8_t GETBYT();
extern std::int8_t gNumericCompareResult;
extern bool gNumericCompareCarry;
extern std::uint8_t gFloatInput;
extern std::uint8_t gPendingErrorCode;

void SetTextPointer(std::uint16_t address) {
    variables().writeWord(ApplesoftVariables::ZP_TXTPTR, address);
}

void ClearErrFlag() {
    variables().writeByte(ApplesoftVariables::ZP_ERRFLG, 0);
}

void MarkDirectMode() {
    variables().writeByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_CURLIN + 1u), 0xffu);
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

std::uint8_t ReadProgramByte(std::uint16_t address) {
    // Program text lives in the same flat address space as zero-page variables;
    // ApplesoftVariables::readByte handles all address regions.
    return variables_const().readByte(address);
}

void WriteProgramByte(std::uint16_t address, std::uint8_t value) {
    variables().writeByte(address, value);
}

bool IsOnErr() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ERROR (inclusive) .. L_ERROR_1 (exclusive)
    // Name normalization: helper name chosen for the inline ERROR predicate.
    // `bit ERRFLG` + `bpl` means ON ERR is active when ERRFLG bit 7 is set.
    return (ReadZeroPageByte(ApplesoftVariables::ZP_ERRFLG) & 0x80u) != 0u;
}

bool IsDirectMode() {
    return ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_CURLIN + 1u)) == 0xffu;
}

void NORMAL();
void CRDO();
void SCRTCH();
void RESTART();

constexpr std::array<std::uint8_t, 29> kGenericCHRGETImage = {
    0xe6, 0xb8, 0xd0, 0x02, 0xe6, 0xb9, 0xad, 0x60, 0xea, 0xc9,
    0x3a, 0xb0, 0x0a, 0xc9, 0x20, 0xf0, 0xef, 0x38, 0xe9, 0x30,
    0x38, 0xe9, 0xd0, 0x60, 0x80, 0x4f, 0xc7, 0x52, 0x58,
};

void COLD_START();

void GENERIC_END() {
    // Label-only range in ROM: this address immediately falls into COLD_START.
    COLD_START();
}

// Forward declaration: MON_COUT is defined in asm_port_outdo.cpp.
void MON_COUT(std::uint8_t value);

namespace {

void MON_SETNORM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: SETNORM (inclusive) .. SETKBD (exclusive)
    // Name normalization: SETNORM -> MON_SETNORM (monitor label gets MON_ prefix).

    constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
    WriteZeroPageByte(kMON_INVFLG, 0xffu);
}

void MON_INIT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
    // Labels: INIT (inclusive) .. SETTXT (exclusive)
    // Name normalization: INIT -> MON_INIT (monitor label gets MON_ prefix).
    //
    // Falls through into SETTXT in ROM; modeled by explicit call.

    WriteZeroPageByte(ApplesoftVariables::ZP_MON_STATUS, 0u);
    (void)variables_const().readByte(IOPorts::ADDR_SW_LORES);
    (void)variables_const().readByte(IOPorts::ADDR_SW_LOWSCR);

    MON_SETTXT();
}

void MON_SETVID() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: SETVID (inclusive) .. OUTPORT (exclusive)
    // Name normalization: SETVID -> MON_SETVID (monitor label gets MON_ prefix).

    MON_OUTPORT(0u);
}

void MON_SETKBD() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: SETKBD (inclusive) .. INPORT (exclusive)
    // Name normalization: SETKBD -> MON_SETKBD (monitor label gets MON_ prefix).

    MON_INPORT(0u);
}

void MON_BELL_impl() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: BELL (inclusive) .. RESTORE (exclusive)
    // Name normalization: BELL -> MON_BELL (monitor label gets MON_ prefix).

    constexpr std::uint8_t kBellChar = 0x87u;
    MON_COUT(kBellChar);
}

void MON_LFB60() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/math.o65.lst
    // Labels: LFB60 (inclusive) .. LFB78 (exclusive)
    // Name normalization: LFB60 -> MON_LFB60 (monitor label gets MON_ prefix).
    //
    // Clears the monitor text window, then writes the 9-byte "<APPLE ]["
    // banner to screen memory at $040E..$0416.

    static constexpr std::array<std::uint8_t, 9> kLFB08 = {
        0x3cu,
        static_cast<std::uint8_t>('A' | 0x80u),
        static_cast<std::uint8_t>('P' | 0x80u),
        static_cast<std::uint8_t>('P' | 0x80u),
        static_cast<std::uint8_t>('L' | 0x80u),
        static_cast<std::uint8_t>('E' | 0x80u),
        static_cast<std::uint8_t>(' ' | 0x80u),
        static_cast<std::uint8_t>(']' | 0x80u),
        static_cast<std::uint8_t>('[' | 0x80u),
    };
    constexpr std::uint16_t kBannerAddress = 0x040eu;

    MON_HOME();

    for (std::uint8_t y = 8u;; --y) {
        // LFB65 copies the banner bytes in descending index order.
        variables().writeByte(static_cast<std::uint16_t>(kBannerAddress + y), kLFB08[y]);
        if (y == 0u) {
            break;
        }
    }
}

void MON_CROUT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: CROUT (inclusive) .. PRA1 (exclusive)
    // Name normalization: CROUT -> MON_CROUT (monitor label gets MON_ prefix).
    //
    // `lda #$8d` + `bne COUT` is an unconditional transfer to COUT with CR.
    constexpr std::uint8_t kCarriageReturn = 0x8du;
    MON_COUT(kCarriageReturn);
}

void MON_PRBYTE(std::uint8_t value) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: PRBYTE (inclusive) .. COUT (exclusive)
    // Name normalization: PRBYTE -> MON_PRBYTE (monitor label gets MON_ prefix).

    const auto emitHexNibble = [](std::uint8_t nibble) -> std::uint8_t {
        // PRHEX/PRHEXZ: ORA #"0"|$80; if >= ':' add 6 to reach 'A'..'F'.
        std::uint8_t ch = static_cast<std::uint8_t>((nibble & 0x0fu) | 0xb0u);
        if (ch >= 0xbau) {
            ch = static_cast<std::uint8_t>(ch + 0x06u);
        }
        return ch;
    };

    MON_COUT(emitHexNibble(static_cast<std::uint8_t>(value >> 4u)));
    MON_COUT(emitHexNibble(value));
}

bool MON_JumpByAddress(std::uint16_t target) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/debug.o65.lst
    // Labels: LFAA3 (inclusive) .. LFAA6 (exclusive), plus LFAC7 tail-jump use
    // Name normalization: none (helper name kept for C++ indirect-jump modeling).
    //
    // Emulates monitor reset-time indirect transfers used by `jmp ($03f2)` and
    // `jmp ($00)` in RESET2's warm/slot scan flow.

    // Standard RESET2 warm vector target: transfer to Applesoft cold start.
    if (target == 0xe000u) {
        COLD_START();
        return true;
    }

    // RESET2 slot scan (`jmp ($00)`) jumps to Cx00 for x in [1,7] after a ROM
    // signature match. Slot-ROM runtime is not ported yet, so preserve
    // non-returning transfer semantics by falling back to BASIC cold start.
    const bool isSlotRomJump = ((target & 0x00ffu) == 0x0000u) && (target >= 0xc100u) && (target <= 0xc700u);
    if (isSlotRomJump) {
        COLD_START();
        return true;
    }

    // Unknown monitor vector target: keep startup progress deterministic.
    COLD_START();
    return true;
}

} // namespace

void MON_BELL() {
    MON_BELL_impl();
}

void MON_RESET2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/debug.o65.lst
    // Labels: RESET2 (inclusive) .. REGDSP (exclusive)
    // Name normalization: RESET2 -> MON_RESET2 (monitor label gets MON_ prefix).
    //
    // One-sentence behavior summary: initialize monitor I/O modes, validate
    // bootstrap sentinel bytes, then either jump through warm vectors or scan
    // ROM pages for a signature and jump to the matched page entrypoint.

    constexpr std::uint16_t kWarmVector = ApplesoftVariables::ADDR_MON_DEBUG_WARM_VECTOR;
    constexpr std::uint16_t kSentinel1 = ApplesoftVariables::ADDR_MON_DEBUG_SENTINEL_1;
    constexpr std::uint16_t kSentinel2 = ApplesoftVariables::ADDR_MON_DEBUG_SENTINEL_2;
    constexpr std::uint16_t kBootstrap = ApplesoftVariables::ADDR_MON_DEBUG_BOOTSTRAP;
    constexpr std::uint16_t kScanPageMirror = ApplesoftVariables::ADDR_MON_DEBUG_SCAN_PAGE;

    constexpr std::uint16_t kJumpToBasic = 0xe000u;
    constexpr std::uint8_t kWarmEnable = 0x03u;
    constexpr std::uint8_t kSentinelCheckMask = 0xa5u;
    constexpr std::uint8_t kSentinelExpected = 0xe0u;

    // Signature bytes used by LFAA6/LFABA/LFAC7 scan path.
    constexpr std::array<std::uint8_t, 7> kLFB01 = {0x45u, 0x20u, 0xffu, 0x00u, 0xffu, 0x03u, 0xffu};
    constexpr std::array<std::uint8_t, 6> kLFAFC = {0x00u, 0x00u, 0xfau, 0x59u, 0xe0u, 0x00u};

    // RESET2 prologue.
    MON_SETNORM();
    MON_INIT();
    MON_SETVID();
    MON_SETKBD();

    // Hardware-side effect reads routed through the I/O companion class.
    (void)variables_const().readByte(IOPorts::ADDR_SW_AN0);
    (void)variables_const().readByte(IOPorts::ADDR_SW_AN1);
    (void)variables_const().readByte(IOPorts::ADDR_SW_AN2);
    (void)variables_const().readByte(IOPorts::ADDR_SW_AN3);
    (void)variables_const().readByte(IOPorts::ADDR_ROM_SIGNATURE);
    (void)variables_const().readByte(IOPorts::ADDR_KEYBOARD_STROBE);

    MON_BELL_impl();

    const std::uint8_t sentinel1 = variables_const().readByte(kSentinel1);
    const std::uint8_t sentinel2 = variables_const().readByte(kSentinel2);
    const std::uint8_t warmState = variables_const().readByte(kWarmVector);

    if (static_cast<std::uint8_t>(sentinel1 ^ kSentinelCheckMask) == sentinel2 &&
        warmState == 0u &&
        sentinel1 == kSentinelExpected) {
        variables().writeByte(kWarmVector, kWarmEnable);
        (void)MON_JumpByAddress(kJumpToBasic);
        return;
    }

    // LFAA3 path: jump through warm vector ($03F2/$03F3).
    if (warmState != 0u) {
        const std::uint16_t warmTarget = variables_const().readWord(kWarmVector);
        (void)MON_JumpByAddress(warmTarget);
        return;
    }

    // LFAA6 path: install bootstrap bytes, then scan descending pages.
    MON_LFB60();

    // LFAAB loop copies indices 5..1 from LFAFC to $03F4..$03F0.
    for (std::uint8_t x = 5u; x != 0u; --x) {
        variables().writeByte(static_cast<std::uint16_t>(kBootstrap + x), kLFAFC[x]);
    }

    // Unified pointer for $00/$01 pair used by LFABA/LFAC7 and jmp($00).
    std::uint16_t scanPtr = 0xc800u;
    while (true) {
        scanPtr = static_cast<std::uint16_t>(scanPtr - 0x0100u);

        const std::uint8_t page = ApplesoftVariables::highByte(scanPtr);
        if (page == 0xc0u) {
            variables().writeByte(kWarmVector, kWarmEnable);
            (void)MON_JumpByAddress(kJumpToBasic);
            return;
        }

        variables().writeByte(kScanPageMirror, page);

        bool match = true;
        for (std::int8_t y = 7; y >= 0; y -= 2) {
            const std::uint8_t lhs = variables_const().readByte(static_cast<std::uint16_t>(scanPtr + static_cast<std::uint16_t>(y)));
            const std::uint8_t rhs = kLFB01[static_cast<std::size_t>(y)];
            if (lhs != rhs) {
                match = false;
                break;
            }
        }

        if (!match) {
            continue;
        }

        (void)MON_JumpByAddress(scanPtr);
        return;
    }
}

void MON_REGDSP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/debug.o65.lst
    // Labels: REGDSP (inclusive) .. LFB01 (exclusive)
    // Name normalization: REGDSP -> MON_REGDSP (monitor label gets MON_ prefix).
    //
    // Prints monitor register labels and the saved register byte values.

    constexpr std::uint8_t kMON_A4 = ApplesoftVariables::ZP_MON_A4;
    constexpr std::uint8_t kRegBase = ApplesoftVariables::ZP_MON_DEBUG_REG_A;
    constexpr std::uint8_t kSpace = static_cast<std::uint8_t>(' ' | 0x80u);
    constexpr std::uint8_t kEquals = static_cast<std::uint8_t>('=' | 0x80u);
    constexpr std::array<std::uint8_t, 5> kRTBL = {
        static_cast<std::uint8_t>('A' | 0x80u),
        static_cast<std::uint8_t>('X' | 0x80u),
        static_cast<std::uint8_t>('Y' | 0x80u),
        static_cast<std::uint8_t>('P' | 0x80u),
        static_cast<std::uint8_t>('S' | 0x80u),
    };

    MON_CROUT();

    // Unified pointer candidate from RGDSP1: $40/$41 points at the saved-register block.
    const std::uint16_t regPointer = kRegBase;
    WriteZeroPageWord(kMON_A4, regPointer);

    for (std::size_t i = 0; i < kRTBL.size(); ++i) {
        MON_COUT(kSpace);
        MON_COUT(kRTBL[i]);
        MON_COUT(kEquals);
        const std::uint8_t regValue = variables_const().readByte(static_cast<std::uint16_t>(regPointer + i));
        MON_PRBYTE(regValue);
    }
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
    theStack().setStackPointer(0xfbu);

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
    theStack().pushByte(0u);
    WriteZeroPageByte(ApplesoftVariables::ZP_DSCLEN, 3u);

    CRDO();

    variables().writeByte(ApplesoftVariables::ADDR_INPUT_BUFFER_MINUS_3, 1u);
    variables().writeByte(ApplesoftVariables::ADDR_INPUT_BUFFER_MINUS_4, 1u);
    WriteZeroPageByte(ApplesoftVariables::ZP_TEMPPT, ApplesoftVariables::ZP_TEMPST);

    // Unified RAM probe pointer lifted from LINNUM low/high carry-chain in ROM.
    std::uint16_t ramProbe = kProgramStart;
    while ((ramProbe & 0xff00u) < IOPorts::ADDR_BASE) {
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

void BADNAM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BADNAM (inclusive) .. NAMOK (exclusive)
    // Name normalization: none (assembler label BADNAM kept verbatim).

    SYNERR();
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

void GME() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GME (inclusive) .. MULTIPLY_SUBSCRIPT (exclusive)
    // Name normalization: none (assembler label GME kept verbatim).

    (void)MEMERR();
}

void SUBERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SUBERR (inclusive) .. IQERR (exclusive)
    // Name normalization: none (assembler label SUBERR kept verbatim).

    gJerErrorCode = ERR_BADSUBS;
    JER();
}

void GSE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GSE (inclusive) .. GME (exclusive)
    // Name normalization: none (assembler label GSE kept verbatim).

    SUBERR();
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

void C_ZERO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: C_ZERO (inclusive) .. MAKE_NEW_VARIABLE (exclusive)
    // Name normalization: none (assembler label C_ZERO kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_RESULT, kCZeroData[0]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 1u), kCZeroData[1]);
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
    // Labels: MAKE_NEW_ARRAY (T:11b8, inclusive) .. FIND_ARRAY_ELEMENT (T:124b, exclusive)
    // Name normalization: none (assembler label MAKE_NEW_ARRAY kept verbatim).
    //
    // Creates a new array entry at LOWTR: writes header (name, size, numdim, dim extents),
    // allocates element space, zeroes it, and stores the array byte-size in the descriptor.
    // Falls through to FIND_ARRAY_ELEMENT unless called from DIM (DIMFLG != 0).

    // T:11b8 – lda SUBFLG; bne ERR_NODATA
    if (ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) != 0u) {
        ERROR(ERR_NODATA);
        return;
    }

    // T:11c1 – jsr GETARY: sets ARYPNT = LOWTR + 5 + 2*NUMDIM (first element address)
    GETARY();

    // T:11c4 – jsr REASON: ensure array header fits below FRETOP
    {
        const std::uint16_t arypnt = ReadZeroPageWord(ApplesoftVariables::ZP_ARYPNT);
        const std::uint16_t fretop = ReadZeroPageWord(ApplesoftVariables::ZP_FRETOP);
        REASONState rs{};
        rs.a = static_cast<std::uint8_t>(arypnt & 0xffu);
        rs.y = static_cast<std::uint8_t>(arypnt >> 8u);
        rs.fretopLo = static_cast<std::uint8_t>(fretop & 0xffu);
        rs.fretopHi = static_cast<std::uint8_t>(fretop >> 8u);
        if (!REASON(rs).ok) return;
    }

    // T:11c7 – lda #0; tay; sta STRNG2+1; ldx #5  (seed element-size accumulator)
    WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_STRNG2 + 1u), 0u);
    std::uint8_t elemSize = 5u;   // X in asm: float default

    // T:11cd – Y=0: write VARNAM byte to descriptor[0]; bit 7 → integer, dex
    const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    const std::uint8_t varnam0 = ReadZeroPageByte(ApplesoftVariables::ZP_VARNAM);
    lowtr.write(varnam0, 0u);
    if ((varnam0 & 0x80u) != 0u) { --elemSize; }   // integer: 5→4

    // T:11d5 – Y=1: write VARNAM+1 to descriptor[1]; bit 7 → integer/string, dex dex
    const std::uint8_t varnam1 =
        ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_VARNAM + 1u));
    lowtr.write(varnam1, 1u);
    if ((varnam1 & 0x80u) != 0u) { elemSize -= 2u; }  // integer→2, string→3, float→5

    // T:11de – stx STRNG2: seed running product with element size
    WriteZeroPageByte(ApplesoftVariables::ZP_STRNG2, elemSize);

    // T:11e0 – Y=4: write NUMDIM to descriptor[4]  (three iny's advance Y past size slots)
    const std::uint8_t numDim = ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM);
    lowtr.write(numDim, 4u);

    // T:11e7 – dimension loop: write each dim to descriptor and accumulate total bytes.
    // Y starts at 4 (NUMDIM slot); each iteration writes 2 bytes and advances Y by 2.
    std::uint8_t descriptorY  = 4u;
    std::uint8_t remainingDims = numDim;
    while (remainingDims != 0u) {
        // T:11e7 – ldx #11; lda #0; bit DIMFLG; bvc use_default
        std::uint8_t dimLo = 11u;   // default: 11 elements (indices 0..10)
        std::uint8_t dimHi = 0u;

        // bit DIMFLG: V flag (bit 6) set → explicit dimension was pushed on 6502 stack.
        if ((ReadZeroPageByte(ApplesoftVariables::ZP_DIMFLG) & 0x40u) != 0u) {
            // T:11ef – pla (raw dim_lo); clc; adc #1; tax
            const std::uint8_t rawLo = theStack().popByte();
            const std::uint16_t lo16 = static_cast<std::uint16_t>(rawLo) + 1u;
            dimLo = static_cast<std::uint8_t>(lo16 & 0xffu);
            const std::uint8_t carry = static_cast<std::uint8_t>(lo16 >> 8u);
            // T:11f4 – pla (dim_hi); adc #0 (carry from low add)
            dimHi = static_cast<std::uint8_t>(theStack().popByte() + carry);
        }

        // T:11f7 – iny; sta (LOWTR),Y [dim_hi]; iny; txa; sta (LOWTR),Y [dim_lo]
        ++descriptorY;
        lowtr.write(dimHi, descriptorY);
        ++descriptorY;
        lowtr.write(dimLo, descriptorY);

        // T:11fe – jsr MULTIPLY_SUBSCRIPT: running_product = STRNG2 * dim_count
        // STRNG2 must be set before the call (done above / updated each iteration).
        const std::uint16_t product = MULTIPLY_SUBSCRIPT(descriptorY);
        WriteZeroPageByte(ApplesoftVariables::ZP_STRNG2,
                          static_cast<std::uint8_t>(product & 0xffu));
        WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_STRNG2 + 1u),
                          static_cast<std::uint8_t>(product >> 8u));

        // T:1205 – ldy INDEX: restore Y (MULTIPLY_SUBSCRIPT saved descriptorY there)
        descriptorY = ReadZeroPageByte(ApplesoftVariables::ZP_INDEX);

        // T:1207 – dec NUMDIM; bne loop
        --remainingDims;
        WriteZeroPageByte(ApplesoftVariables::ZP_NUMDIM, remainingDims);
    }

    // T:120b – compute endAddr = ARYPNT + totalBytes; both overflow checks collapse to one.
    const std::uint16_t totalBytes = ReadZeroPageWord(ApplesoftVariables::ZP_STRNG2);
    const std::uint16_t arypnt    = ReadZeroPageWord(ApplesoftVariables::ZP_ARYPNT);
    const std::uint32_t endAddr32 =
        static_cast<std::uint32_t>(arypnt) + static_cast<std::uint32_t>(totalBytes);
    if (endAddr32 > 0xffffu) {
        // Covers asm: bcs GME (high-byte overflow) and iny;beq GME (wrap-to-zero).
        GME();
        return;
    }
    const std::uint16_t endAddr = static_cast<std::uint16_t>(endAddr32);

    // T:121a – jsr REASON: ensure end address fits below FRETOP; result → new STREND.
    std::uint16_t strend;
    {
        const std::uint16_t fretop = ReadZeroPageWord(ApplesoftVariables::ZP_FRETOP);
        REASONState rs{};
        rs.a      = static_cast<std::uint8_t>(endAddr & 0xffu);
        rs.y      = static_cast<std::uint8_t>(endAddr >> 8u);
        rs.fretopLo = static_cast<std::uint8_t>(fretop & 0xffu);
        rs.fretopHi = static_cast<std::uint8_t>(fretop >> 8u);
        const REASONResult rr = REASON(rs);
        if (!rr.ok) return;
        strend = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(rr.y) << 8u | static_cast<std::uint16_t>(rr.a));
    }

    // T:121d – sta STREND; sty STREND+1
    WriteZeroPageWord(ApplesoftVariables::ZP_STREND, strend);

    // T:1221–1232 – zero element region [ARYPNT .. STREND)
    // Asm uses a page-by-page backward sweep; C++ equivalent byte loop has same effect.
    for (std::uint16_t addr = arypnt; addr != strend;
         addr = static_cast<std::uint16_t>(addr + 1u)) {
        WriteProgramByte(addr, 0u);
    }

    // T:1234 – inc ARYPNT+1: after zeroing the asm modified ARYPNT+1 for the loop;
    // restoring ARYPNT to elemStart is equivalent here.
    WriteZeroPageWord(ApplesoftVariables::ZP_ARYPNT, arypnt);

    // T:1236–1244 – sec; STREND - LOWTR → descriptor[2..3] (offset to next array entry)
    const std::uint16_t lowtrAddr = ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR);
    const std::uint16_t arraySize = static_cast<std::uint16_t>(strend - lowtrAddr);
    lowtr.write(static_cast<std::uint8_t>(arraySize & 0xffu), 2u);
    lowtr.write(static_cast<std::uint8_t>(arraySize >> 8u),  3u);

    // T:1246 – lda DIMFLG; bne RTS_9: DIM statement is done; otherwise find element.
    if (ReadZeroPageByte(ApplesoftVariables::ZP_DIMFLG) != 0u) {
        return;
    }
    // T:124a – iny (Y=4 for FIND_ARRAY_ELEMENT descriptor[4] = NUMDIM); fall through.
    FIND_ARRAY_ELEMENT();
}

void FIND_ARRAY_ELEMENT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIND_ARRAY_ELEMENT (inclusive) .. FAE_1 (exclusive)
    // Name normalization: none (assembler label FIND_ARRAY_ELEMENT kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_NUMDIM, ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM)); // TODO(asm-port): fetch #dims from descriptor pointer.
    WriteZeroPageWord(ApplesoftVariables::ZP_STRNG2, 0u); // STRNG2 accumulator
    FAE_1();
}

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


void SNGFLT(std::uint8_t value) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SNGFLT (inclusive) .. ERRDIR (exclusive)
    // Name normalization: none (assembler label SNGFLT kept verbatim).

    GIVAYF(static_cast<std::int16_t>(value));
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


void SET_VARPNT_AND_YA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SET_VARPNT_AND_YA (inclusive) .. GETARY (exclusive)
    // Name normalization: none (assembler label SET_VARPNT_AND_YA kept verbatim).

    const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    WriteZeroPageWord(ApplesoftVariables::ZP_VARPNT, lowtr.advanced(2u).address); // VARPNT
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


void NAME_NOT_FOUND() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NAME_NOT_FOUND (inclusive) .. C_ZERO (exclusive)
    // Name normalization: none (assembler label NAME_NOT_FOUND kept verbatim).

    // TODO(asm-port): preserve FRM_VARIABLE_CALL return-address special case.
    MAKE_NEW_VARIABLE();
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


void PTRGET2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET2 (inclusive) .. PTRGET3 (exclusive)
    // Name normalization: none (assembler label PTRGET2 kept verbatim).

    PTRGET3();
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


void NXDIM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NXDIM (inclusive) .. DIM (exclusive)
    // Name normalization: none (assembler label NXDIM kept verbatim).

    CHKCOM();
    DIM();
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


void SetPendingErrorCode(std::uint8_t errorCode) {
    gPendingErrorCode = errorCode;
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


void HANDLERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HANDLERR (inclusive) .. RESUME (exclusive)
    // Name normalization: none (assembler label HANDLERR kept verbatim).
    constexpr std::uint8_t kERRNUM = ApplesoftVariables::ZP_ERRNUM;
    constexpr std::uint8_t kERRLIN = ApplesoftVariables::ZP_ERRLIN;
    constexpr std::uint8_t kERRPOS = ApplesoftVariables::ZP_ERRPOS;
    constexpr std::uint8_t kERRSTK = ApplesoftVariables::ZP_ERRSTK;
    constexpr std::uint8_t kTXTPSV = ApplesoftVariables::ZP_TXTPSV;
    constexpr std::uint8_t kCURLSV = ApplesoftVariables::ZP_CURLSV;
    constexpr std::uint8_t kREMSTK = ApplesoftVariables::ZP_REMSTK;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    WriteZeroPageByte(kERRNUM, gPendingErrorCode);
    WriteZeroPageByte(kERRSTK, ReadZeroPageByte(kREMSTK));

    WriteZeroPageWord(kERRLIN, ReadZeroPageWord(kCURLIN));
    WriteZeroPageWord(kERRPOS, ReadZeroPageWord(kOLDTEXT));

    WriteZeroPageWord(kTXTPTR, ReadZeroPageWord(kTXTPSV));
    WriteZeroPageWord(kCURLIN, ReadZeroPageWord(kCURLSV));

    CHRGOT();
    GOTO();
    NEWSTT();
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
    //
    // Dispatch a built-in unary function or string function identified by
    // the current token.  String functions (LEFT$, RIGHT$, MID$) parse two
    // arguments; all other functions parse one argument via PARCHK.

    constexpr std::uint8_t kTOKEN_SCRN    = 0xd7u;
    constexpr std::uint8_t kTOKEN_SGN     = 0xd2u;
    constexpr std::uint8_t kTOKEN_LEFTSTR = 0xe8u;

    const std::uint8_t token = CHRGOT();

    if (token == kTOKEN_SCRN) {
        // ROM branches back to SCREEN for SCRN(.
        SCREEN();
        return;
    }

    // ASL: double the token (8-bit) as the UNFNC table index key.
    const std::uint8_t doubled = static_cast<std::uint8_t>(token << 1);

    CHRGET();

    // cpx #<(TOKEN_LEFTSTR*2-1) = $cf
    // bcc L_UNARY_1: doubled < $cf means numeric or CHR$ function.
    if (doubled >= static_cast<std::uint8_t>((kTOKEN_LEFTSTR * 2u) - 1u)) {
        // String function path (LEFT$, RIGHT$, MID$).
        CHKOPN();   // require '('
        FRMEVL();   // evaluate string argument into FAC
        CHKCOM();   // require ','
        CHKSTR();   // ensure FAC holds a string value

        // Save the string descriptor address (VPNT = FAC+3/FAC+4) into
        // DSCPTR before GETBYT overwrites FAC+4 with the sub-argument.
        // VPNT = $a0/$a1 = ZP_FAC+3/+4; DSCPTR = ZP_DSCPTR ($8c).
        const std::uint16_t vpnt = variables_const().readWord(
            static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 3u));
        variables().writeWord(ApplesoftVariables::ZP_DSCPTR, vpnt);

        // GETBYT evaluates the numeric sub-argument; result lands in FAC+4.
        (void)GETBYT();
    } else {
        // L_UNARY_1: numeric or CHR$ function — require "(expression)".
        PARCHK();
    }

    // L_UNARY_2: dispatch through UNFNC table.
    // Index = token - TOKEN_SGN (0-based, matches UNFNC table layout).
    const std::size_t index = static_cast<std::size_t>(token - kTOKEN_SGN);
    UNFNC(index)();
    // CHR$, LEFT$, RIGHT$, MID$ handlers do not return to this point.
    CHKNUM();
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
    WriteZeroPageByte(kSUBFLG, static_cast<std::uint8_t>(subflg | 0x80u));

    // Parse function name via PTRGET3
    PTRGET3();

    // PTRGET3 leaves A=name_lo, Y=name_hi
    // Store to FNCNAM
    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    const std::uint8_t nameA = ReadZeroPageByte(ApplesoftVariables::ZP_STRNG1);  // Temp storage from PTRGET3
    const std::uint8_t nameY = ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_STRNG1 + 1u));
    WriteZeroPageByte(kFNCNAM, nameA);
    WriteZeroPageByte(static_cast<std::uint8_t>(kFNCNAM + 1u), nameY);

    // Jump to CHKNUM to validate numeric type
    CHKNUM();
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
    WriteZeroPageByte(static_cast<std::uint8_t>(kVARPNT + 1u), ReadZeroPageByte(static_cast<std::uint8_t>(kVARPNT + 1u)));
    WriteZeroPageByte(kVARPNT, ReadZeroPageByte(kVARPNT));

    // Stack the text pointer (TXTPTR)
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    const std::uint16_t txtPtr = ReadZeroPageWord(kTXTPTR);
    (void)txtPtr;

    // Scan to next statement
    DATA();

    // Fall through to FNCDATA to store 5-byte FAC
}


// Stack emulation for FUNCT/FNCDATA.
// 6502 stack is LIFO at $0100-$01FF; we model it as a simple deque during execution.
// FN call stack is managed via theStack().pushFnByte() / peekFnByte() / popFnByte().

void FUNCT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FUNCT (inclusive) .. FNCDATA (exclusive)
    // Name normalization: none (assembler label FUNCT kept verbatim).
    //
    // "FN" FUNCTION CALL - invoke user-defined function
    // Parse FN name, save old argument value, evaluate expression with new value,
    // restore old value via FNCDATA.

    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    constexpr std::uint8_t kVARPNT = ApplesoftVariables::ZP_VARPNT;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    // Clear stack for this function call
    theStack().clearFnStack();

    // Parse "FN name"
    FNC_();

    // Stack function address for nested FN calls (push high byte, then low byte)
    const std::uint16_t fncAddr = ReadZeroPageWord(kFNCNAM);
    theStack().pushFnByte(ApplesoftVariables::highByte(fncAddr));
    theStack().pushFnByte(ApplesoftVariables::lowByte(fncAddr));

    // Parse "(expression)" and evaluate
    PARCHK();

    // Result in FAC - must be numeric
    CHKNUM();

    // Pop function address back (in reverse order: low byte, then high byte)
    std::uint16_t tempAddr = ReadZeroPageWord(kFNCNAM);
    ApplesoftVariables::setLowByte(tempAddr, theStack().peekFnByte());
    theStack().popFnByte();
    ApplesoftVariables::setHighByte(tempAddr, theStack().peekFnByte());
    theStack().popFnByte();
    WriteZeroPageWord(kFNCNAM, tempAddr);

    // Get argument variable pointer from FNCNAM+2,+3 (offsets within function definition)
    // Read 16-bit pointer from function definition at offset +2,+3
    const std::uint16_t funcDefAddr = ReadZeroPageWord(kFNCNAM);
    const std::uint8_t argVarAddrLo = variables_const().readByte(funcDefAddr + 2u);
    const std::uint8_t argVarAddrHi = variables_const().readByte(funcDefAddr + 3u);
    
    // Check for undefined function (high byte of address must be non-zero)
    if (argVarAddrHi == 0u) {
        // Undefined function error
        UNDFNC();
        return;
    }
    
    const std::uint16_t argVarAddr = variables_const().makeWord(argVarAddrLo, argVarAddrHi);
    WriteZeroPageWord(kVARPNT, argVarAddr);

    // Save old value of argument variable (5 bytes) to stack
    // Loop from Y=4 down to Y=0 (inclusive)
    for (std::int8_t y = 4; y >= 0; --y) {
        const std::uint8_t byte = variables_const().readByte(argVarAddr + y);
        theStack().pushFnByte(byte);
    }

    // Store FAC to argument variable (rounded)
    // The assembly loads Y with VARPNT+1 and then calls STORE_FACDB_YX_ROUNDED.
    // Since STORE_FACDB_YX_ROUNDED takes no parameters, it must read the target
    // address from VARPNT (which we just set).
    STORE_FACDB_YX_ROUNDED();

    // Save current TXTPTR to stack (push high byte, then low byte)
    const std::uint16_t savedTxtPtr = ReadZeroPageWord(kTXTPTR);
    theStack().pushFnByte(ApplesoftVariables::highByte(savedTxtPtr));
    theStack().pushFnByte(ApplesoftVariables::lowByte(savedTxtPtr));

    // Load function definition address to TXTPTR (point to function body)
    WriteZeroPageWord(kTXTPTR, funcDefAddr);

    // Save argument variable address to stack (push high byte, then low byte)
    theStack().pushFnByte(ApplesoftVariables::highByte(argVarAddr));
    theStack().pushFnByte(ApplesoftVariables::lowByte(argVarAddr));

    // Evaluate the function expression
    FRMNUM();

    // Pop argument variable address back and store to FNCNAM
    std::uint16_t argAddr = ReadZeroPageWord(kVARPNT);
    ApplesoftVariables::setLowByte(argAddr, theStack().peekFnByte());
    theStack().popFnByte();
    ApplesoftVariables::setHighByte(argAddr, theStack().peekFnByte());
    theStack().popFnByte();
    WriteZeroPageWord(kFNCNAM, argAddr);

    // Check for ":" or EOL
    if (CHRGOT() != 0u && CHRGOT() != static_cast<std::uint8_t>(':')) {
        SYNERR();
    }

    // Pop and restore TXTPTR
    std::uint16_t txtAddr = ReadZeroPageWord(kTXTPTR);
    ApplesoftVariables::setLowByte(txtAddr, theStack().peekFnByte());
    theStack().popFnByte();
    ApplesoftVariables::setHighByte(txtAddr, theStack().peekFnByte());
    theStack().popFnByte();
    WriteZeroPageWord(kTXTPTR, txtAddr);

    // Stack now contains 5 saved bytes - fall through to FNCDATA to restore
}


void FNCDATA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNCDATA (inclusive) .. STR (exclusive)
    // Name normalization: none (assembler label FNCDATA kept verbatim).
    //
    // STORE FIVE BYTES FROM STACK AT (FNCNAM)
    // Pop 5 stack bytes and store to (FNCNAM),Y with Y incrementing from 0 to 4.

    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    const std::uint16_t fncnampnt = ReadZeroPageWord(kFNCNAM);

    // Loop 5 times: pop stack byte and store to (FNCNAM)+Y
    for (std::uint8_t y = 0u; y < 5u; ++y) {
        if (!theStack().fnStackEmpty()) {
            const std::uint8_t byte = theStack().peekFnByte();
            theStack().popFnByte();
            variables().writeByte(fncnampnt + y, byte);
        }
    }
}


#if 0
void FLOAT_1(std::uint8_t exponent) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FLOAT_1 (inclusive) .. FLOAT_2 (exclusive)
    // Name normalization: none (assembler label FLOAT_1 kept verbatim).
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exponent);
}


void FLOAT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FLOAT (inclusive) .. QINT (exclusive)
    // Name normalization: none (assembler label FLOAT kept verbatim).
}
#endif


void FRMEVL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FRMEVL (inclusive) .. FRM_STACK_1 (exclusive)
    // Name normalization: FRMEVL_1/2 and related sublabels are modeled inline.
    //
    // Incremental port note:
    // This now includes the FRM_RECURSE..FRM_STACK_1 tranche by modeling the
    // recursive precedence walk and the stacked-LHS frame handoff to ARG/CPRMASK.

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;
    constexpr std::uint8_t kCPRTYP = ApplesoftVariables::ZP_CPRTYP;
    constexpr std::uint8_t kCPRMASK = ApplesoftVariables::ZP_CPRMASK;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kFAC_SIGN = ApplesoftVariables::ZP_FAC_SIGN;
    constexpr std::uint8_t kARG = ApplesoftVariables::ZP_ARG;
    constexpr std::uint8_t kSGNCPR = ApplesoftVariables::ZP_STRNG1; // SGNCPR shares $AB with STRNG1.
    constexpr std::uint8_t kSTACK_ROOM_BYTES = 1u;
    constexpr std::uint8_t kTOKEN_PLUS = 0xc8u;
    constexpr std::uint8_t kTOKEN_GREATER = 0xcfu;
    constexpr std::uint8_t kTOKEN_EQUAL = 0xd0u;
    constexpr std::uint8_t kTOKEN_LESS = 0xd1u;

    const auto frmevl_eval = [&](auto&& self, std::uint8_t callerPrecedence, bool runEntryBackstep) -> void {
        // FRMEVL entry point only: back TXTPTR up one byte so FRM_ELEMENT starts
        // from the current token. Recursive FRMEVL_1 calls skip this backstep.
        if (runEntryBackstep) {
            const std::uint16_t txtptr = ReadZeroPageWord(kTXTPTR);
            WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(txtptr - 1u));
        }

        // FRMEVL_1 prologue: CHKMEM(1), FRM_ELEMENT.
        CHKMEMState chkmemState{};
        chkmemState.a = kSTACK_ROOM_BYTES;
        if (!CHKMEM(chkmemState).ok) {
            return;
        }
        UNARY();
        WriteZeroPageByte(kCPRTYP, 0u);

        while (true) {
            std::uint8_t token = CHRGOT();

            // FRMEVL_2 relational scan: absorb chains of <, =, >.
            while (token == kTOKEN_GREATER || token == kTOKEN_EQUAL || token == kTOKEN_LESS) {
                std::uint8_t mask = 0u;
                if (token == kTOKEN_GREATER) {
                    mask = 0x01u;
                } else if (token == kTOKEN_EQUAL) {
                    mask = 0x02u;
                } else {
                    mask = 0x04u;
                }

                const std::uint8_t existing = ReadZeroPageByte(kCPRTYP);
                if ((existing & mask) != 0u) {
                    SYNERR();
                    return;
                }

                WriteZeroPageByte(kCPRTYP, static_cast<std::uint8_t>(existing | mask));
                token = CHRGET();
            }

            MathTblEntry pendingEntry{};
            std::uint8_t cprtypForFrame = 0u;
            bool relationalPath = false;

            if (ReadZeroPageByte(kCPRTYP) != 0u) {
                // FRM_RELATIONAL: fold string-vs-numeric state into CPRTYP and
                // treat as MATHTBL M_REL for precedence dispatch.
                relationalPath = true;
                const std::uint8_t relFlags = ReadZeroPageByte(kCPRTYP);
                const bool facIsString = (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u;
                cprtypForFrame = static_cast<std::uint8_t>((relFlags << 1u) | (facIsString ? 1u : 0u));

                const std::uint16_t txtptr = ReadZeroPageWord(kTXTPTR);
                WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(txtptr - 1u));
                pendingEntry = MATHTBL(M_REL_IDX);
            } else {
                // NOTMATH/GOEX: stop when the next token is not an infix operator.
                if (token < kTOKEN_PLUS || token > kTOKEN_LESS) {
                    return;
                }

                // FRMEVL_2_3 special-case (+ with string FAC) is CAT in ROM.
                if (token == kTOKEN_PLUS && (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u) {
                    CAT();
                    return;
                }

                CHKNUM();

                const std::size_t mathIndex = static_cast<std::size_t>(token - kTOKEN_PLUS);
                if (mathIndex > M_REL_IDX) {
                    return;
                }
                pendingEntry = MATHTBL(mathIndex);
            }

            // FRM_PRECEDENCE_TEST/PREFNC: defer lower-or-equal precedence work
            // to the caller stack frame.
            if (callerPrecedence >= pendingEntry.precedence) {
                return;
            }

            // FRM_RECURSE (inclusive) .. FRM_STACK_1 (exclusive): recurse into
            // FRMEVL_1 while carrying pending operator/precedence state.
            const std::array<std::uint8_t, 5> lhsFac = {
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 0u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 1u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 2u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 3u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 4u)),
            };
            const std::uint8_t lhsSign = ReadZeroPageByte(kFAC_SIGN);

            if (!relationalPath) {
                (void)CHRGET();
            }
            self(self, pendingEntry.precedence, false);

            // FRM_PERFORM_2 frame handoff: move stacked left operand to ARG and
            // synthesize CPRMASK/SGNCPR as if popped from the ROM expression stack.
            WriteZeroPageByte(kCPRMASK, static_cast<std::uint8_t>(cprtypForFrame >> 1u));
            for (std::uint8_t i = 0; i < lhsFac.size(); ++i) {
                WriteZeroPageByte(static_cast<std::uint8_t>(kARG + i), lhsFac[i]);
            }
            WriteZeroPageByte(static_cast<std::uint8_t>(kARG + 5u), lhsSign);
            WriteZeroPageByte(kSGNCPR, static_cast<std::uint8_t>(lhsSign ^ ReadZeroPageByte(kFAC_SIGN)));

            if (cprtypForFrame != 0u) {
                WriteZeroPageByte(kCPRTYP, cprtypForFrame);
            }

            if (pendingEntry.handler != nullptr) {
                pendingEntry.handler();
            }
        }
    };

    frmevl_eval(frmevl_eval, 0u, true);
}

std::uint8_t MON_SCRN(std::uint8_t row, std::uint8_t column) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
    // Labels: SCRN (inclusive) .. end of SCRN routine (exclusive)
    // Name normalization: SCRN -> MON_SCRN (monitor label gets MON_ prefix).
    //
    // Monitor path:
    //   lsr A / php / jsr GBASCALC / lda (GBASL),Y / plp
    //   if odd row, shift high nibble down; then mask to 4-bit color.

    const std::uint8_t halfRow = static_cast<std::uint8_t>(row >> 1u);
    const bool gbasCarry = (row & 0x01u) != 0u;

    // Inline GBASCALC for base address synthesis in page $0400-$07ff.
    const std::uint8_t gbash = static_cast<std::uint8_t>(((halfRow >> 1u) & 0x03u) | 0x04u);
    std::uint8_t gbasl = static_cast<std::uint8_t>(halfRow & 0x18u);
    if (gbasCarry) {
        gbasl = static_cast<std::uint8_t>(gbasl + 0x80u);
    }
    const std::uint8_t gbaslBase = gbasl;
    gbasl = static_cast<std::uint8_t>((gbasl << 2u) | gbaslBase);

    const std::uint16_t baseAddress = ApplesoftVariables::makeWord(gbasl, gbash);
    const std::uint16_t screenAddress = static_cast<std::uint16_t>(baseAddress + column);

    std::uint8_t value = variables_const().readByte(screenAddress);
    if ((row & 0x01u) != 0u) {
        value = static_cast<std::uint8_t>(value >> 4u);
    }
    return static_cast<std::uint8_t>(value & 0x0fu);
}

void MON_IRQ();
void MON_ADDR_03FB();

class MON_M6502VEC {
public:
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/vectors.o65.lst
    // Labels: M6502VEC (inclusive) .. end of listing (exclusive)
    // Name normalization: M6502VEC -> MON_M6502VEC (monitor label gets MON_ prefix).
    //
    // EOM vector helpers: each method directly transfers control to the target
    // used by the original vector entry.
    static void NMI_VECTOR() {
        // FFFA points at $03FB.
        MON_ADDR_03FB();
    }

    static void RESET_VECTOR() {
        // FFFC points to RESET2.
        MON_RESET2();
    }

    static void IRQ_VECTOR() {
        // FFFE points to IRQ.
        MON_IRQ();
    }
};

void MON_ADDR_03FB() {
    // TODO(asm-port): port monitor vector target at address $03FB.
}

void MON_IRQ() {
    // TODO(asm-port): port IRQ monitor label.
}

std::int8_t FCOMP(std::uint16_t /*argAddress*/) {
    return 0;
}

std::int8_t CompareArgAndFacStrings() {
    return 0;
}

std::uint8_t MON_PREAD() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/paddles.o65.lst
    // Labels: PREAD (inclusive) .. end of listing routine (exclusive)
    // Name normalization: PREAD -> MON_PREAD (monitor label gets MON_ prefix).
    //
    // Monitor flow:
    //   lda PTRIG        ; trigger paddle timing
    //   ldy #$00         ; counter
    // PREAD2:
    //   lda PADDL0,X     ; selected paddle input
    //   bpl RTS2D        ; return when bit 7 clears
    //   iny              ; count while timing bit remains set
    //   bne PREAD2       ; saturate at 255
    //   dey
    // RTS2D: rts

    constexpr std::uint8_t kFAC_LAST = static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 4u);

    // In the original monitor, paddle index is supplied in X.
    // The current C++ calling path provides the converted operand in FAC+4.
    const std::uint8_t paddleIndex = ReadZeroPageByte(kFAC_LAST);
    const std::uint16_t paddleAddress = static_cast<std::uint16_t>(IOPorts::ADDR_PADDLE_0 + paddleIndex);

    (void)variables_const().readByte(IOPorts::ADDR_PADDLE_TRIGGER);

    std::uint8_t count = 0u;
    for (;;) {
        const std::uint8_t paddleValue = variables_const().readByte(paddleAddress);
        if ((paddleValue & 0x80u) == 0u) {
            return count;
        }

        count = static_cast<std::uint8_t>(count + 1u);
        if (count == 0u) {
            return 0xffu;
        }
    }
}


}  // namespace applesoft::asm_port
