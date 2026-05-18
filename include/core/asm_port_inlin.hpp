#pragma once

#include "core/asm_port_inlin2.hpp"

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_INLIN (inclusive) .. AS_INLIN2 (exclusive)
// Name normalization: none (assembler label AS_INLIN kept verbatim).
Inlin2Result AS_INLIN();

} // namespace applesoft::asm_port