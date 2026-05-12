#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/keyin.o65.lst
// Labels: GETLN (inclusive) .. CROUT (exclusive)
// All monitor labels carry a virtual MON_ prefix in C++; GETLN -> MON_GETLN.
std::uint8_t MON_GETLN();

} // namespace applesoft::asm_port