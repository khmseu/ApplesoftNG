#include "core/asm_port_inlin2.hpp"
#include "core/applesoft_variables.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

constexpr std::uint16_t kInputBufferAddress = 0x0200;

void write_MON_PROMPT(std::uint8_t v) {
    variables().writeByte(0x0033u, v);
}

// TODO(asm-port): port GETLN monitor call; return line length in X.
std::uint8_t GETLN() { return 0; }

// MON_GETLN is a monitor alias for GETLN.
std::uint8_t MON_GETLN() {
    return GETLN();
}

void write_INPUT_BUFFER(std::uint8_t index, std::uint8_t v) {
    variables().writeByte(static_cast<std::uint16_t>(kInputBufferAddress + index), v);
}

std::uint8_t read_INPUT_BUFFER_minus_1(std::uint8_t index) {
    return variables_const().readByte(static_cast<std::uint16_t>((kInputBufferAddress - 1u) + index));
}

void write_INPUT_BUFFER_minus_1(std::uint8_t index, std::uint8_t v) {
    variables().writeByte(static_cast<std::uint16_t>((kInputBufferAddress - 1u) + index), v);
}

// TODO(asm-port): port RDKEY monitor call; read a key from the monitor.
std::uint8_t RDKEY() { return 0; }

// MON_RDKEY is a monitor alias for RDKEY.
std::uint8_t MON_RDKEY() {
    return RDKEY();
}

} // namespace

Inlin2Result INLIN2(std::uint8_t x) {
    // READ A LINE, AND STRIP OFF SIGN BITS.
    write_MON_PROMPT(x);

    std::uint8_t length = MON_GETLN();
    if (length >= static_cast<std::uint8_t>(239)) {
        length = static_cast<std::uint8_t>(239);
    }

    // Mark end-of-line with a trailing NUL byte.
    write_INPUT_BUFFER(length, 0);

    // Strip high bits from INPUT_BUFFER[0..length-1].
    while (length != 0) {
        std::uint8_t ch = read_INPUT_BUFFER_minus_1(length);
        ch = static_cast<std::uint8_t>(ch & 0x7fu);
        write_INPUT_BUFFER_minus_1(length, ch);
        --length;
    }

    // A=0, YX points at INPUT_BUFFER-1.
    return Inlin2Result{
        0,
        static_cast<std::uint8_t>((kInputBufferAddress - 1u) & 0xffu),
        static_cast<std::uint8_t>(((kInputBufferAddress - 1u) >> 8) & 0xffu)
    };
}

std::uint8_t INCHR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: INCHR (inclusive) .. PARSE_INPUT_LINE (exclusive)
    // Name normalization: none (assembler label INCHR kept verbatim).

    return MON_RDKEY();
}

} // namespace applesoft::asm_port