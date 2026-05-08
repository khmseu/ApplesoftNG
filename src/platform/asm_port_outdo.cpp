#include "platform/asm_port_outdo.hpp"

#include "core/applesoft_variables.hpp"

#include <cstdint>
#include <iostream>

namespace applesoft::asm_port {

namespace {

// MON_COUT = $fded  (Apple II monitor character-output routine)
// Alias: MON_COUT is a monitor alias for COUT; implemented here as a forwarder
// to the host console.  Apple II display-mode bits (inverse/flash) are stripped
// so that the host terminal receives plain 7-bit ASCII.
void MON_COUT(std::uint8_t a) {
    // Strip Apple II high-bit encoding; $0d (CR) becomes newline.
    const char ch = static_cast<char>(a & 0x7fu);
    if (ch == '\r') {
        std::cout << '\n';
    } else if (ch != '\0') {
        std::cout << ch;
    }
    std::cout.flush();
}

// MON_WAIT = $fca8  (Apple II monitor busy-wait routine)
// Alias: MON_WAIT is a monitor alias for WAIT; speed delay not applicable on a
// modern host.
void MON_WAIT(std::uint8_t /* speed_complement */) {
    // TODO(asm-port): speed delay not meaningful on host; left as no-op.
}

} // namespace

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: OUTDO (inclusive) .. INPUTERR (exclusive)
// Name normalization: none (assembler label OUTDO kept verbatim).
std::uint8_t OUTDO(std::uint8_t a) {
    // ora #$80 — set Apple II high-bit display flag
    a |= 0x80u;

    // cmp #$a0 / bcc L_OUTDO_1 — control characters (A < $a0) skip flash
    if (a >= 0xa0u) {
        // ora FLASH_BIT ($f3): apply flash/inverse mode if active
        a |= variables_const().FLASH_BIT;
    }

    // jsr MON_COUT ($fded) — output character
    MON_COUT(a);

    // and #$7f — strip high bit for return value (used by caller comparisons)
    a &= 0x7fu;

    // lda SPEEDZ ($f1) / jsr MON_WAIT ($fca8) — speed delay
    MON_WAIT(variables_const().SPEEDZ);

    // pla / rts — return A = char with high bit cleared
    return a;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: OUTSP (inclusive) .. OUTQUES (exclusive)
// Name normalization: none (assembler label OUTSP kept verbatim).
// Original uses .byt $2c (BIT abs opcode) to skip OUTQUES's lda and fall
// directly to OUTDO with A=$20.  In C++, OUTDO is called directly.
void OUTSP() {
    OUTDO(static_cast<std::uint8_t>(' ' & 0x7fu)); // $20
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: OUTQUES (inclusive) .. OUTDO (exclusive)
// Name normalization: none (assembler label OUTQUES kept verbatim).
// Falls through to OUTDO in the original; modeled as a direct call in C++.
void OUTQUES() {
    OUTDO(static_cast<std::uint8_t>('?' & 0x7fu)); // $3f
}

} // namespace applesoft::asm_port
