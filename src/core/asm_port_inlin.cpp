#include "core/asm_port_inlin.hpp"

#include <cstdint>

namespace applesoft::asm_port {

Inlin2Result AS_INLIN() {
    // AS_INLIN loads X with $80 (null prompt) and falls through to AS_INLIN2.
    return AS_INLIN2(static_cast<std::uint8_t>(0x80));
}

} // namespace applesoft::asm_port