#include "core/asm_port_nxin.hpp"

#include "platform/asm_port_outdo.hpp"

namespace applesoft::asm_port {
namespace {

// TODO(asm-port): port INLIN label.
void INLIN() {}

} // namespace

void NXIN() {
    // PRINT "?"
    OUTQUES();

    // Original code tail-jumps to INLIN.
    INLIN();
}

} // namespace applesoft::asm_port