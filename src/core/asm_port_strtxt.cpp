#include "core/asm_port_strtxt.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_strlit.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

std::uint16_t read_TXTPTR() {
    return variables_const().readWord(0x00b8u);
}

bool read_carry_flag() {
    return variables_const().carryFlag;
}

void STRLIT_from_word(std::uint16_t address) {
    STRLIT(address);
}

// TODO(asm-port): port POINT label.
void POINT() {}

} // namespace

void STRTXT() {
    // STRING CONSTANT ELEMENT
    // SET Y,A = (TXTPTR)+CARRY

    const std::uint16_t textPointer = read_TXTPTR();
    const std::uint16_t stringStart = static_cast<std::uint16_t>(
        textPointer + (read_carry_flag() ? static_cast<std::uint16_t>(1) : static_cast<std::uint16_t>(0)));

    // BUILD DESCRIPTOR TO STRING
    STRLIT_from_word(stringStart);

    // GET ADDRESS OF DESCRIPTOR IN FAC; POINT TXTPTR AFTER TRAILING QUOTE
    POINT();
}

} // namespace applesoft::asm_port
