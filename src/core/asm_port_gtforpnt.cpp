#include "core/asm_port_gtforpnt.hpp"

namespace applesoft::asm_port {
namespace {

constexpr std::uint8_t kFrameSize = 18;

std::uint8_t stack_at(const GTFORPNTState& state, std::uint8_t x, std::uint8_t plus) {
    return state.stackPage[static_cast<std::uint8_t>(x + plus)];
}

} // namespace

GTFORPNTResult GTFORPNT(std::uint8_t stackPointer, GTFORPNTState& state) {
    // TSX + 4: skip return address and caller context to first candidate frame.
    std::uint8_t x = static_cast<std::uint8_t>(stackPointer + 4);

    while (x != 0) {
        // FRAME MARKER at STACK+1,X must be TOKEN_FOR ($81).
        if (stack_at(state, x, 1) == TOKEN_FOR) {
            // NEXT with no variable: bind FORPNT from current frame first.
            if (state.forpntHi == 0) {
                state.forpntLo = stack_at(state, x, 2);
                state.forpntHi = stack_at(state, x, 3);
            }

            // Compare FORPNT against frame variable pointer (hi then lo, as ROM).
            if (state.forpntHi == stack_at(state, x, 3) &&
                state.forpntLo == stack_at(state, x, 2)) {
                return GTFORPNTResult{true, x};
            }
        }

        // Advance to next frame candidate, wrapping in 8-bit space exactly as 6502.
        x = static_cast<std::uint8_t>(x + kFrameSize);
    }

    return GTFORPNTResult{false, x};
}

} // namespace applesoft::asm_port
