#include "core/asm_port_input.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_strtxt.hpp"

namespace applesoft::asm_port {

void CONTROL_C_TYPED();

// TODO(asm-port): port CHRGOT label.
std::uint8_t CHRGOT() { return 0; }

namespace {

// --- Dummy callees for incremental porting ---

// TODO(asm-port): port SYNCHR label.
void SYNCHR(std::uint8_t /*expected*/) {}

// TODO(asm-port): port STRPRT label.
void STRPRT() {}

// TODO(asm-port): port OUTQUES label.
void OUTQUES() {}

// TODO(asm-port): port ERRDIR label.
void ERRDIR() {}

// TODO(asm-port): port INLIN label.
void INLIN() {}

void write_INPUT_BUFFER_minus_1(std::uint8_t v) {
    variables().writeByte(0x01ffu, v);
}

std::uint8_t read_INPUT_BUFFER_0() {
    return variables_const().readByte(0x0200u);
}

} // namespace

InputDispatch INPUT() {
    // "INPUT" statement
    // Check for optional prompt string.
    if (CHRGOT() == static_cast<std::uint8_t>('"')) {
        // INPUT "prompt"; ...
        STRTXT();
        SYNCHR(static_cast<std::uint8_t>(';'));
        STRPRT();
    } else {
        // No string prompt => print "?"
        OUTQUES();
    }

    // Illegal in direct mode.
    ERRDIR();

    // Prime input buffer with comma at INPUT_BUFFER-1.
    write_INPUT_BUFFER_minus_1(static_cast<std::uint8_t>(','));

    // Read a line into input buffer.
    INLIN();

    // If first char is CTRL-C ($03), dispatch control-break path.
    if (read_INPUT_BUFFER_0() == static_cast<std::uint8_t>(0x03)) {
        CONTROL_C_TYPED();
        return InputDispatch::ControlCTyped;
    }

    // Fall-through target in original is INPUT_FLAG_ZERO (outside this slice).
    return InputDispatch::ContinueAt_INPUT_FLAG_ZERO;
}

} // namespace applesoft::asm_port