#pragma once

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_STRTXT (inclusive) .. AS_NOT_ (exclusive)
// Name normalization: none (assembler label AS_STRTXT kept verbatim).
void AS_STRTXT();

// AS_Labels: AS_POINT (inclusive) .. AS_L_POINT_1 (exclusive)
void AS_POINT();

} // namespace applesoft::asm_port
