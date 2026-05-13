#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"

#include <cstdint>

namespace applesoft::asm_port {

std::uint16_t ReadZeroPageWord(std::uint8_t address);
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void MON_WRITE();
void MON_READ();
bool SETPTRS();
void FIX_LINKS();
void ADDON(std::uint8_t offset);
std::uint8_t DATAN();
void VARTIO();
void PROGIO();

void DATA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DATA (inclusive) .. ADDON (exclusive)
    // Name normalization: none (assembler label DATA kept verbatim).

    const std::uint8_t offset = DATAN();
    ADDON(offset);
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