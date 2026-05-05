#include "core/asm_port_strtxt.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

// TODO(asm-port): map TXTPTR low byte access to interpreter memory/register state.
std::uint8_t read_TXTPTR_low() { return 0; }

// TODO(asm-port): map TXTPTR high byte access to interpreter memory/register state.
std::uint8_t read_TXTPTR_high() { return 0; }

// TODO(asm-port): read CPU carry flag from arithmetic state.
bool read_carry_flag() { return false; }

// TODO(asm-port): port STRLIT label.
void STRLIT(std::uint8_t /*a*/, std::uint8_t /*y*/) {}

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
