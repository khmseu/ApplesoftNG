#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

class IOPorts {
public:
  static constexpr std::uint16_t ADDR_BASE = 0xc000u;
  static constexpr std::uint16_t ADDR_END = 0xd000u;
  static constexpr std::uint16_t ADDR_AS_KEYBOARD = 0xc000u;
  static constexpr std::uint16_t ADDR_AS_KEYBOARD_STROBE = 0xc010u;
  static constexpr std::uint16_t ADDR_AS_SW_MIXCLR = 0xc052u;
  static constexpr std::uint16_t ADDR_AS_SW_MIXSET = 0xc053u;
  static constexpr std::uint16_t ADDR_AS_SW_LOWSCR = 0xc054u;
  static constexpr std::uint16_t ADDR_AS_SW_HISCR = 0xc055u;
  static constexpr std::uint16_t ADDR_AS_SW_LORES = 0xc056u;
  static constexpr std::uint16_t ADDR_AS_SW_HIRES = 0xc057u;
  static constexpr std::uint16_t ADDR_SW_AN0 = 0xc058u;
  static constexpr std::uint16_t ADDR_SW_AN1 = 0xc05au;
  static constexpr std::uint16_t ADDR_SW_AN2 = 0xc05du;
  static constexpr std::uint16_t ADDR_SW_AN3 = 0xc05fu;
  static constexpr std::uint16_t ADDR_MON_TAPE_INPUT = 0xc060u;
  static constexpr std::uint16_t ADDR_MON_TAPE_OUTPUT = 0xc020u;
  static constexpr std::uint16_t ADDR_AS_SW_TXTCLR = 0xc050u;
  static constexpr std::uint16_t ADDR_SW_TXTSET = 0xc051u;
  static constexpr std::uint16_t ADDR_PADDLE_0 = 0xc064u;
  static constexpr std::uint16_t ADDR_PADDLE_TRIGGER = 0xc070u;
  static constexpr std::uint16_t ADDR_ROM_AS_SIGNATURE = 0xcfffu;

  std::uint8_t readByte(std::uint16_t address) const;
  void writeByte(std::uint16_t address, std::uint8_t value);

private:
  std::array<std::uint8_t, ADDR_END - ADDR_BASE> ports_{};
};

IOPorts &ioPorts();
const IOPorts &ioPorts_const();

} // namespace applesoft::asm_port
