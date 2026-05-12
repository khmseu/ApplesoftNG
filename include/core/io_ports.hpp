#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

class IOPorts {
public:
    static constexpr std::uint16_t ADDR_BASE = 0xc000u;
    static constexpr std::uint16_t ADDR_END = 0xd000u;
    static constexpr std::uint16_t ADDR_KEYBOARD = 0xc000u;
    static constexpr std::uint16_t ADDR_SW_MIXCLR = 0xc052u;
    static constexpr std::uint16_t ADDR_SW_MIXSET = 0xc053u;
    static constexpr std::uint16_t ADDR_SW_LOWSCR = 0xc054u;
    static constexpr std::uint16_t ADDR_SW_HISCR = 0xc055u;
    static constexpr std::uint16_t ADDR_SW_LORES = 0xc056u;
    static constexpr std::uint16_t ADDR_SW_HIRES = 0xc057u;
    static constexpr std::uint16_t ADDR_SW_TXTCLR = 0xc050u;

    std::uint8_t readByte(std::uint16_t address) const;
    void writeByte(std::uint16_t address, std::uint8_t value);

private:
    std::array<std::uint8_t, ADDR_END - ADDR_BASE> ports_{};
};

IOPorts& ioPorts();
const IOPorts& ioPorts_const();

} // namespace applesoft::asm_port
