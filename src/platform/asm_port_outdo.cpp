#include "platform/asm_port_outdo.hpp"

#include "core/applesoft_variables.hpp"

#include <cstdint>
#include <iostream>

namespace applesoft::asm_port {

namespace {

// MON_VIDOUT -- the ROM's VIDOUT ($fb3c in display2.o65): the actual character
// renderer that strips the Apple II high-bit encoding and maps monitor control
// codes ($8d CR, $8a LF, $88 BS, $87 BEL) to host terminal equivalents.
// Called from MON_COUT via LFB78.  File-local: implementation detail only.
void MON_VIDOUT(std::uint8_t a) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // Labels: VIDOUT (inclusive) .. ESC1 (exclusive)
    // Name normalization: VIDOUT -> MON_VIDOUT (monitor label gets MON_ prefix).
    //
    // VIDOUT  cmp #$a0 / bcs STOADV -- printable high-bit chars fall through to output.
    // Control chars with bit 7 set ($80-$9f): dispatch on code.
    // Strip high bit before passing to host terminal.

    const std::uint8_t ch = static_cast<std::uint8_t>(a & 0x7fu);

    switch (ch) {
    case 0x07u: // BEL  ($87 with high bit set)
        std::cout << '\a';
        break;
    case 0x08u: // BS   ($88)
        std::cout << '\b';
        break;
    case 0x0au: // LF   ($8a)
        std::cout << '\n';
        break;
    case 0x0du: // CR   ($8d) -- Apple II carriage return, maps to newline
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
void MON_WAIT(std::uint8_t a) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // Labels: WAIT (inclusive) .. NXTA4 (exclusive)
    // Name normalization: WAIT -> MON_WAIT (monitor label gets MON_ prefix).
    //
    // WAIT  sec                     ; C=1 for first inner SBC (no borrow penalty)
    // WAIT2 pha                     ; save outer counter
    // WAIT3 sbc #1 / bne WAIT3      ; inner loop: outer..0
    //       pla / sbc #1 / bne WAIT2; outer loop: a..0
    // Total inner iterations ~a*(a+1)/2  (~(13+27/2*a+5/2*a^2) cycles on real hw)
    // On a modern host the spin produces no meaningful delay; volatile prevents
    // the compiler from eliding the loop.
    volatile std::uint8_t outer = a;
    do {
        volatile std::uint8_t inner = outer;
        do {
            inner = static_cast<std::uint8_t>(inner - 1u); // WAIT3: sbc #1
        } while (inner != 0u);                              //        bne WAIT3
        outer = static_cast<std::uint8_t>(outer - 1u);     // pla; sbc #1
    } while (outer != 0u);                                  // bne WAIT2
}

} // namespace

// MON_COUT = $fded  (Apple II monitor character-output routine)
// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst label COUT.
// All monitor labels carry a virtual MON_ prefix in C++; COUT -> MON_COUT.
void MON_COUT(std::uint8_t a) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: COUT (inclusive) .. COUT1 (exclusive)
    // Name normalization: COUT -> MON_COUT (monitor label gets MON_ prefix).
    //
    // COUT      jmp ($36)             ; dispatch through CSW output vector ($36/$37)
    //                                 ; default target: COUT1 at $fdf0
    // COUT1     cmp #$a0              ; printable char (A >= $a0)?
    //           bcc COUTZ             ;   no: control char -- skip INVFLG masking
    //           and $32               ;   yes: mask with INVFLG ($ff=normal, $3f=inverse)
    // COUTZ     jsr LFB78             ; LFB78 -> jmp VIDOUT: write to screen/terminal

    // COUT1: apply INVFLG mask for printable (high-bit) chars only.
    if (a >= 0xa0u) {
        a &= variables_const().MON_INVFLG; // and $32
    }

    // LFB78 -> VIDOUT: write character to host terminal.
    MON_VIDOUT(a);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: OUTDO (inclusive) .. INPUTERR (exclusive)
// Name normalization: none (assembler label OUTDO kept verbatim).
std::uint8_t OUTDO(std::uint8_t a) {
    // ora #$80 -- set Apple II high-bit display flag
    a |= 0x80u;

    // cmp #$a0 / bcc L_OUTDO_1 -- control characters (A < $a0) skip flash
    if (a >= 0xa0u) {
        // ora FLASH_BIT ($f3): apply flash/inverse mode if active
        a |= variables_const().FLASH_BIT;
    }

    // jsr MON_COUT ($fded) -- output character
    MON_COUT(a);

    // and #$7f -- strip high bit for return value (used by caller comparisons)
    a &= 0x7fu;

    // lda SPEEDZ ($f1) / jsr MON_WAIT ($fca8) -- speed delay
    MON_WAIT(variables_const().SPEEDZ);

    // pla / rts -- return A = char with high bit cleared
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
