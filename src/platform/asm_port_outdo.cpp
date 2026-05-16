#include "platform/asm_port_outdo.hpp"
#include "core/applesoft_variables.hpp"
#include "core/io_ports.hpp"
#include <cstdint>
namespace applesoft::asm_port {
void MON_TABV(std::uint8_t row_zero_based);
namespace {
using MonitorOutputRoutine = void (*)(std::uint8_t);
constexpr std::uint16_t kMonitorCout1Vector = 0xfd62u;
void MON_VIDOUT(std::uint8_t a);
} // namespace
std::uint8_t readZeroPageByte(std::uint8_t address) {
  return variables_const().readByte(address);
}
void writeZeroPageByte(std::uint8_t address, std::uint8_t value) {
  variables().writeByte(address, value);
}
std::uint16_t readZeroPageWord(std::uint8_t address) {
  return ApplesoftVariables::makeWord(
      readZeroPageByte(address),
      readZeroPageByte(static_cast<std::uint8_t>(address + 1u)));
}
std::uint16_t computeTextRowBase(std::uint8_t row_zero_based) {
  const bool carryFromAS_Lsr = (row_zero_based & 0x01u) != 0u;
  const std::uint8_t bash =
      static_cast<std::uint8_t>(((row_zero_based >> 1u) & 0x03u) | 0x04u);
  std::uint8_t basl = static_cast<std::uint8_t>(row_zero_based & 0x18u);
  if (carryFromAS_Lsr)
    basl = static_cast<std::uint8_t>(basl + 0x80u);
  const std::uint8_t baslBase = basl;
  basl = static_cast<std::uint8_t>((basl << 2u) | baslBase);
  basl = static_cast<std::uint8_t>(
      basl + readZeroPageByte(ApplesoftVariables::ZP_MON_WNDLFT));
  return ApplesoftVariables::makeWord(basl, bash);
}
void setCursorRow(std::uint8_t row_zero_based) { MON_TABV(row_zero_based); }
void scrollWindowUp() {
  constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);
  const std::uint8_t top = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDTOP);
  const std::uint8_t bottom =
      readZeroPageByte(ApplesoftVariables::ZP_MON_WNDBTM);
  const std::uint8_t width =
      readZeroPageByte(ApplesoftVariables::ZP_MON_WNDWDTH);
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
void advanceCursorToNextAS_Line(bool resetColumn) {
  const std::uint8_t top = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDTOP);
  const std::uint8_t bottom =
      readZeroPageByte(ApplesoftVariables::ZP_MON_WNDBTM);
  std::uint8_t row = readZeroPageByte(ApplesoftVariables::ZP_MON_CV);
  if (resetColumn)
    writeZeroPageByte(ApplesoftVariables::ZP_MON_CH, 0u);
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
void consumeKeyboardAS_Latch(std::uint8_t keycode) {
  (void)ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD_STROBE);
  ioPorts().writeByte(IOPorts::ADDR_AS_KEYBOARD,
                      static_cast<std::uint8_t>(keycode & 0x7fu));
}
void MON_LFB78(std::uint8_t a) {
  constexpr std::uint8_t kCarriageReturn = 0x8du;
  constexpr std::uint8_t kCtrlS = 0x93u;
  constexpr std::uint8_t kCtrlC = 0x83u;
  if (a == kCarriageReturn) {
    std::uint8_t keycode = ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD);
    if ((keycode & 0x80u) != 0u && keycode == kCtrlS) {
      consumeKeyboardAS_Latch(keycode);
      do {
        keycode = ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD);
      } while ((keycode & 0x80u) == 0u);
      if (keycode != kCtrlC)
        consumeKeyboardAS_Latch(keycode);
    }
  }
  MON_VIDOUT(a);
}
namespace {
void MON_VIDOUT(std::uint8_t a) {
  const std::uint8_t ch = static_cast<std::uint8_t>(a & 0x7fu);
  switch (ch) {
  case 0x07u:
    break;
  case 0x08u: {
    const std::uint8_t column = readZeroPageByte(ApplesoftVariables::ZP_MON_CH);
    if (column != 0u)
      writeZeroPageByte(ApplesoftVariables::ZP_MON_CH,
                        static_cast<std::uint8_t>(column - 1u));
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
      const std::uint8_t column =
          readZeroPageByte(ApplesoftVariables::ZP_MON_CH);
      const std::uint8_t width =
          readZeroPageByte(ApplesoftVariables::ZP_MON_WNDWDTH);
      const std::uint16_t base =
          readZeroPageWord(ApplesoftVariables::ZP_MON_BASL);
      variables().writeByte(static_cast<std::uint16_t>(base + column), a);
      const std::uint8_t nextColumn = static_cast<std::uint8_t>(column + 1u);
      if (nextColumn >= width)
        advanceCursorToNextAS_Line(true);
      else
        writeZeroPageByte(ApplesoftVariables::ZP_MON_CH, nextColumn);
    }
    break;
  }
}
MonitorOutputRoutine resolveMonitorOutputVector(std::uint16_t vector) {
  return (vector == kMonitorCout1Vector) ? &MON_COUT1 : &MON_COUT1;
}
} // namespace
void MON_COUT1(std::uint8_t a) {
  if (a >= 0xa0u)
    a &= readZeroPageByte(ApplesoftVariables::ZP_MON_INVFLG);
  MON_LFB78(a);
}
void MON_WAIT(std::uint8_t a) {
  volatile std::uint8_t outer = a;
  do {
    volatile std::uint8_t inner = outer;
    do {
      inner = static_cast<std::uint8_t>(inner - 1u);
    } while (inner != 0u);
    outer = static_cast<std::uint8_t>(outer - 1u);
  } while (outer != 0u);
}
void MON_COUT(std::uint8_t a) {
  resolveMonitorOutputVector(readZeroPageWord(ApplesoftVariables::ZP_MON_CSW))(
      a);
}
std::uint8_t AS_OUTDO(std::uint8_t a) {
  a |= 0x80u;
  if (a >= 0xa0u)
    a |= variables_const().AS_FLASH_BIT;
  MON_COUT(a);
  a &= 0x7fu;
  MON_WAIT(variables_const().AS_SPEEDZ);
  return a;
}
void AS_OUTSP() { AS_OUTDO(static_cast<std::uint8_t>(' ' & 0x7fu)); }
void AS_OUTQUES() { AS_OUTDO(static_cast<std::uint8_t>('?' & 0x7fu)); }
} // namespace applesoft::asm_port
