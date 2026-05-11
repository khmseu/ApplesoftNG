#include "core/asm_port_strlt2.hpp"

#include "core/applesoft_variables.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

std::uint8_t read_CHARAC() {
    return variables_const().readByte(0x000du);
}

std::uint8_t read_ENDCHR() {
    return variables_const().readByte(0x000eu);
}

void write_STRNG1(std::uint16_t value) {
    variables().writeWord(0x00abu, value);
}

void write_FAC(std::uint8_t v) {
    variables().writeByte(0x009du, v);
}

void write_FAC_pointer(std::uint16_t value) {
    variables().writeWord(0x009eu, value);
}

void write_STRNG2(std::uint16_t value) {
    variables().writeWord(0x00adu, value);
}

// TODO(asm-port): port STRINI label.
void STRINI(std::uint8_t /*length*/) {}

// TODO(asm-port): port MOVSTR label.
void MOVSTR(std::uint8_t /*x*/, std::uint8_t /*y*/) {}

void MOVSTR(std::uint16_t address) {
    MOVSTR(ApplesoftVariables::lowByte(address), ApplesoftVariables::highByte(address));
}

// TODO(asm-port): port PUTNEW label.
void PUTNEW() {}

} // namespace

void STRLT2(std::uint16_t address) {
    // BUILD A DESCRIPTOR FOR STRING STARTING AT address
    // AND TERMINATED BY $00, (CHARAC), OR (ENDCHR)

    const std::uint16_t start = address;
    const auto startPtr = variables_const().pointer(start);
    write_STRNG1(start);
    write_FAC_pointer(start);

    std::uint8_t length = 0;
    bool include_quote_in_length = false;

    // Find end of string, terminated by $00 or alternate terminators.
    while (true) {
        const std::uint8_t ch = startPtr.read(length);
        if (ch == 0) {
            break;
        }
        if (ch == read_CHARAC() || ch == read_ENDCHR()) {
            include_quote_in_length = (ch == static_cast<std::uint8_t>(0x22));
            break;
        }
        ++length;
    }

    if (include_quote_in_length) {
        ++length;
    }

    write_FAC(length);

    const std::uint16_t endAddress = startPtr.advanced(length).address();
    write_STRNG2(endAddress);

    // If source is not on page 0 or page 2, branch directly to PUTNEW.
    const std::uint8_t startPage = ApplesoftVariables::highByte(start);
    if (startPage != 0 && startPage != 2) {
        PUTNEW();
        return;
    }

    // For page 0/2 source, allocate and move string before PUTNEW handling.
    STRINI(length);
    MOVSTR(start);

    // Fall-through target in original control flow is PUTNEW.
    PUTNEW();
}

} // namespace applesoft::asm_port
