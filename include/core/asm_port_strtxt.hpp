#pragma once

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRTXT (inclusive) .. NOT_ (exclusive)
// Name normalization: none (assembler label STRTXT kept verbatim).
void STRTXT();

// Labels: POINT (inclusive) .. L_POINT_1 (exclusive)
void POINT();

} // namespace applesoft::asm_port
