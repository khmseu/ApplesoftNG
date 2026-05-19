#include "core/asm_port_inlin2.hpp"
#include "core/applesoft_variables.hpp"
#include "core/io_ports.hpp"
#include "core/jump_table.hpp"
#include "platform/asm_port_getln.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

constexpr std::uint16_t kInputBufferAddress =
    ApplesoftVariables::ADDR_AS_INPUT_BUFFER;

void write_MON_PROMPT(std::uint8_t v) { variables().MON_PROMPT = v; }

void write_AS_INPUT_BUFFER(std::uint8_t index, std::uint8_t v) {
  variables().pointer(kInputBufferAddress).write(v, index);
}

std::uint8_t read_AS_INPUT_BUFFER_minus_1(std::uint8_t index) {
  return variables_const()
      .pointer(static_cast<std::uint16_t>(kInputBufferAddress - 1u))
      .read(index);
}

void write_AS_INPUT_BUFFER_minus_1(std::uint8_t index, std::uint8_t v) {
  variables()
      .pointer(static_cast<std::uint16_t>(kInputBufferAddress - 1u))
      .write(v, index);
}

std::uint16_t readZeroPageWord(std::uint8_t address) {
  return ApplesoftVariables::makeWord(
      variables_const().readByte(address),
      variables_const().readByte(static_cast<std::uint8_t>(address + 1u)));
}
} // namespace

std::uint8_t MON_KEYIN() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: KEYIN (inclusive) .. ESC (exclusive)
  // Name normalization: KEYIN helper target kept internal; public entry is
  // useMonitorInputVector.

  constexpr std::uint8_t kMON_RNDL = ApplesoftVariables::ZP_MON_RNDL;
  constexpr std::uint8_t kMON_RNDH = ApplesoftVariables::ZP_MON_RNDH;

  while (true) {
    const std::uint8_t rndAS_Lo =
        static_cast<std::uint8_t>(variables_const().readByte(kMON_RNDL) + 1u);
    variables().writeByte(kMON_RNDL, rndAS_Lo);
    if (rndAS_Lo == 0u) {
      const std::uint8_t rndHi =
          static_cast<std::uint8_t>(variables_const().readByte(kMON_RNDH) + 1u);
      variables().writeByte(kMON_RNDH, rndHi);
    }

    const std::uint8_t keyboardValue =
        ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD);
    if ((keyboardValue & 0x80u) != 0u) {
      (void)ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD_STROBE);
      ioPorts().writeByte(IOPorts::ADDR_AS_KEYBOARD,
                          static_cast<std::uint8_t>(keyboardValue & 0x7fu));
      return keyboardValue;
    }
  }
}

// MON_RDKEY: monitor label RDKEY (keyin.o65.lst).
// All monitor labels carry a virtual MON_ prefix in C++; RDKEY -> MON_RDKEY.
std::uint8_t MON_RDKEY() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: RDKEY (inclusive) .. RDCHAR (exclusive)
  // Name normalization: monitor label RDKEY mapped to MON_RDKEY in C++.

  constexpr std::uint8_t kMON_BASL = ApplesoftVariables::ZP_MON_BASL;
  // RDKEY prologue: fetch cursor row/column source char through ($28),Y,
  // then rewrite it in flashing form while waiting for input.
  const std::uint8_t y = variables_const().MON_CH;
  const std::uint16_t screenBase = ApplesoftVariables::makeWord(
      variables_const().readByte(kMON_BASL),
      variables_const().readByte(static_cast<std::uint16_t>(kMON_BASL + 1u)));
  const std::uint16_t cursorAddress =
      static_cast<std::uint16_t>(screenBase + y);

  const std::uint8_t originalChar = variables_const().readByte(cursorAddress);
  const std::uint8_t flashingChar =
      static_cast<std::uint8_t>((originalChar & 0x3fu) | 0x40u);
  variables().writeByte(cursorAddress, flashingChar);

  const std::uint8_t keyboardValue =
      ApplesoftNG::ExternalJumpDispatcher::JumpFromWord<std::uint8_t>(
          (ApplesoftVariables::ZP_MON_KSW));

  // KEYIN epilogue: restore original screen char, clear keyboard strobe, return
  // keycode.
  variables().writeByte(cursorAddress, originalChar);
  return keyboardValue;
}

Inlin2Result AS_INLIN2(std::uint8_t x) {
  // AS_READ A AS_LINE, AND AS_STRIP OFF AS_SIGN BITS.
  write_MON_PROMPT(x);

  std::uint8_t length = MON_GETLN();
  if (length >= static_cast<std::uint8_t>(239)) {
    length = static_cast<std::uint8_t>(239);
  }

  // Mark end-of-line with a trailing NUL byte.
  write_AS_INPUT_BUFFER(length, 0);

  // Strip high bits from AS_INPUT_BUFFER[0..length-1].
  while (length != 0) {
    std::uint8_t ch = read_AS_INPUT_BUFFER_minus_1(length);
    ch = static_cast<std::uint8_t>(ch & 0x7fu);
    write_AS_INPUT_BUFFER_minus_1(length, ch);
    --length;
  }

  // A=0, YX points at AS_INPUT_BUFFER-1.
  const std::uint16_t inputBufferMinus1 =
      static_cast<std::uint16_t>(kInputBufferAddress - 1u);
  return Inlin2Result::fromAddress(0u, inputBufferMinus1);
}

std::uint8_t AS_INCHR() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_INCHR (inclusive) .. AS_PARSE_INPUT_LINE (exclusive)
  // name normalization: none (assembler label AS_INCHR kept verbatim).

  return ApplesoftNG::ExternalJumpDispatcher::JumpFromWord<std::uint8_t>(
             (ApplesoftVariables::ZP_MON_KSW)) &
         0x7fu;
}

} // namespace applesoft::asm_port