#include "platform/asm_port_getln.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_inlin2.hpp"
#include "platform/asm_port_outdo.hpp"

#include <cstdint>

namespace applesoft::asm_port {
void MON_COUT(std::uint8_t a);
// Implemented in src/core/asm_port_core.cpp (monitor BELL routine).
void MON_BELL();
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
// AS_Labels: COUT (inclusive) .. COUT1 (exclusive)
// Name normalization: none (assembler label COUT kept verbatim).
void COUT(std::uint8_t value) { MON_COUT(value); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: RDCHAR (inclusive) .. NOTCR (exclusive)
// Name normalization: none (assembler label RDCHAR kept verbatim).
std::uint8_t RDCHAR() {
  // ESC ($9B) branches back through ESC to read another key; model as a loop.
  constexpr std::uint8_t kEsc = 0x9bu;
  while (true) {
    const std::uint8_t key = MON_RDKEY();
    if (key != kEsc) {
      return key;
    }
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: CLREOL (inclusive) .. AS_WAIT (exclusive)
// Name normalization: none (assembler label CLREOL kept verbatim).
void CLREOL() {
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
// AS_Labels: CROUT (inclusive) .. PRA1 (exclusive)
// Name normalization: none (assembler label CROUT kept verbatim).
void CROUT() { COUT(0x8du); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GETLNZ (inclusive) .. BCKSPC (exclusive)
// Name normalization: none (assembler label AS_GETLNZ kept verbatim).
std::uint8_t AS_GETLNZ() {
  constexpr std::uint8_t kInitialBufferIndex = 1u;
  CROUT();
  COUT(read_prompt_char());
  // AS_GETLNZ falls through into AS_GETLN, which immediately executes `ldx
  // #$01`.
  return kInitialBufferIndex;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: NOTCR (inclusive) .. AS_GETLNZ (exclusive)
// Name normalization: none (assembler label NOTCR kept verbatim).
void NOTCR(std::uint8_t &x) {
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
  COUT(current);
  variables().MON_INVFLG = savedInv;

  if (current == kBackspace || current == kCtrlX) {
    if (x == 0u) {
      x = AS_GETLNZ();
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
    COUT(kCancelSlash);
    x = AS_GETLNZ();
  }
}

} // namespace

std::uint8_t MON_GETLN() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_GETLN (inclusive) .. CROUT (exclusive)
  // Name normalization: AS_GETLN -> MON_GETLN (monitor label gets MON_ prefix).
  constexpr std::uint8_t kCtrlU = 0x95u;
  constexpr std::uint8_t kCarriageReturn = 0x8du;
  constexpr std::uint8_t kAS_LowercaseThreshold = 0xe0u;

  std::uint8_t x = AS_GETLNZ();

  while (true) {
    std::uint8_t ch = RDCHAR();
    if (ch == kCtrlU) {
      ch = read_screen_char_via_28_y();
    }

    if (ch >= kAS_LowercaseThreshold) {
      ch = static_cast<std::uint8_t>(ch & 0xdfu);
    }

    write_input_buffer(x, ch);
    if (ch == kCarriageReturn) {
      CLREOL();
      return x;
    }

    NOTCR(x);
  }
}

} // namespace applesoft::asm_port
