#pragma once

#include <array>
#include <cstdint>

#include "core/applesoft_variables.hpp"
#include "core/asm_port_reason.hpp"

namespace applesoft::asm_port {

struct AS_BLTUState {
  // Inputs carried in A/Y on entry.
  std::uint8_t a = 0;
  std::uint8_t y = 0;

  // Minimal AS_REASON state needed by the AS_BLTU prologue.
  AS_REASONState reason{};

  // Destination state updated by AS_BLTU.
  std::uint8_t strendAS_Lo = 0;
  std::uint8_t strendHi = 0;

  std::uint16_t strend() const {
    return ApplesoftVariables::makeWord(strendAS_Lo, strendHi);
  }

  void setStrend(std::uint16_t value) {
    strendAS_Lo = static_cast<std::uint8_t>(value & 0x00ffu);
    strendHi = static_cast<std::uint8_t>(value >> 8);
  }
};

struct AS_BLTUResult {
  // A/Y after AS_REASON and stores; AS_BLTU2 consumes these next.
  std::uint8_t a = 0;
  std::uint8_t y = 0;
};

// Minimal state needed by AS_BLTU2 while broader runtime memory wiring is
// pending.
struct AS_BLTU2State {
  // Flat 6502 address space used by the backward copy routine.
  std::array<std::uint8_t, 65536> memory{};

  // Registers on entry to AS_BLTU2.
  std::uint8_t x = 0;
  std::uint8_t y = 0;

  // Zero-page pointers used by the ROM routine.
  std::uint8_t hightrAS_Lo = 0;
  std::uint8_t hightrHi = 0;
  std::uint8_t highdsAS_Lo = 0;
  std::uint8_t highdsHi = 0;

  std::uint16_t sourcePointer() const {
    return ApplesoftVariables::makeWord(hightrAS_Lo, hightrHi);
  }

  void setSourcePointer(std::uint16_t value) {
    hightrAS_Lo = static_cast<std::uint8_t>(value & 0x00ffu);
    hightrHi = static_cast<std::uint8_t>(value >> 8);
  }

  std::uint16_t destinationPointer() const {
    return ApplesoftVariables::makeWord(highdsAS_Lo, highdsHi);
  }

  void setDestinationPointer(std::uint16_t value) {
    highdsAS_Lo = static_cast<std::uint8_t>(value & 0x00ffu);
    highdsHi = static_cast<std::uint8_t>(value >> 8);
  }
};

struct AS_BLTU2Result {
  // Registers on return.
  std::uint8_t x = 0;
  std::uint8_t y = 0;
};

// Execute the AS_BLTU prologue before control falls into AS_BLTU2.
AS_BLTUResult AS_BLTU(AS_BLTUState &state);

// Copy bytes backward from AS_HIGHTR to AS_HIGHDS, finishing with full 256-byte
// pages.
AS_BLTU2Result AS_BLTU2(AS_BLTU2State &state);

} // namespace applesoft::asm_port
