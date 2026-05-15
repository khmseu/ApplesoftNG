#include "core/asm_port_jump_table.hpp"
#include "core/applesoft_variables.hpp"
#include "core/io_ports.hpp"
#include "core/asm_port_stack.hpp"

namespace applesoft::asm_port {
    void AS_COLD_START();
    void MON_PRBYTE(std::uint8_t value);
    void MON_COUT(std::uint8_t a);
    std::uint8_t MON_GETLN();
}

namespace ApplesoftNG {

void ExternalJumpDispatcher::Jump(std::uint16_t address) {
    using namespace applesoft::asm_port;

    // Handle KSW/CSW vectors or explicit JMP targets
    switch (address) {
        case 0xE000u:
            AS_COLD_START();
            break;
        
        case ADDR_MON_KEYIN:
            // KEYIN is usually called through the vector at $38/$39 (KSW)
            // or directly as a fallback if the vector is uninitialized.
            // For now, we don't have a standalone MON_KEYIN, but we can
            // simulate its side effect if needed or call a stub.
            break;

        case ADDR_MON_GETLN:
            MON_GETLN();
            break;

        case ADDR_MON_COUT:
            // Characters are passed in A register (6502).
            // In C++, the caller should have extracted it from the processor state emulation
            // if we were a full CPU emulator, but here we handle it via the switch.
            // If we are calling it from a simulated `jmp (vector)`, we might need
            // to know the current value of 'A'. 
            // Since this is a specialized dispatcher, we might need a version that takes 'A'.
            break;

        case ADDR_MON_PRBYTE:
            // Similar to COUT, prints A.
            break;

        case ADDR_MON_IOREST:
            // Restore registers. Stub.
            break;

        default:
            // RESET slot scan (Cx00) fallback
            if ((address & 0xFF00u) >= 0xC100u && (address & 0xFF00u) <= 0xC700u) {
                AS_COLD_START();
            }
            break;
    }
}

} // namespace ApplesoftNG
