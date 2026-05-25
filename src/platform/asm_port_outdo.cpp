#include "platform/asm_port_outdo.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_characters.hpp"
#include "core/asm_port_graphics.hpp"
#include "core/io_ports.hpp"
#include "core/jump_table.hpp"
#include <cstdint>

namespace applesoft::asm_port {
namespace {
using MonitorOutputRoutine = void (*)(std::uint8_t);
} // namespace
static std::uint16_t computeTextRowBase(std::uint8_t row_zero_based) {
  const bool carryFromAS_Lsr = (row_zero_based & 0x01u) != 0u;
  const std::uint8_t bash =
      static_cast<std::uint8_t>(((row_zero_based >> 1u) & 0x03u) | 0x04u);
  std::uint8_t basl = static_cast<std::uint8_t>(row_zero_based & 0x18u);
  if (carryFromAS_Lsr)
    basl = static_cast<std::uint8_t>(basl + 0x80u);
  const std::uint8_t baslBase = basl;
  basl = static_cast<std::uint8_t>((basl << 2u) | baslBase);
  basl = static_cast<std::uint8_t>(basl + variables_const().MON_WNDLFT);
  return ApplesoftVariables::makeWord(basl, bash);
}
void setCursorRow(std::uint8_t row_zero_based) { MON_TABV(row_zero_based); }
static void scrollWindowUp() {
  constexpr std::uint8_t kBlank = kCharSpaceHigh;
  const std::uint8_t top = variables_const().MON_WNDTOP;
  const std::uint8_t bottom = variables_const().MON_WNDBTM;
  const std::uint8_t width = variables_const().MON_WNDWDTH;
  for (std::uint8_t row = top; static_cast<std::uint8_t>(row + 1u) < bottom;
       ++row) {
    const auto dstBase = computeTextRowBase(row);
    const auto srcBase =
        computeTextRowBase(static_cast<std::uint8_t>(row + 1u));
    for (std::uint8_t col = 0u; col < width; ++col)
      variables().writeByte(static_cast<std::uint16_t>(dstBase + col),
                            variables_const().readByte(
                                static_cast<std::uint16_t>(srcBase + col)));
  }
  const auto lastRowBase =
      computeTextRowBase(static_cast<std::uint8_t>(bottom - 1u));
  for (std::uint8_t col = 0u; col < width; ++col)
    variables().writeByte(static_cast<std::uint16_t>(lastRowBase + col),
                          kBlank);
}
static void advanceCursorToNextAS_Line(bool resetColumn) {
  const std::uint8_t top = variables_const().MON_WNDTOP;
  const std::uint8_t bottom = variables_const().MON_WNDBTM;
  std::uint8_t row = variables_const().MON_CV;
  if (resetColumn)
    variables().MON_CH = 0u;
  if (bottom <= top) {
    setCursorRow(top);
    return;
  }
  if (static_cast<std::uint8_t>(row + 1u) >= bottom) {
    scrollWindowUp();
    row = static_cast<std::uint8_t>(bottom - 1u);
  } else
    row = static_cast<std::uint8_t>(row + 1u);
  setCursorRow(row);
}
static void consumeKeyboardAS_Latch(std::uint8_t keycode) {
  (void)ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD_STROBE);
  ioPorts().writeByte(IOPorts::ADDR_AS_KEYBOARD,
                      static_cast<std::uint8_t>(keycode & 0x7fu));
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_VIDOUT (inclusive) .. MON_ESC1 (exclusive)
// Name normalization: none (assembler label MON_VIDOUT kept verbatim).
static void MON_VIDOUT(std::uint8_t a) {
  const std::uint8_t ch = static_cast<std::uint8_t>(a & 0x7fu);
  switch (ch) {
  case 0x07u:
    break;
  case 0x08u: {
    const std::uint8_t column = variables_const().MON_CH;
    if (column != 0u)
      variables().MON_CH = static_cast<std::uint8_t>(column - 1u);
    break;
  }
  case 0x0au:
    advanceCursorToNextAS_Line(false);
    break;
  case 0x0du:
    advanceCursorToNextAS_Line(true);
    break;
  default:
    if (a >= 0xa0u) {
      const std::uint8_t column = variables_const().MON_CH;
      const std::uint8_t width = variables_const().MON_WNDWDTH;
      const std::uint16_t base = variables_const().MON_BASL;
      variables().writeByte(static_cast<std::uint16_t>(base + column), a);
      const std::uint8_t nextColumn = static_cast<std::uint8_t>(column + 1u);
      if (nextColumn >= width)
        advanceCursorToNextAS_Line(true);
      else
        variables().MON_CH = nextColumn;
    }
    break;
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_VIDWAIT (inclusive) .. MON_ESCOLD (exclusive)
// Name normalization: none (assembler label MON_VIDWAIT kept verbatim).
static void MON_VIDWAIT(std::uint8_t a) {
  if (a == kCharCarriageReturnHigh) {
    std::uint8_t keycode = ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD);
    if ((keycode & 0x80u) != 0u && keycode == kControlCharSHigh) {
      consumeKeyboardAS_Latch(keycode);
      do {
        keycode = ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD);
      } while ((keycode & 0x80u) == 0u);
      if (keycode != kControlCharCHigh)
        consumeKeyboardAS_Latch(keycode);
    }
  }
  MON_VIDOUT(a);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_COUT1 (inclusive) .. MON_BL1 (exclusive)
void MON_COUT1(std::uint8_t a) {
  if (a >= 0xa0u)
    a &= variables_const().MON_INVFLG;
  MON_VIDWAIT(a);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_WAIT (inclusive) .. MON_NXTA4 (exclusive)
// Name normalization: none (assembler label MON_WAIT kept verbatim).
static void MON_WAIT(std::uint8_t a) {
  volatile std::uint8_t outer = a;
  do {
    volatile std::uint8_t inner = outer;
    do {
      inner = static_cast<std::uint8_t>(inner - 1u);
    } while (inner != 0u);
    outer = static_cast<std::uint8_t>(outer - 1u);
  } while (outer != 0u);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_COUT (inclusive) .. MON_COUT1 (exclusive)
void MON_COUT(std::uint8_t a) {
  ApplesoftNG::ExternalJumpDispatcher::JumpFromWord(
      ApplesoftVariables::ZP_MON_CSW, a);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_OUTDO (inclusive) .. AS_INPUTERR (exclusive)
// Name normalization: none (assembler label AS_OUTDO kept verbatim).
std::uint8_t AS_OUTDO(std::uint8_t a) {
  a |= 0x80u;
  if (a >= 0xa0u)
    a |= variables_const().AS_FLASH_BIT;
  MON_COUT(a);
  a &= 0x7fu;
  MON_WAIT(variables_const().AS_SPEEDZ);
  return a;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_OUTSP (inclusive) .. AS_OUTQUES (exclusive)
// Name normalization: none (assembler label AS_OUTSP kept verbatim).
void AS_OUTSP() { AS_OUTDO(kCharSpace); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_OUTQUES (inclusive) .. AS_OUTDO (exclusive)
// Name normalization: none (assembler label AS_OUTQUES kept verbatim).
void AS_OUTQUES() { AS_OUTDO(kCharQuestionMark); }
} // namespace applesoft::asm_port
