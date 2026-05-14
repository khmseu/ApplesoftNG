#include "platform/asm_port_getln.hpp"
#include "platform/asm_port_outdo.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/applesoft_variables.hpp"

#include <cstdint>

namespace applesoft::asm_port {
void MON_COUT(std::uint8_t a);
void MON_BELL();
namespace {

std::uint8_t read_prompt_char() {
    return variables_const().readByte(ApplesoftVariables::ZP_MON_PROMPT);
}

void write_input_buffer(std::uint8_t index, std::uint8_t value) {
    variables().writeByte(static_cast<std::uint16_t>(ApplesoftVariables::ADDR_INPUT_BUFFER + index), value);
}

std::uint8_t read_screen_char_via_28_y() {
    const std::uint8_t y = variables_const().readByte(ApplesoftVariables::ZP_MON_CH);
    const std::uint16_t base = variables_const().readWord(ApplesoftVariables::ZP_MON_BASL);
    return variables_const().readByte(static_cast<std::uint16_t>(base + y));
}

void CROUT();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
// Labels: COUT (inclusive) .. COUT1 (exclusive)
// Name normalization: none (assembler label COUT kept verbatim).
void COUT(std::uint8_t value) {
    MON_COUT(value);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/keyin.o65.lst
// Labels: RDCHAR (inclusive) .. NOTCR (exclusive)
// Name normalization: none (assembler label RDCHAR kept verbatim).
std::uint8_t RDCHAR() {
    // ESC ($9B) branches back through ESC to read another key; model as a loop.
    constexpr std::uint8_t kEsc = 0x9bu;
    while (true) {
        const std::uint8_t key = MON_RDKEY();
        if (key != kEsc) {
            return key;
        }
    }
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
// Labels: CLREOL (inclusive) .. WAIT (exclusive)
// Name normalization: none (assembler label CLREOL kept verbatim).
void CLREOL() {
    constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);
    const std::uint8_t columnStart = variables_const().readByte(ApplesoftVariables::ZP_MON_CH);
    const std::uint8_t width = variables_const().readByte(ApplesoftVariables::ZP_MON_WNDWDTH);
    const std::uint16_t base = variables_const().readWord(ApplesoftVariables::ZP_MON_BASL);

    for (std::uint8_t column = columnStart; column < width; ++column) {
        variables().writeByte(static_cast<std::uint16_t>(base + column), kBlank);
    }
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/keyin.o65.lst
// Labels: GETLNZ (inclusive) .. BCKSPC (exclusive)
// Name normalization: none (assembler label GETLNZ kept verbatim).
std::uint8_t GETLNZ() {
    CROUT();
    COUT(read_prompt_char());
    return 1u;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/keyin.o65.lst
// Labels: NOTCR (inclusive) .. GETLNZ (exclusive)
// Name normalization: none (assembler label NOTCR kept verbatim).
void NOTCR(std::uint8_t& x) {
    constexpr std::uint8_t kBackspace = 0x88u;
    constexpr std::uint8_t kCtrlX = 0x98u;
    constexpr std::uint8_t kMargin = 0xf8u;
    constexpr std::uint8_t kCancelSlash = static_cast<std::uint8_t>('\\' | 0x80u);

    const std::uint8_t savedInv = variables_const().readByte(ApplesoftVariables::ZP_MON_INVFLG);
    variables().writeByte(ApplesoftVariables::ZP_MON_INVFLG, 0xffu);
    COUT(variables_const().readByte(static_cast<std::uint16_t>(ApplesoftVariables::ADDR_INPUT_BUFFER + x)));
    variables().writeByte(ApplesoftVariables::ZP_MON_INVFLG, savedInv);

    const std::uint8_t current = variables_const().readByte(
        static_cast<std::uint16_t>(ApplesoftVariables::ADDR_INPUT_BUFFER + x));

    if (current == kBackspace || current == kCtrlX) {
        if (x == 0u) {
            x = GETLNZ();
            return;
        }
        --x;
        return;
    }

    if (x >= kMargin) {
        MON_BELL();
    }

    ++x;
    if (x == 0u) {
        COUT(kCancelSlash);
        x = GETLNZ();
    }
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
// Labels: CROUT (inclusive) .. PRA1 (exclusive)
// Name normalization: none (assembler label CROUT kept verbatim).
void CROUT() {
    COUT(0x8du);
}

} // namespace

std::uint8_t MON_GETLN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/keyin.o65.lst
    // Labels: GETLN (inclusive) .. CROUT (exclusive)
    // Name normalization: GETLN -> MON_GETLN (monitor label gets MON_ prefix).
    constexpr std::uint8_t kCtrlU = 0x95u;
    constexpr std::uint8_t kCarriageReturn = 0x8du;
    constexpr std::uint8_t kLowercaseThreshold = 0xe0u;

    std::uint8_t x = GETLNZ();

    while (true) {
        std::uint8_t ch = RDCHAR();
        if (ch == kCtrlU) {
            ch = read_screen_char_via_28_y();
        }

        if (ch >= kLowercaseThreshold) {
            ch = static_cast<std::uint8_t>(ch & 0xdfu);
        }

        write_input_buffer(x, ch);
        if (ch == kCarriageReturn) {
            CLREOL();
            return x;
        }

        NOTCR(x);
    }
}

} // namespace applesoft::asm_port
