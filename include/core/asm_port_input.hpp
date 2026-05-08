#pragma once

#include <cstdint>

namespace applesoft::asm_port {

enum class InputDispatch : std::uint8_t {
    ContinueAt_INPUT_FLAG_ZERO = 0,
    ControlCTyped = 1
};

enum class InputErrorSource : std::uint8_t {
    FromInput = 0,
    FromRead = 1,
    FromGet = 2
};

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUT (inclusive) .. NXIN (exclusive)
// Name normalization: none (assembler label INPUT kept verbatim).
InputDispatch INPUT();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUTERR (inclusive) .. READERR (exclusive)
// Name normalization: none (assembler label INPUTERR kept verbatim).
void INPUTERR();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: READERR (inclusive) .. ERLIN (exclusive)
// Name normalization: none (assembler label READERR kept verbatim).
void READERR();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: ERLIN (inclusive) .. INPERR (exclusive)
// Name normalization: none (assembler label ERLIN kept verbatim).
void ERLIN(std::uint8_t a, std::uint8_t y);

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPERR (inclusive) .. RESPERR (exclusive)
// Name normalization: none (assembler label INPERR kept verbatim).
void INPERR();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: RESPERR (inclusive) .. GET (exclusive)
// Name normalization: none (assembler label RESPERR kept verbatim).
void RESPERR();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GET (inclusive) .. INPUT (exclusive)
// Name normalization: none (assembler label GET kept verbatim).
void GET();

} // namespace applesoft::asm_port