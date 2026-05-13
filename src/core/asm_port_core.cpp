#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_reason.hpp"
#include "core/io_ports.hpp"

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

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

}  // namespace applesoft::asm_port
