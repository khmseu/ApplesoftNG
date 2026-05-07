#include "core/asm_port_strtxt.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_strlit.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

std::uint8_t read_TXTPTR_low() {
    return variables_const().readByte(0x00b8u);
}

std::uint8_t read_TXTPTR_high() {
    return variables_const().readByte(0x00b9u);
}

bool read_carry_flag() {
    return variables_const().carryFlag;
}

// TODO(asm-port): port POINT label.
void POINT() {}

} // namespace

void STRTXT() {
    // STRING CONSTANT ELEMENT
    // SET Y,A = (TXTPTR)+CARRY

    std::uint8_t a = read_TXTPTR_low();
    std::uint8_t y = read_TXTPTR_high();

    const std::uint16_t sum = static_cast<std::uint16_t>(a) +
        (read_carry_flag() ? static_cast<std::uint16_t>(1) : static_cast<std::uint16_t>(0));
    a = static_cast<std::uint8_t>(sum & 0xffu);
    if ((sum & 0x100u) != 0) {
        ++y;
    }

    // BUILD DESCRIPTOR TO STRING
    STRLIT(a, y);

    // GET ADDRESS OF DESCRIPTOR IN FAC; POINT TXTPTR AFTER TRAILING QUOTE
    POINT();
}

} // namespace applesoft::asm_port
