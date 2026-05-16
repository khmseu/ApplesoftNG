#pragma once

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_NXIN (inclusive) .. AS_READ (exclusive)
// Name normalization: none (assembler label AS_NXIN kept verbatim).
void AS_NXIN();

} // namespace applesoft::asm_port