#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GETLN (inclusive) .. CROUT (exclusive)
// All monitor labels carry a virtual MON_ prefix in C++; AS_GETLN -> MON_GETLN.
std::uint8_t MON_GETLN();

} // namespace applesoft::asm_port