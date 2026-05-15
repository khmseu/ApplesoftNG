#include "platform/asm_port_outdo.hpp"

#include "core/applesoft_variables.hpp"
#include "core/io_ports.hpp"

#include <cstdint>

namespace applesoft::asm_port {

void MON_TABV(std::uint8_t row_zero_based);

namespace {

using MonitorOutputRoutine = void (*)(std::uint8_t);

constexpr std::uint16_t kMonitorCout1Vector = 0xfd62u;

void MON_VIDOUT(std::uint8_t a);

std::uint8_t readZeroPageByte(std::uint8_t address) {
    return variables_const().readByte(address);
}

void writeZeroPageByte(std::uint8_t address, std::uint8_t value) {
    variables().writeByte(address, value);
}

std::uint16_t readZeroPageWord(std::uint8_t address) {
    return ApplesoftVariables::makeWord(readZeroPageByte(address),
                                        readZeroPageByte(static_cast<std::uint8_t>(address + 1u)));
}

std::uint16_t computeTextRowBase(std::uint8_t row_zero_based) {
    const bool carryFromLsr = (row_zero_based & 0x01u) != 0u;
    const std::uint8_t bash = static_cast<std::uint8_t>(((row_zero_based >> 1u) & 0x03u) | 0x04u);

    std::uint8_t basl = static_cast<std::uint8_t>(row_zero_based & 0x18u);
    if (carryFromLsr) {
        basl = static_cast<std::uint8_t>(basl + 0x80u);
    }

    const std::uint8_t baslBase = basl;
    basl = static_cast<std::uint8_t>((basl << 2u) | baslBase);
    basl = static_cast<std::uint8_t>(basl + readZeroPageByte(ApplesoftVariables::ZP_MON_WNDLFT));

    return ApplesoftVariables::makeWord(basl, bash);
}

void setCursorRow(std::uint8_t row_zero_based) {
    MON_TABV(row_zero_based);
}

void scrollWindowUp() {
    constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);

    const std::uint8_t top = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDTOP);
    const std::uint8_t bottom = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDBTM);
    const std::uint8_t width = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDWDTH);

    for (std::uint8_t row = top; static_cast<std::uint8_t>(row + 1u) < bottom; ++row) {
        const auto dstBase = computeTextRowBase(row);
        const auto srcBase = computeTextRowBase(static_cast<std::uint8_t>(row + 1u));
        for (std::uint8_t col = 0u; col < width; ++col) {
            variables().writeByte(static_cast<std::uint16_t>(dstBase + col),
                                  variables_const().readByte(static_cast<std::uint16_t>(srcBase + col)));
        }
    }

    const auto lastRowBase = computeTextRowBase(static_cast<std::uint8_t>(bottom - 1u));
    for (std::uint8_t col = 0u; col < width; ++col) {
        variables().writeByte(static_cast<std::uint16_t>(lastRowBase + col), kBlank);
    }
}

void advanceCursorToNextLine(bool resetColumn) {
    const std::uint8_t top = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDTOP);
    const std::uint8_t bottom = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDBTM);
    std::uint8_t row = readZeroPageByte(ApplesoftVariables::ZP_MON_CV);

    if (resetColumn) {
        writeZeroPageByte(ApplesoftVariables::ZP_MON_CH, 0u);
    }

    if (bottom <= top) {
        setCursorRow(top);
        return;
    }

    if (static_cast<std::uint8_t>(row + 1u) >= bottom) {
        scrollWindowUp();
        row = static_cast<std::uint8_t>(bottom - 1u);
    } else {
        row = static_cast<std::uint8_t>(row + 1u);
    }

    setCursorRow(row);
}

void consumeKeyboardLatch(std::uint8_t keycode) {
    (void)ioPorts_const().readByte(IOPorts::ADDR_KEYBOARD_STROBE);
    ioPorts().writeByte(IOPorts::ADDR_KEYBOARD, static_cast<std::uint8_t>(keycode & 0x7fu));
}

void MON_LFB78(std::uint8_t a) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/math.o65.lst
    // Labels: LFB78 (inclusive) .. LFB94 (exclusive)
    // Name normalization: LFB78 -> MON_LFB78 (monitor label gets MON_ prefix).
    //
    // On carriage return, Ctrl-S pauses monitor output until another key is
    // pressed. A resumed Ctrl-C is left latched so the interpreter can still
    // observe it; all other resume keys are consumed before falling through to
    // VIDOUT.

    constexpr std::uint8_t kCarriageReturn = 0x8du;
    constexpr std::uint8_t kCtrlS = 0x93u;
    constexpr std::uint8_t kCtrlC = 0x83u;

    if (a == kCarriageReturn) {
        std::uint8_t keycode = ioPorts_const().readByte(IOPorts::ADDR_KEYBOARD);
        if ((keycode & 0x80u) != 0u && keycode == kCtrlS) {
            consumeKeyboardLatch(keycode);

            do {
                keycode = ioPorts_const().readByte(IOPorts::ADDR_KEYBOARD);
            } while ((keycode & 0x80u) == 0u);

            if (keycode != kCtrlC) {
                // LFB88 consumes the resume key unless it is Ctrl-C.
                consumeKeyboardLatch(keycode);
            }
        }
    }

    // LFB94 tail-jumps to VIDOUT.
    MON_VIDOUT(a);
}

// MON_VIDOUT -- the ROM's VIDOUT ($fb3c in display2.o65): the character
// renderer that updates monitor screen state. Called from MON_COUT1 via LFB78.
void MON_VIDOUT(std::uint8_t a) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // Labels: VIDOUT (inclusive) .. ESC1 (exclusive)
    // Name normalization: VIDOUT -> MON_VIDOUT (monitor label gets MON_ prefix).
    //
    // VIDOUT  cmp #$a0 / bcs STOADV -- printable high-bit chars fall through to output.
    // Control chars with bit 7 set ($80-$9f): dispatch on code.

    const std::uint8_t ch = static_cast<std::uint8_t>(a & 0x7fu);

    switch (ch) {
    case 0x07u: // BEL ($87) -- speaker toggle not modeled yet.
        break;
    case 0x08u: { // BS ($88)
        const std::uint8_t column = readZeroPageByte(ApplesoftVariables::ZP_MON_CH);
        if (column != 0u) {
            writeZeroPageByte(ApplesoftVariables::ZP_MON_CH, static_cast<std::uint8_t>(column - 1u));
        }
        break;
    }
    case 0x0au: // LF ($8a)
        advanceCursorToNextLine(false);
        break;
    case 0x0du: // CR ($8d)
        advanceCursorToNextLine(true);
        break;
    default:
        if (a >= 0xa0u) {
            const std::uint8_t column = readZeroPageByte(ApplesoftVariables::ZP_MON_CH);
            const std::uint8_t width = readZeroPageByte(ApplesoftVariables::ZP_MON_WNDWDTH);
            const std::uint16_t base = readZeroPageWord(ApplesoftVariables::ZP_MON_BASL);

            variables().writeByte(static_cast<std::uint16_t>(base + column), a);

            const std::uint8_t nextColumn = static_cast<std::uint8_t>(column + 1u);
            if (nextColumn >= width) {
                advanceCursorToNextLine(true);
            } else {
                writeZeroPageByte(ApplesoftVariables::ZP_MON_CH, nextColumn);
            }
        }
        break;
    }
}

void MON_COUT1(std::uint8_t a) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: COUT1 (inclusive) .. LFB78 call site tail (exclusive)
    // Name normalization: COUT1 -> MON_COUT1 (monitor label gets MON_ prefix).

    if (a >= 0xa0u) {
        a &= readZeroPageByte(ApplesoftVariables::ZP_MON_INVFLG); // and $32
    }

    MON_LFB78(a);
}

MonitorOutputRoutine resolveMonitorOutputVector(std::uint16_t vector) {
    switch (vector) {
    case kMonitorCout1Vector:
        return &MON_COUT1;
    default:
        // Slot ROM output vectors are not ported yet; keep monitor default semantics.
        return &MON_COUT1;
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
    //                                 ; default target: COUT1 at $fd62

    const auto routine = resolveMonitorOutputVector(readZeroPageWord(ApplesoftVariables::ZP_MON_CSW));
    routine(a);
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
