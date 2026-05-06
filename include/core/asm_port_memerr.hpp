#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MEMERR (inclusive) .. ERROR (exclusive)
// Name normalization: none (assembler label MEMERR kept verbatim).

// Load the Applesoft error code for "OUT OF MEMORY" into X.
std::uint8_t MEMERR();

} // namespace applesoft::asm_port
