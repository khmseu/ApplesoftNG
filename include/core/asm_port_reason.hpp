#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

struct AS_REASONState {
  // Target address in A/Y on entry and return.
  std::uint8_t a = 0;
  std::uint8_t y = 0;

  // AS_FRETOP low/high bytes used for the space check.
  std::uint8_t fretopAS_Lo = 0;
  std::uint8_t fretopHi = 0;

  // Minimal scratch range preserved across garbage collection.
  std::array<std::uint8_t, 10> temp1ToFacExclusive{};

  // X is only observed when AS_MEMERR is taken.
  std::uint8_t x = 0;

  bool garbageCollected = false;
  bool memerrCalled = false;
};

struct AS_REASONResult {
  bool ok = false;
  std::uint8_t a = 0;
  std::uint8_t y = 0;
  std::uint8_t x = 0;
};

// Ensure the array-growth address in A/Y stays below AS_FRETOP, attempting
// AS_GARBAG first.
AS_REASONResult AS_REASON(AS_REASONState &state);

} // namespace applesoft::asm_port