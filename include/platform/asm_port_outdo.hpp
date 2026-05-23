#pragma once

#include <cstdint>

namespace applesoft::asm_port {
std::uint8_t AS_OUTDO(std::uint8_t a);

// Monitor output routines
void MON_COUT(std::uint8_t a);
void MON_COUT1(std::uint8_t a);
void AS_OUTSP();
void AS_OUTQUES();

} // namespace applesoft::asm_port
