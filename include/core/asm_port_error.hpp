#pragma once

#include "core/asm_port_control_flow.hpp"
#include "core/asm_port_core.hpp"
#include "core/asm_port_parser.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_statements.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

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

std::uint8_t AS_CHRGET();
std::uint8_t AS_CHRGOT();

void AS_ERROR(std::uint8_t error_code_offset);

void AS_PRINT_ERROR_LINUM(
    std::string_view prefix = AS_QT_ERROR(AS_QT_ERROR_INDEX));

// Helper stubs for error printing and restart handling.
void AS_CRDO();
void AS_INPRT();
void AS_STKINI();
void AS_OUTQUES();

} // namespace applesoft::asm_port
