#include "core/asm_port_nxin.hpp"

#include "core/asm_port_inlin.hpp"
#include "platform/asm_port_outdo.hpp"

namespace applesoft::asm_port {

void NXIN() {
    // PRINT "?"
    OUTQUES();

    // Original code tail-jumps to INLIN.
    (void)INLIN();
}

} // namespace applesoft::asm_port