#include "platform/asm_port_getln.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_characters.hpp"
#include "core/asm_port_core.hpp"
#include "core/asm_port_inlin2.hpp"
#include "platform/asm_port_outdo.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

std::uint8_t read_prompt_char() { return variables_const().MON_PROMPT; }

void write_input_buffer(std::uint8_t index, std::uint8_t value) {
  variables().writeByte(static_cast<std::uint16_t>(
                            ApplesoftVariables::ADDR_AS_INPUT_BUFFER + index),
                        value);
}

std::uint8_t read_input_buffer(std::uint8_t index) {
  return variables_const().readByte(static_cast<std::uint16_t>(
      ApplesoftVariables::ADDR_AS_INPUT_BUFFER + index));
}

std::uint8_t read_screen_char_via_28_y() {
  const std::uint8_t y = variables_const().MON_CH;
  const std::uint16_t base = variables_const().MON_BASL;
  return variables_const().readByte(static_cast<std::uint16_t>(base + y));
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_RDCHAR (inclusive) .. MON_NOTCR (exclusive)
// Name normalization: none (assembler label MON_RDCHAR kept verbatim).
std::uint8_t MON_RDCHAR() {
  // ESC ($9B) branches back through ESC to read another key; model as a loop.
  while (true) {
    const std::uint8_t key = MON_RDKEY();
    if (key != kCharEscapeHigh) {
      return key;
    }
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_CLREOL (inclusive) .. MON_WAIT (exclusive)
// Name normalization: none (assembler label MON_CLREOL kept verbatim).
void MON_CLREOL() {
  constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);
  const std::uint8_t columnStart = variables_const().MON_CH;
  const std::uint8_t width = variables_const().MON_WNDWDTH;
  const std::uint16_t base = variables_const().MON_BASL;

  for (std::uint8_t column = columnStart; column < width; ++column) {
    variables().writeByte(static_cast<std::uint16_t>(base + column), kBlank);
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_CROUT (inclusive) .. MON_PRA1 (exclusive)
// Name normalization: none (assembler label MON_CROUT kept verbatim).
void MON_CROUT() { MON_COUT(kCharCarriageReturnHigh); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_GETLNZ (inclusive) .. MON_BCKSPC (exclusive)
// Name normalization: none (assembler label MON_GETLNZ kept verbatim).
std::uint8_t MON_GETLNZ() {
  constexpr std::uint8_t kInitialBufferIndex = 1u;
  MON_CROUT();
  MON_COUT(read_prompt_char());
  // AS_GETLNZ falls through into AS_GETLN, which immediately executes `ldx
  // #$01`.
  return kInitialBufferIndex;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_NOTCR (inclusive) .. MON_GETLNZ (exclusive)
// Name normalization: none (assembler label MON_NOTCR kept verbatim).
void MON_NOTCR(std::uint8_t &x) {
  constexpr std::uint8_t kBackspace = 0x88u;
  constexpr std::uint8_t kCtrlX = 0x98u;
  constexpr std::uint8_t kMargin = 0xf8u;
  constexpr std::uint8_t kCancelSlash = static_cast<std::uint8_t>('\\' | 0x80u);

  const std::uint8_t savedInv = variables_const().MON_INVFLG;
  variables().MON_INVFLG = 0xffu;
  // X is an 8-bit page-local index into $0200..$02FF; 0xff is the last valid
  // slot. The subsequent ++x wrap to 0 intentionally triggers the
  // cancel/restart path.
  const std::uint8_t current = read_input_buffer(x);
  MON_COUT(current);
  variables().MON_INVFLG = savedInv;

  if (current == kBackspace || current == kCtrlX) {
    if (x == 0u) {
      x = MON_GETLNZ();
      return;
    }
    --x;
    return;
  }

  if (x >= kMargin) {
    MON_BELL();
  }

  ++x;
  if (x == 0u) {
    MON_COUT(kCancelSlash);
    x = MON_GETLNZ();
  }
}

} // namespace

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_GETLN (inclusive) .. MON_CROUT (exclusive)
// Name normalization: none (assembler label MON_GETLN kept verbatim).
std::uint8_t MON_GETLN() {
  constexpr std::uint8_t kCtrlU = 0x95u;
  constexpr std::uint8_t kAS_LowercaseThreshold = 0xe0u;

  std::uint8_t x = MON_GETLNZ();

  while (true) {
    std::uint8_t ch = MON_RDCHAR();
    if (ch == kCtrlU) {
      ch = read_screen_char_via_28_y();
    }

    if (ch >= kAS_LowercaseThreshold) {
      ch = static_cast<std::uint8_t>(ch & 0xdfu);
    }

    write_input_buffer(x, ch);
    if (ch == kCharCarriageReturnHigh) {
      MON_CLREOL();
      return x;
    }

    MON_NOTCR(x);
  }
}

} // namespace applesoft::asm_port
