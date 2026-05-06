#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/keyin.o65.lst
// Labels: GETLN (inclusive) .. CROUT (exclusive)
// Name normalization: none (assembler label GETLN kept verbatim).
std::uint8_t GETLN();

} // namespace applesoft::asm_port