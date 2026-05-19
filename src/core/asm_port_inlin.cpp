#include "core/asm_port_inlin.hpp"

#include <cstdint>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_INLIN (inclusive) .. AS_INCHR (exclusive)
// Name normalization: none (assembler label AS_INLIN kept verbatim).
Inlin2Result AS_INLIN() {
  // AS_INLIN loads X with $80 (null prompt) and falls through to AS_INLIN2.
  return AS_INLIN2(static_cast<std::uint8_t>(0x80));
}

} // namespace applesoft::asm_port