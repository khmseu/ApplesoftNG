#pragma once

#include <cstdint>

namespace applesoft::asm_port {
void AS_STR();
void AS_STRLIT(std::uint16_t address);

} // namespace applesoft::asm_port