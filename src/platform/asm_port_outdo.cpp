#include "platform/asm_port_outdo.hpp"

#include "core/applesoft_variables.hpp"

#include <cstdint>
#include <iostream>

namespace applesoft::asm_port {

namespace {

// MON_COUT = $fded  (Apple II monitor character-output routine)
// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst label COUT.
// All monitor labels carry a virtual MON_ prefix in C++; COUT -> MON_COUT.
// Apple II display-mode bits (inverse/flash) are stripped so that the host
// terminal receives plain 7-bit ASCII.
void MON_COUT(std::uint8_t a) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: COUT (inclusive) .. COUT1 (exclusive)
    // Name normalization: COUT -> MON_COUT (monitor label gets MON_ prefix).
    //
    // Strip Apple II high-bit display flag; translate monitor control codes to
    // portable equivalents before writing to the host console.

    const std::uint8_t ch = static_cast<std::uint8_t>(a & 0x7fu);

    switch (ch) {
    case 0x07u: // BEL  ($87 with high bit)
        std::cout << '\a';
        break;
    case 0x08u: // BS   ($88)
        std::cout << '\b';
        break;
    case 0x0au: // LF   ($8a)
        std::cout << '\n';
        break;
    case 0x0du: // CR   ($8d) — Apple II carriage return, maps to newline
        std::cout << '\n';
        break;
    default:
        if (ch >= 0x20u && ch < 0x7fu) {
            std::cout << static_cast<char>(ch);
        }
        break;
    }
}

// MON_WAIT = $fca8  (Apple II monitor busy-wait routine)
// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst label WAIT.
// All monitor labels carry a virtual MON_ prefix in C++; WAIT -> MON_WAIT.
// Speed delay is not meaningful on a modern host; implemented as a no-op.
void MON_WAIT(std::uint8_t speed_complement) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // Labels: WAIT (inclusive) .. NXTA4 (exclusive)
    // Name normalization: WAIT -> MON_WAIT (monitor label gets MON_ prefix).
    //
    // ROM spins a nested delay loop proportional to speed_complement.
    // On a modern host the loop is meaningless; we consume the argument to
    // preserve the calling-convention contract without any actual delay.
    static volatile std::uint8_t sink;
    sink = speed_complement;
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
