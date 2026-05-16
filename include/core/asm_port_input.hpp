#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPUT (inclusive) .. AS_NXIN (exclusive)
// Name normalization: none (assembler label AS_INPUT kept verbatim).
void AS_INPUT();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_NXIN (inclusive) .. AS_READ (exclusive)
// Name normalization: none (assembler label AS_NXIN kept verbatim).
void AS_NXIN();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_READ (inclusive) .. AS_INPUT_FLAG_ZERO (exclusive)
// Name normalization: none (assembler label AS_READ kept verbatim).
void AS_READ();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPUT_FLAG_ZERO (inclusive) .. AS_PROCESS_INPUT_LIST
// (exclusive) Name normalization: none (assembler label AS_INPUT_FLAG_ZERO kept
// verbatim).
void AS_INPUT_FLAG_ZERO(std::uint16_t input_ptr);

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_PROCESS_INPUT_LIST (inclusive) .. AS_NEXT (exclusive)
// Name normalization: none (assembler label AS_PROCESS_INPUT_LIST kept
// verbatim).
void AS_PROCESS_INPUT_LIST(std::uint16_t input_ptr, std::uint8_t input_flag);

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPUTERR (inclusive) .. AS_READERR (exclusive)
// Name normalization: none (assembler label AS_INPUTERR kept verbatim).
void AS_INPUTERR();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_READERR (inclusive) .. AS_ERLIN (exclusive)
// Name normalization: none (assembler label AS_READERR kept verbatim).
void AS_READERR();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_ERLIN (inclusive) .. AS_INPERR (exclusive)
// Name normalization: none (assembler label AS_ERLIN kept verbatim).
void AS_ERLIN(std::uint16_t lineNumber);

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_INPERR (inclusive) .. AS_RESPERR (exclusive)
// Name normalization: none (assembler label AS_INPERR kept verbatim).
void AS_INPERR();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_RESPERR (inclusive) .. AS_GET (exclusive)
// Name normalization: none (assembler label AS_RESPERR kept verbatim).
void AS_RESPERR();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_GET (inclusive) .. AS_INPUT (exclusive)
// Name normalization: none (assembler label AS_GET kept verbatim).
void AS_GET();

} // namespace applesoft::asm_port