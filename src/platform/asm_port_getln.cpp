#include "platform/asm_port_getln.hpp"
#include "platform/asm_port_outdo.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/applesoft_variables.hpp"

#include <cstdint>

namespace applesoft::asm_port {
void MON_COUT(std::uint8_t a);
namespace {

// TODO(asm-port): map zero-page prompt character at $33.
std::uint8_t read_prompt_char() { return 0; }

// TODO(asm-port): map indexed INPUT_BUFFER write at $0200,X.
void write_input_buffer(std::uint8_t /*index*/, std::uint8_t /*value*/) {}

// TODO(asm-port): map screen fetch through ($28),Y used by Ctrl-U handling.
std::uint8_t read_screen_char_via_28_y() { return 0; }

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
    const std::uint8_t yStart = variables_const().readByte(ApplesoftVariables::ZP_MON_CH);
    const std::uint8_t width = variables_const().readByte(ApplesoftVariables::ZP_MON_WNDWDTH);
    const std::uint16_t base = variables_const().readWord(ApplesoftVariables::ZP_MON_BASL);

    for (std::uint8_t y = yStart; y < width; ++y) {
        variables().writeByte(static_cast<std::uint16_t>(base + y), kBlank);
    }
}

// TODO(asm-port): port GETLNZ label.
void GETLNZ() {}

// TODO(asm-port): port NOTCR label handling (echo/edit/margin logic).
void NOTCR(std::uint8_t& /*x*/) {}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
// Labels: CROUT (inclusive) .. PRA1 (exclusive)
// Name normalization: none (assembler label CROUT kept verbatim).
void CROUT() {
    COUT(0x8du);
}

} // namespace

std::uint8_t MON_GETLN() {
    // TODO(asm-port): port GETLN monitor line-input routine.
return 0;
}

} // namespace applesoft::asm_port
