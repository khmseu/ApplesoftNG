#include "core/asm_port_inlin2.hpp"
#include "core/applesoft_variables.hpp"
#include "platform/asm_port_getln.hpp"

#include <cstdint>
#include <iostream>

namespace applesoft::asm_port {
namespace {

constexpr std::uint16_t kInputBufferAddress = ApplesoftVariables::ADDR_INPUT_BUFFER;

void write_MON_PROMPT(std::uint8_t v) {
    variables().writeByte(ApplesoftVariables::ZP_MON_PROMPT, v);
}

void write_INPUT_BUFFER(std::uint8_t index, std::uint8_t v) {
    variables().pointer(kInputBufferAddress).write(v, index);
}

std::uint8_t read_INPUT_BUFFER_minus_1(std::uint8_t index) {
    return variables_const().pointer(static_cast<std::uint16_t>(kInputBufferAddress - 1u)).read(index);
}

void write_INPUT_BUFFER_minus_1(std::uint8_t index, std::uint8_t v) {
    variables().pointer(static_cast<std::uint16_t>(kInputBufferAddress - 1u)).write(v, index);
}

} // namespace

// MON_RDKEY: monitor label RDKEY (keyin.o65.lst).
// All monitor labels carry a virtual MON_ prefix in C++; RDKEY -> MON_RDKEY.
std::uint8_t MON_RDKEY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/keyin.o65.lst
    // Labels: RDKEY (inclusive) .. KEYIN (exclusive)
    // Name normalization: monitor label RDKEY mapped to MON_RDKEY in C++.

    const int ch = std::cin.get();
    if (ch == EOF) {
        return 0u;
    }

    return static_cast<std::uint8_t>(ch);
}

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
    const std::uint16_t inputBufferMinus1 = static_cast<std::uint16_t>(kInputBufferAddress - 1u);
    return Inlin2Result::fromAddress(0u, inputBufferMinus1);
}

std::uint8_t INCHR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: INCHR (inclusive) .. PARSE_INPUT_LINE (exclusive)
    // Name normalization: none (assembler label INCHR kept verbatim).

    return MON_RDKEY();
}

} // namespace applesoft::asm_port