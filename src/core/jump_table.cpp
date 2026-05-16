#include "core/jump_table.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/asm_port_stack.hpp"
#include "core/io_ports.hpp"
#include "platform/asm_port_outdo.hpp"

namespace applesoft::asm_port {
void AS_COLD_START();
std::uint8_t MON_GETLN();
} // namespace applesoft::asm_port

namespace ApplesoftNG {

void ExternalJumpDispatcher::Jump(std::uint16_t address) {
  using namespace applesoft::asm_port;

  // Handle KSW/CSW vectors or explicit JMP targets
  switch (address) {
  case 0xE000u:
    AS_COLD_START();
    break;

  case ADDR_MON_KEYIN:
    MON_KEYIN();
    break;

  case ADDR_MON_GETLN:
    MON_GETLN();
    break;

  case ADDR_MON_COUT1:
    MON_COUT1(variables().readByte(ApplesoftVariables::ZP_MON_DEBUG_REG_A));
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
