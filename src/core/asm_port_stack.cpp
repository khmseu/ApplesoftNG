#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_clear.hpp"

#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);

static std::uint8_t gStackPointer = 0xffu;

void SetStackPointer(std::uint8_t value) {
    gStackPointer = value;
}

std::uint8_t ReadStackPointer() {
    return gStackPointer;
}

void PushByteToStack(std::uint8_t value) {
    WriteProgramByte(static_cast<std::uint16_t>(0x0100u + ReadStackPointer()), value);
    SetStackPointer(static_cast<std::uint8_t>(ReadStackPointer() - 1u));
}

void PushWordToStack(std::uint16_t value) {
    // Push word big-endian (hi first) per 6502 stack convention.
    PushByteToStack(ApplesoftVariables::highByte(value));
    PushByteToStack(ApplesoftVariables::lowByte(value));
}

std::uint8_t PopByteFromStack() {
    SetStackPointer(static_cast<std::uint8_t>(ReadStackPointer() + 1u));
    return ReadProgramByte(static_cast<std::uint16_t>(0x0100u + ReadStackPointer()));
}

std::uint16_t PopWordFromStack() {
    // Pop word in reverse order: lo byte popped first, then hi.
    const std::uint8_t lo = PopByteFromStack();
    const std::uint8_t hi = PopByteFromStack();
    return ApplesoftVariables::makeWord(lo, hi);
}

void PopReturnAddress() {
    (void)PopByteFromStack();
    (void)PopByteFromStack();
}

void PushTextPointerAddress() {
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    const std::uint16_t textPointer = ReadZeroPageWord(kTXTPTR);
    PushByteToStack(ApplesoftVariables::highByte(textPointer));
    PushByteToStack(ApplesoftVariables::lowByte(textPointer));
}

void PushCurrentLineNumber() {
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    const std::uint16_t currentLine = ReadZeroPageWord(kCURLIN);
    PushByteToStack(ApplesoftVariables::highByte(currentLine));
    PushByteToStack(ApplesoftVariables::lowByte(currentLine));
}

void PushTokenTo(std::uint8_t token) {
    PushByteToStack(token);
}

}  // namespace applesoft::asm_port
