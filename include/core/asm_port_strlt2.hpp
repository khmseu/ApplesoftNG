#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_STRLT2 (inclusive) .. AS_PUTNEW (exclusive)
// Name normalization: none (assembler label AS_STRLT2 kept verbatim).
void AS_STRLT2(std::uint16_t address);

// AS_Labels: AS_GARBAG (inclusive) .. AS_FIND_HIGHEST_STRING (exclusive)
void AS_GARBAG();

// AS_Labels: AS_STRINI (inclusive) .. AS_STRSPA (exclusive)
void AS_STRINI(std::uint8_t length);

// AS_Labels: AS_STRSPA (inclusive) .. AS_L_STRSPA_1 (exclusive)
void AS_STRSPA(std::uint8_t length);

// AS_Labels: AS_MOVINS (inclusive) .. AS_MOVSTR (exclusive)
void AS_MOVINS();

// AS_Labels: AS_FRETMP (inclusive) .. AS_FRETMS (exclusive)
std::uint8_t AS_FRETMP(std::uint16_t descriptorAddress);

// AS_Labels: AS_FRETMS (inclusive) .. AS_CHRSTR (exclusive)
bool AS_FRETMS(std::uint16_t descriptorAddress);

// AS_Labels: AS_FREFAC (inclusive) .. AS_FRETMP (exclusive)
std::uint8_t AS_FREFAC();

// AS_Labels: AS_CHRSTR (inclusive) .. AS_LEFTSTR (exclusive)
void AS_CHRSTR();

// AS_Labels: AS_LEFTSTR (inclusive) .. AS_RIGHTSTR (exclusive)
void AS_LEFTSTR();

// AS_Labels: AS_RIGHTSTR (inclusive) .. AS_MIDSTR (exclusive)
void AS_RIGHTSTR();

// AS_Labels: AS_MIDSTR (inclusive) .. AS_SUBSTRING_SETUP (exclusive)
void AS_MIDSTR();

// AS_Labels: AS_LEN (inclusive) .. AS_GETSTR (exclusive)
void AS_LEN();

// AS_Labels: AS_GETSTR (inclusive) .. AS_ASC (exclusive)
std::uint8_t AS_GETSTR();

// AS_Labels: AS_ASC (inclusive) .. AS_GOIQ (exclusive)
void AS_ASC();

} // namespace applesoft::asm_port
