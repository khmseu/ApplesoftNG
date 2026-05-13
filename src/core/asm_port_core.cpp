#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"

#include <cstdint>

namespace applesoft::asm_port {

void SetTextPointer(std::uint16_t address) {
    variables().writeWord(ApplesoftVariables::ZP_TXTPTR, address);
}

void ClearErrFlag() {
    variables().writeByte(ApplesoftVariables::ZP_ERRFLG, 0);
}

void MarkDirectMode() {
    variables().writeByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_CURLIN + 1u), 0xffu);
}

std::uint8_t ReadZeroPageByte(std::uint8_t address) {
    return variables_const().readByte(address);
}

void WriteZeroPageByte(std::uint8_t address, std::uint8_t value) {
    variables().writeByte(address, value);
}

void WriteZeroPageWord(std::uint8_t address, std::uint16_t value) {
    variables().writeWord(address, value);
}

std::uint16_t ReadZeroPageWord(std::uint8_t address) {
    return variables_const().readWord(address);
}

}  // namespace applesoft::asm_port
