#pragma once

#include <cstdint>

namespace applesoft::asm_port {

enum class InputDispatch : std::uint8_t {
    ContinueAt_INPUT_FLAG_ZERO = 0,
    ControlCTyped = 1
};

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUT (inclusive) .. NXIN (exclusive)
// Name normalization: none (assembler label INPUT kept verbatim).
InputDispatch INPUT();

} // namespace applesoft::asm_port