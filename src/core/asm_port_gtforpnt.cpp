#include "core/asm_port_gtforpnt.hpp"
#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {
namespace {

constexpr std::uint8_t kFrameSize = 18;

std::uint8_t stack_at(const GTFORPNTState& state, std::uint8_t x, std::uint8_t plus) {
    return state.stackPage[static_cast<std::uint8_t>(x + plus)];
}

std::uint16_t stack_word_at(const GTFORPNTState& state, std::uint8_t x, std::uint8_t plus) {
    return ApplesoftVariables::makeWord(
        stack_at(state, x, plus),
        stack_at(state, x, static_cast<std::uint8_t>(plus + 1u)));
}

} // namespace

GTFORPNTResult GTFORPNT(std::uint8_t stackPointer, GTFORPNTState& state) {
    // TSX + 4: skip return address and caller context to first candidate frame.
    std::uint8_t x = static_cast<std::uint8_t>(stackPointer + 4);

    while (x != 0) {
        // FRAME MARKER at STACK+1,X must be TOKEN_FOR ($81).
        if (stack_at(state, x, 1) == TOKEN_FOR) {
            const std::uint16_t frameVariablePointer = stack_word_at(state, x, 2);

            // NEXT with no variable: bind FORPNT from current frame first.
            if (state.forpntHi == 0) {
                state.setVariablePointer(frameVariablePointer);
            }

            // Compare FORPNT against frame variable pointer (hi then lo, as ROM).
            if (state.variablePointer() == frameVariablePointer) {
                return GTFORPNTResult{true, x};
            }
        }

        // Advance to next frame candidate, wrapping in 8-bit space exactly as 6502.
        x = static_cast<std::uint8_t>(x + kFrameSize);
    }

    return GTFORPNTResult{false, x};
}

} // namespace applesoft::asm_port
