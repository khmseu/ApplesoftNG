#pragma once

#include <cstdint>
#include <vector>

namespace applesoft::asm_port {

/**
 * @brief 6502 hardware stack and FN-call stack, unified.
 *
 * Hardware stack: AS_LIFO at $0100–$01FF, SP grows downward (6502 convention).
 * FN call stack:  auxiliary AS_LIFO used by AS_FUNCT/AS_FNCDATA for
 * user-defined function call/return context.
 */
class ApplesoftStack {
public:
  // --- Hardware stack ---

  void setStackPointer(std::uint8_t value) noexcept;
  [[nodiscard]] std::uint8_t readStackPointer() const noexcept;

  void pushByte(std::uint8_t value);
  void pushWord(std::uint16_t value);
  [[nodiscard]] std::uint8_t popByte();
  [[nodiscard]] std::uint16_t popWord();
  void popReturnAddress();

  void pushTextPointerAddress();
  void pushCurrentAS_LineNumber();
  void pushToken(std::uint8_t token);

  /// Read a byte at stack address (0x0100 + x + plus).
  [[nodiscard]] std::uint8_t readByteAt(std::uint8_t x,
                                        std::uint8_t plus) const;

  /**
   * @brief Specialized stack probe for 6502 NAME_NOT_FOUND behavior.
   *
   * In the original ROM, this probes for a specific return address on the 6502
   * stack. In the C++ model, this is a placeholder that documents the logic.
   */
  [[nodiscard]] bool probeIsCalledFrom(std::uint16_t address) const;

  // --- FN call stack ---

  void clearFnStack();
  void pushFnByte(std::uint8_t value);
  [[nodiscard]] std::uint8_t peekFnByte() const;
  void popFnByte();
  [[nodiscard]] bool fnStackEmpty() const noexcept;

private:
  std::uint8_t m_sp = 0xffu;
  std::vector<std::uint8_t> m_fn_stack;
};

/// Global accessor — thread_local so each thread owns independent stack state.
ApplesoftStack &theStack();

} // namespace applesoft::asm_port
