#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_memerr.hpp"
#include "core/asm_port_reason.hpp"
#include "core/io_ports.hpp"

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

void SYNERR();
extern std::uint8_t gJerErrorCode;
constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
    return static_cast<std::uint8_t>(lhs + rhs);
}
constexpr std::uint8_t kNEG32768Data[4] = {0x90u, 0x80u, 0x00u, 0x00u};
constexpr std::uint8_t kCZeroData[2] = {0x00u, 0x00u};
void GETARY();
void GETARY2();
void FIND_ARRAY_ELEMENT();
void ERROR(std::uint8_t error_code_offset);
std::uint16_t MULTIPLY_SUBS_1(std::uint8_t multiplierHigh);
void GIVAYF(std::int16_t value);
void SNGFLT(std::uint8_t value);
void FALSE();
void TRUE();
void ANDOP();

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

void SetStackPointer(std::uint8_t value);
void PushByteToStack(std::uint8_t value);
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

void JER() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: JER (inclusive) .. USE_OLD_ARRAY (exclusive)
    // Name normalization: none (assembler label JER kept verbatim).

    ERROR(gJerErrorCode);
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

void NEG32768() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEG32768 (inclusive) .. MAKINT (exclusive)
    // Name normalization: none (assembler label NEG32768 kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_RESULT, kNEG32768Data[0]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 1u), kNEG32768Data[1]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 2u), kNEG32768Data[2]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 3u), kNEG32768Data[3]);
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

}  // namespace applesoft::asm_port
