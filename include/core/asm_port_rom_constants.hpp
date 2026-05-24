#pragma once

#include "core/applesoft_dual_pointer.hpp"

#include <cstdint>

namespace applesoft::asm_port {

ApplesoftDualPointer<const std::uint8_t> MON_TITLE();

ApplesoftDualPointer<const std::uint8_t> AS_CON_ONE();
ApplesoftDualPointer<const std::uint8_t> AS_POLY_LOG();
ApplesoftDualPointer<const std::uint8_t> AS_CON_SQR_HALF();
ApplesoftDualPointer<const std::uint8_t> AS_CON_SQR_TWO();
ApplesoftDualPointer<const std::uint8_t> AS_CON_NEG_HALF();
ApplesoftDualPointer<const std::uint8_t> AS_CON_LOG_TWO();
ApplesoftDualPointer<const std::uint8_t> AS_CON_99999999P9();
ApplesoftDualPointer<const std::uint8_t> AS_CON_999999999();
ApplesoftDualPointer<const std::uint8_t> AS_CON_BILLION();
ApplesoftDualPointer<const std::uint8_t> AS_CON_HALF();
ApplesoftDualPointer<const std::uint8_t> AS_CON_LOG_E();
ApplesoftDualPointer<const std::uint8_t> AS_POLY_EXP();
ApplesoftDualPointer<const std::uint8_t> AS_CON_RND_1();
ApplesoftDualPointer<const std::uint8_t> AS_CON_RND_2();
ApplesoftDualPointer<const std::uint8_t> AS_CON_PI_HALF();
ApplesoftDualPointer<const std::uint8_t> AS_CON_PI_DOUB();
ApplesoftDualPointer<const std::uint8_t> AS_QUARTER();
ApplesoftDualPointer<const std::uint8_t> AS_POLY_SIN();
ApplesoftDualPointer<const std::uint8_t> AS_POLY_ATN();

} // namespace applesoft::asm_port