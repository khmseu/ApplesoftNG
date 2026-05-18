#pragma once

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_NXIN (inclusive) .. AS_READ (exclusive)
// Name normalization: none (assembler label AS_NXIN kept verbatim).
void AS_NXIN();

} // namespace applesoft::asm_port