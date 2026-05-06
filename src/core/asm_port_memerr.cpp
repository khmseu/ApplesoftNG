#include "core/asm_port_memerr.hpp"

#include "core/asm_port_error_messages.hpp"

namespace applesoft::asm_port {

std::uint8_t MEMERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MEMERR (inclusive) .. ERROR (exclusive)
    // Name normalization: none (assembler label MEMERR kept verbatim).

    return ERR_MEMFULL;
}

} // namespace applesoft::asm_port
