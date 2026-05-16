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

void AS_CLEAR() {
  if (AS_CHRGOT() != 0) {
    return;
  }
  AS_CLEARC();
}

void AS_CLEARC() {
  constexpr auto kAS_MEMSIZ = ApplesoftVariables::ZP_AS_MEMSIZ;
  constexpr auto kAS_FRETOP = ApplesoftVariables::ZP_AS_FRETOP;
  constexpr auto kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
  constexpr auto kAS_ARYTAB = ApplesoftVariables::ZP_AS_ARYTAB;
  constexpr auto kAS_STREND = ApplesoftVariables::ZP_AS_STREND;

  WriteZeroPageWord(kAS_FRETOP, ReadZeroPageWord(kAS_MEMSIZ));

  std::uint16_t vartab = ReadZeroPageWord(kAS_VARTAB);
  WriteZeroPageWord(kAS_ARYTAB, vartab);
  WriteZeroPageWord(kAS_STREND, vartab);

  AS_RESTORE();
  AS_STKINI();
}

void AS_STKINI() {
  constexpr auto kAS_TEMPPT = ApplesoftVariables::ZP_AS_TEMPPT;
  constexpr auto kAS_TEMPST = ApplesoftVariables::ZP_AS_TEMPST;
  constexpr auto kAS_OLDTEXT = ApplesoftVariables::ZP_AS_OLDTEXT;
  constexpr auto kAS_SUBFLG = ApplesoftVariables::ZP_AS_SUBFLG;

  WriteZeroPageByte(kAS_TEMPPT, kAS_TEMPST);

  // 1727-1734: Stack management
  // Applesoft preserves the return address across the stack reset (TXS $F8).
  // In our C++ interpreter, theStack().setStackPointer() mimics TXS.
  theStack().setStackPointer(0xf8u);

  // Resetting AS_OLDTEXT (high byte) and AS_SUBFLG
  WriteZeroPageByte(kAS_OLDTEXT + 1, 0);
  WriteZeroPageByte(kAS_SUBFLG, 0);
}

void AS_STXTPT() {
  constexpr auto kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  constexpr auto kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;

  WriteZeroPageWord(kAS_TXTPTR, ReadZeroPageWord(kAS_TXTTAB) - 1);
}

void AS_RESTORE() {
  constexpr auto kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  AS_SETDA(ReadZeroPageWord(kAS_TXTTAB) - 1);
}

void AS_SETDA(std::uint16_t addr) {
  constexpr auto kAS_DATPTR = ApplesoftVariables::ZP_AS_DATPTR;
  WriteZeroPageWord(kAS_DATPTR, addr);
}

} // namespace applesoft::asm_port
