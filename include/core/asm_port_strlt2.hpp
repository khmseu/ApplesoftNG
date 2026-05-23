#pragma once

#include <cstdint>

namespace applesoft::asm_port {
void AS_STRLT2(std::uint16_t address);

void AS_GARBAG();

void AS_STRINI(std::uint8_t length);

void AS_STRSPA(std::uint8_t length);

void AS_MOVINS();

std::uint8_t AS_FRETMP(std::uint16_t descriptorAddress);

bool AS_FRETMS(std::uint16_t descriptorAddress);

std::uint8_t AS_FREFAC();

void AS_CHRSTR();

void AS_LEFTSTR();

void AS_RIGHTSTR();

void AS_MIDSTR();

void AS_LEN();

std::uint8_t AS_GETSTR();

void AS_ASC();

void AS_CAT();

} // namespace applesoft::asm_port
