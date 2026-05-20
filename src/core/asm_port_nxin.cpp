#include "core/asm_port_nxin.hpp"

#include "core/asm_port_inlin.hpp"
#include "platform/asm_port_outdo.hpp"

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_NXIN (inclusive) .. AS_READ (exclusive)
// Name normalization: none (assembler label AS_NXIN kept verbatim).
void AS_NXIN() {
  // AS_PRINT "?"
  AS_OUTQUES();

  // Original code tail-jumps to AS_INLIN.
  (void)AS_INLIN();
}

} // namespace applesoft::asm_port