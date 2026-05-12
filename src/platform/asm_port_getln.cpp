#include "platform/asm_port_getln.hpp"

#include <cstdint>

namespace applesoft::asm_port {
namespace {

// TODO(asm-port): map zero-page prompt character at $33.
std::uint8_t read_prompt_char() { return 0; }

// TODO(asm-port): map indexed INPUT_BUFFER write at $0200,X.
void write_input_buffer(std::uint8_t /*index*/, std::uint8_t /*value*/) {}

// TODO(asm-port): map screen fetch through ($28),Y used by Ctrl-U handling.
std::uint8_t read_screen_char_via_28_y() { return 0; }

// TODO(asm-port): port COUT monitor output routine.
void COUT(std::uint8_t /*value*/) {}

// TODO(asm-port): port RDCHAR monitor key-read routine.
std::uint8_t RDCHAR() { return 0; }

// TODO(asm-port): port CLREOL monitor clear-to-end-of-line routine.
void CLREOL() {}

// TODO(asm-port): port GETLNZ label.
void GETLNZ() {}

// TODO(asm-port): port NOTCR label handling (echo/edit/margin logic).
void NOTCR(std::uint8_t& /*x*/) {}

// TODO(asm-port): port CROUT label.
void CROUT() {}

} // namespace

std::uint8_t MON_GETLN() {
    // TODO(asm-port): port GETLN monitor line-input routine.
return 0;
}

} // namespace applesoft::asm_port