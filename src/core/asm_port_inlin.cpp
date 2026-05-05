#include "core/asm_port_inlin.hpp"

#include <cstdint>

namespace applesoft::asm_port {

Inlin2Result INLIN() {
    // INLIN loads X with $80 (null prompt) and falls through to INLIN2.
    return INLIN2(static_cast<std::uint8_t>(0x80));
}

} // namespace applesoft::asm_port