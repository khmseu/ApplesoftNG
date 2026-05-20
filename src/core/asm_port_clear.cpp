#include "core/asm_port_clear.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_stack.hpp"
#include <cstdint>

namespace applesoft::asm_port {

// AS_Labels: AS_CLEAR (inclusive) .. AS_CLEARC (exclusive)
void AS_CLEAR() {
  if (AS_CHRGOT() != 0) {
    return;
  }
  AS_CLEARC();
}

// AS_Labels: AS_CLEARC (inclusive) .. AS_STKINI (exclusive)
void AS_CLEARC() {
  variables().AS_FRETOP = variables_const().AS_MEMSIZ;

  std::uint16_t vartab = variables_const().AS_VARTAB;
  variables().AS_ARYTAB = vartab;
  variables().AS_STREND = vartab;

  AS_RESTORE();
  AS_STKINI();
}

// AS_Labels: AS_STKINI (inclusive) .. AS_STXTPT (exclusive)
void AS_STKINI() {
  variables().AS_TEMPPT = ApplesoftVariables::ZP_AS_TEMPST;

  // 1727-1734: Stack management
  // Applesoft preserves the return address across the stack reset (TXS $F8).
  // In our C++ interpreter, theStack().setStackPointer() mimics TXS.
  theStack().setStackPointer(0xf8u);

  // Resetting AS_OLDTEXT (high byte) and AS_SUBFLG
  ApplesoftVariables::setHighByte(variables().AS_OLDTEXT, 0);
  variables().AS_SUBFLG = 0;
}

// AS_Labels: AS_STXTPT (inclusive) .. AS_LIST (exclusive)
void AS_STXTPT() {
  variables().AS_TXTPTR =
      static_cast<std::uint16_t>(variables_const().AS_TXTTAB - 1u);
}

// AS_Labels: AS_RESTORE (inclusive) .. AS_SETDA (exclusive)
void AS_RESTORE() {
  AS_SETDA(static_cast<std::uint16_t>(variables_const().AS_TXTTAB - 1u));
}

// AS_Labels: AS_SETDA (inclusive) .. AS_ISCNTC (exclusive)
void AS_SETDA(std::uint16_t addr) { variables().AS_DATPTR = addr; }

} // namespace applesoft::asm_port
