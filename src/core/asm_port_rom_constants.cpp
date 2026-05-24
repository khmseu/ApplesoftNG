// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CON_ONE (inclusive) .. AS_LOG (exclusive)
// Name normalization: none

#include "core/asm_port_rom_constants.hpp"

namespace {

constexpr std::uint8_t kAS_CON_ONE[] = {0x81u, 0x00u, 0x00u, 0x00u, 0x00u};
constexpr std::uint8_t kAS_POLY_LOG[] = {
    0x03u, 0x7fu, 0x5eu, 0x56u, 0xcbu, 0x79u, 0x80u, 0x13u, 0x9bu, 0x0bu, 0x64u,
    0x80u, 0x76u, 0x38u, 0x93u, 0x16u, 0x82u, 0x38u, 0xaau, 0x3bu, 0x20u};
constexpr std::uint8_t kAS_CON_SQR_HALF[] = {0x80u, 0x35u, 0x04u, 0xf3u, 0x34u};
constexpr std::uint8_t kAS_CON_SQR_TWO[] = {0x81u, 0x35u, 0x04u, 0xf3u, 0x34u};
constexpr std::uint8_t kAS_CON_NEG_HALF[] = {0x80u, 0x80u, 0x00u, 0x00u, 0x00u};
constexpr std::uint8_t kAS_CON_LOG_TWO[] = {0x80u, 0x31u, 0x72u, 0x17u, 0xf8u};
constexpr std::uint8_t kAS_CON_99999999P9[] = {0x9bu, 0x3eu, 0xbcu, 0x1fu,
                                               0xfdu};
constexpr std::uint8_t kAS_CON_999999999[] = {0x9eu, 0x6eu, 0x6bu, 0x27u,
                                              0xfdu};
constexpr std::uint8_t kAS_CON_BILLION[] = {0x9eu, 0x6eu, 0x6bu, 0x28u, 0x00u};
constexpr std::uint8_t kAS_CON_HALF[] = {0x80u, 0x00u, 0x00u, 0x00u, 0x00u};
constexpr std::uint8_t kAS_CON_LOG_E[] = {0x81u, 0x38u, 0xaau, 0x3bu, 0x29u};
constexpr std::uint8_t kAS_POLY_EXP[] = {
    0x07u, 0x71u, 0x34u, 0x58u, 0x3eu, 0x56u, 0x74u, 0x16u, 0x7eu, 0xb3u, 0x1bu,
    0x77u, 0x2fu, 0xeeu, 0xe3u, 0x85u, 0x7au, 0x1du, 0x84u, 0x1cu, 0x2au, 0x7cu,
    0x63u, 0x59u, 0x58u, 0x0au, 0x7eu, 0x75u, 0xfdu, 0xe7u, 0xc6u, 0x80u, 0x31u,
    0x72u, 0x18u, 0x10u, 0x81u, 0x00u, 0x00u, 0x00u, 0x00u};
constexpr std::uint8_t kAS_CON_RND_1[] = {0x98u, 0x35u, 0x44u, 0x7au, 0x68u};
constexpr std::uint8_t kAS_CON_RND_2[] = {0x68u, 0x28u, 0xb1u, 0x46u, 0xd0u};
constexpr std::uint8_t kAS_CON_PI_HALF[] = {0x81u, 0x49u, 0x0fu, 0xdau, 0xa2u};
constexpr std::uint8_t kAS_CON_PI_DOUB[] = {0x83u, 0x49u, 0x0fu, 0xdau, 0xa2u};
constexpr std::uint8_t kAS_QUARTER[] = {0x7fu, 0x00u, 0x00u, 0x00u, 0x00u};
constexpr std::uint8_t kAS_POLY_SIN[] = {
    0x05u, 0x84u, 0xe6u, 0x1au, 0x2du, 0x1bu, 0x86u, 0x28u, 0x07u, 0xfbu, 0xf8u,
    0x87u, 0x99u, 0x68u, 0x89u, 0x01u, 0x87u, 0x23u, 0x35u, 0xdfu, 0xe1u, 0x86u,
    0xa5u, 0x5du, 0xe7u, 0x28u, 0x83u, 0x49u, 0x0fu, 0xdau, 0xa2u};
constexpr std::uint8_t kAS_POLY_ATN[] = {
    0x0bu, 0x76u, 0xb3u, 0x83u, 0xbdu, 0xd3u, 0x79u, 0x1eu, 0xf4u, 0xa6u,
    0xf5u, 0x7bu, 0x83u, 0xfcu, 0xb0u, 0x10u, 0x7cu, 0x0cu, 0x1fu, 0x67u,
    0xcau, 0x7cu, 0xdeu, 0x53u, 0xcbu, 0xc1u, 0x7du, 0x14u, 0x64u, 0x70u,
    0x4cu, 0x7du, 0xb7u, 0xeau, 0x51u, 0x7au, 0x7du, 0x63u, 0x30u, 0x88u,
    0x7eu, 0x92u, 0x44u, 0x99u, 0x3au, 0x7eu, 0x4cu, 0xccu, 0x91u, 0xc7u,
    0x7fu, 0xaau, 0xaau, 0xaau, 0x13u, 0x81u, 0x00u, 0x00u, 0x00u, 0x00u};

} // namespace

namespace applesoft::asm_port {

ApplesoftDualPointer<const std::uint8_t> AS_CON_ONE() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_ONE);
}

ApplesoftDualPointer<const std::uint8_t> AS_POLY_LOG() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_POLY_LOG);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_SQR_HALF() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_SQR_HALF);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_SQR_TWO() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_SQR_TWO);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_NEG_HALF() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_NEG_HALF);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_LOG_TWO() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_LOG_TWO);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_99999999P9() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_99999999P9);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_999999999() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_999999999);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_BILLION() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_BILLION);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_HALF() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_HALF);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_LOG_E() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_LOG_E);
}

ApplesoftDualPointer<const std::uint8_t> AS_POLY_EXP() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_POLY_EXP);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_RND_1() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_RND_1);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_RND_2() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_RND_2);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_PI_HALF() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_PI_HALF);
}

ApplesoftDualPointer<const std::uint8_t> AS_CON_PI_DOUB() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_CON_PI_DOUB);
}

ApplesoftDualPointer<const std::uint8_t> AS_QUARTER() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_QUARTER);
}

ApplesoftDualPointer<const std::uint8_t> AS_POLY_SIN() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_POLY_SIN);
}

ApplesoftDualPointer<const std::uint8_t> AS_POLY_ATN() {
  return ApplesoftDualPointer<const std::uint8_t>::native(kAS_POLY_ATN);
}

} // namespace applesoft::asm_port