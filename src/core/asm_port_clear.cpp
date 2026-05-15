#include "core/asm_port_clear.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_stack.hpp"
#include <cstdint>

namespace applesoft::asm_port {

// Forward declarations for memory access (should ideally be in a header)
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);

void CLEAR() {
    if (CHRGOT() != 0) {
        return;
    }
    CLEARC();
}

void CLEARC() {
    constexpr auto kMEMSIZ = ApplesoftVariables::ZP_MEMSIZ;
    constexpr auto kFRETOP = ApplesoftVariables::ZP_FRETOP;
    constexpr auto kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr auto kARYTAB = ApplesoftVariables::ZP_ARYTAB;
    constexpr auto kSTREND = ApplesoftVariables::ZP_STREND;

    WriteZeroPageWord(kFRETOP, ReadZeroPageWord(kMEMSIZ));
    
    std::uint16_t vartab = ReadZeroPageWord(kVARTAB);
    WriteZeroPageWord(kARYTAB, vartab);
    WriteZeroPageWord(kSTREND, vartab);

    RESTORE();
    STKINI();
}

void STKINI() {
    constexpr auto kTEMPPT = ApplesoftVariables::ZP_TEMPPT;
    constexpr auto kTEMPST = ApplesoftVariables::ZP_TEMPST;
    constexpr auto kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;
    constexpr auto kSUBFLG = ApplesoftVariables::ZP_SUBFLG;

    WriteZeroPageByte(kTEMPPT, kTEMPST);

    // 1727-1734: Stack management
    // Applesoft preserves the return address across the stack reset (TXS $F8).
    // In our C++ interpreter, theStack().setStackPointer() mimics TXS.
    theStack().setStackPointer(0xf8u);

    // Resetting OLDTEXT (high byte) and SUBFLG
    WriteZeroPageByte(kOLDTEXT + 1, 0); 
    WriteZeroPageByte(kSUBFLG, 0);
}

void STXTPT() {
    constexpr auto kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    constexpr auto kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    WriteZeroPageWord(kTXTPTR, ReadZeroPageWord(kTXTTAB) - 1);
}

void RESTORE() {
    constexpr auto kTXTTAB = ApplesoftVariables::ZP_TXTTAB;
    SETDA(ReadZeroPageWord(kTXTTAB) - 1);
}

void SETDA(std::uint16_t addr) {
    constexpr auto kDATPTR = ApplesoftVariables::ZP_DATPTR;
    WriteZeroPageWord(kDATPTR, addr);
}

} // namespace applesoft::asm_port
