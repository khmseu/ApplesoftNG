#include "core/asm_port_stack.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_core.hpp"

#include <cassert>
#include <cstdint>

namespace applesoft::asm_port {

// --- Hardware stack ---

void ApplesoftStack::setStackPointer(std::uint8_t value) noexcept {
  m_sp = value;
}

std::uint8_t ApplesoftStack::readStackPointer() const noexcept { return m_sp; }

void ApplesoftStack::pushByte(std::uint8_t value) {
  WriteProgramByte(static_cast<std::uint16_t>(0x0100u + m_sp), value);
  m_sp = static_cast<std::uint8_t>(m_sp - 1u);
}

void ApplesoftStack::pushWord(std::uint16_t value) {
  // Push word big-endian (hi first) per 6502 stack convention.
  pushByte(ApplesoftVariables::highByte(value));
  pushByte(ApplesoftVariables::lowByte(value));
}

std::uint8_t ApplesoftStack::popByte() {
  m_sp = static_cast<std::uint8_t>(m_sp + 1u);
  return ReadProgramByte(static_cast<std::uint16_t>(0x0100u + m_sp));
}

std::uint16_t ApplesoftStack::popWord() {
  // Pop word in reverse order: lo byte popped first, then hi.
  const std::uint8_t lo = popByte();
  const std::uint8_t hi = popByte();
  return ApplesoftVariables::makeWord(lo, hi);
}

void ApplesoftStack::popReturnAddress() {
  (void)popByte();
  (void)popByte();
}

void ApplesoftStack::pushTextPointerAddress() {
  const std::uint16_t textPointer = variables_const().AS_TXTPTR;
  pushByte(ApplesoftVariables::highByte(textPointer));
  pushByte(ApplesoftVariables::lowByte(textPointer));
}

void ApplesoftStack::pushCurrentAS_LineNumber() {
  const std::uint16_t currentAS_Line = variables_const().AS_CURLIN;
  pushByte(ApplesoftVariables::highByte(currentAS_Line));
  pushByte(ApplesoftVariables::lowByte(currentAS_Line));
}

void ApplesoftStack::pushToken(std::uint8_t token) { pushByte(token); }

std::uint8_t ApplesoftStack::readByteAt(std::uint8_t x,
                                        std::uint8_t plus) const {
  const std::uint8_t offset = static_cast<std::uint8_t>(x + plus);
  return ReadProgramByte(static_cast<std::uint16_t>(0x0100u + offset));
}

bool ApplesoftStack::probeIsCalledFrom(std::uint16_t address) const {
  // In original ROM:
  // 3443 T:1087  68           pla (pop return low byte)
  // 3444 T:1088  48           pha (push back)
  // 3445 T:1089  c9 d7        cmp #<FRM_VARIABLE_CALL
  // 3446 T:108b  d0 0f        bne MAKE_NEW_VARIABLE
  // 3447 T:108d  ba           tsx
  // 3448 T:108e  bd 02 01     lda STACK+2,X (look ahead in stack for high byte)
  // 3449 T:1091  c9 0e        cmp #>FRM_VARIABLE_CALL
  //
  // The C++ call model does not populate the 6502 stack with return addresses.
  // However, if we ever model call history in ApplesoftStack, we would check it
  // here.
  (void)address;
  return false;
}

// --- FN call stack ---

void ApplesoftStack::clearFnStack() { m_fn_stack.clear(); }

void ApplesoftStack::pushFnByte(std::uint8_t value) {
  m_fn_stack.push_back(value);
}

std::uint8_t ApplesoftStack::peekFnByte() const {
  assert(!m_fn_stack.empty());
  return m_fn_stack.back();
}

void ApplesoftStack::popFnByte() {
  assert(!m_fn_stack.empty());
  m_fn_stack.pop_back();
}

bool ApplesoftStack::fnStackEmpty() const noexcept {
  return m_fn_stack.empty();
}

// --- Global accessor ---

ApplesoftStack &theStack() {
  thread_local ApplesoftStack g_stack;
  return g_stack;
}

} // namespace applesoft::asm_port
