#pragma once

#include "core/asm_port_qt_error.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);

inline constexpr std::uint8_t kAS_NEG32768Data[4] = {0x90u, 0x80u, 0x00u,
                                                     0x00u};
inline constexpr std::uint8_t kCZeroData[2] = {0x00u, 0x00u};

struct ProgramPointer {
  std::uint16_t address = 0;

  std::uint8_t read(std::uint16_t offset = 0) const {
    return ReadProgramByte(static_cast<std::uint16_t>(address + offset));
  }

  void write(std::uint8_t value, std::uint16_t offset = 0) const {
    WriteProgramByte(static_cast<std::uint16_t>(address + offset), value);
  }

  ProgramPointer advanced(std::uint16_t offset) const {
    return ProgramPointer{static_cast<std::uint16_t>(address + offset)};
  }
};

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHRGET / AS_CHRGOT
// Name normalization: none (assembler labels kept verbatim).
std::uint8_t AS_CHRGET();
std::uint8_t AS_CHRGOT();
void AS_CONINT();
std::uint8_t AS_GETBYT();
void AS_GTBYTC();
void AS_SNGFLT(std::uint8_t value);
void AS_NEG32768();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_ANDOP (inclusive) .. AS_FALSE (exclusive)
// Name normalization: none (assembler label AS_ANDOP kept verbatim).
void AS_ANDOP();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHKCOM (inclusive) .. AS_SYNCHR (exclusive)
// Name normalization: none (assembler label AS_CHKCOM kept verbatim).
void AS_CHKCOM();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHKNUM (inclusive) .. AS_CHKSTR (exclusive)
// Name normalization: none (assembler label AS_CHKNUM kept verbatim).
void AS_CHKNUM();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_PTRGET (inclusive) .. AS_PTRGET2 (exclusive)
// Name normalization: none (assembler label AS_PTRGET kept verbatim).
std::uint16_t AS_PTRGET();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_ERROR (inclusive) .. AS_PRINT_ERROR_LINNUM (exclusive)
// Name normalization: none (assembler label AS_ERROR kept verbatim).
void AS_ERROR(std::uint8_t error_code_offset);

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_PRINT_ERROR_LINNUM (inclusive) .. AS_RESTART (exclusive)
// Name normalization: none (assembler label AS_PRINT_ERROR_LINNUM kept
// verbatim).
void AS_PRINT_ERROR_LINUM(
    std::string_view prefix = AS_QT_ERROR(AS_QT_ERROR_INDEX));

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_RESTART (inclusive)
// Name normalization: none (assembler label AS_RESTART kept verbatim).
void AS_RESTART();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_RESTORE (inclusive)
// Name normalization: none (assembler label AS_RESTORE kept verbatim).
void AS_RESTORE();

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FIX_LINKS (inclusive)
// Name normalization: none (assembler label AS_FIX_LINKS kept verbatim).
void AS_FIX_LINKS();

// Helper stubs for error printing and restart handling.
void AS_CRDO();
void AS_INPRT();
void AS_STKINI();
void AS_HANDLERR();
bool IsOnErr();
bool IsDirectMode();
void AS_OUTQUES();

} // namespace applesoft::asm_port
