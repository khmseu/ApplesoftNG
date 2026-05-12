#include "core/io_ports.hpp"

namespace applesoft::asm_port {

namespace {

IOPorts g_ioPorts;

} // namespace

IOPorts& ioPorts() {
    return g_ioPorts;
}

const IOPorts& ioPorts_const() {
    return g_ioPorts;
}

std::uint8_t IOPorts::readByte(std::uint16_t address) const {
    if (address < ADDR_BASE || address >= ADDR_END) {
        return 0u;
    }

    return ports_[address - ADDR_BASE];
}

void IOPorts::writeByte(std::uint16_t address, std::uint8_t value) {
    if (address < ADDR_BASE || address >= ADDR_END) {
        return;
    }

    ports_[address - ADDR_BASE] = value;
}

} // namespace applesoft::asm_port
