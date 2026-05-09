#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUT (inclusive) .. NXIN (exclusive)
// Name normalization: none (assembler label INPUT kept verbatim).
void INPUT();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: NXIN (inclusive) .. READ (exclusive)
// Name normalization: none (assembler label NXIN kept verbatim).
void NXIN();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: READ (inclusive) .. INPUT_FLAG_ZERO (exclusive)
// Name normalization: none (assembler label READ kept verbatim).
void READ();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: INPUT_FLAG_ZERO (inclusive) .. PROCESS_INPUT_LIST (exclusive)
// Name normalization: none (assembler label INPUT_FLAG_ZERO kept verbatim).
void INPUT_FLAG_ZERO(std::uint16_t input_ptr);

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PROCESS_INPUT_LIST (inclusive) .. NEXT (exclusive)
// Name normalization: none (assembler label PROCESS_INPUT_LIST kept verbatim).
void PROCESS_INPUT_LIST(std::uint16_t input_ptr, std::uint8_t input_flag);

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