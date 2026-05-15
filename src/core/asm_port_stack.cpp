#include "core/asm_port_stack.hpp"
#include "core/applesoft_variables.hpp"

#include <cassert>
#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);

// --- Hardware stack ---

void ApplesoftStack::setStackPointer(std::uint8_t value) noexcept {
    m_sp = value;
}

std::uint8_t ApplesoftStack::readStackPointer() const noexcept {
    return m_sp;
}

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
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    const std::uint16_t textPointer = ReadZeroPageWord(kTXTPTR);
    pushByte(ApplesoftVariables::highByte(textPointer));
    pushByte(ApplesoftVariables::lowByte(textPointer));
}

void ApplesoftStack::pushCurrentLineNumber() {
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    const std::uint16_t currentLine = ReadZeroPageWord(kCURLIN);
    pushByte(ApplesoftVariables::highByte(currentLine));
    pushByte(ApplesoftVariables::lowByte(currentLine));
}

void ApplesoftStack::pushToken(std::uint8_t token) {
    pushByte(token);
}

std::uint8_t ApplesoftStack::readByteAt(std::uint8_t x, std::uint8_t plus) const {
    const std::uint8_t offset = static_cast<std::uint8_t>(x + plus);
    return ReadProgramByte(static_cast<std::uint16_t>(0x0100u + offset));
}

// --- FN call stack ---

void ApplesoftStack::clearFnStack() {
    m_fn_stack.clear();
}

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

ApplesoftStack& theStack() {
    thread_local ApplesoftStack g_stack;
    return g_stack;
}

}  // namespace applesoft::asm_port
