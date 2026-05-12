#pragma once

#include <cstdint>

namespace applesoft::asm_port {

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRLT2 (inclusive) .. PUTNEW (exclusive)
// Name normalization: none (assembler label STRLT2 kept verbatim).
void STRLT2(std::uint16_t address);

// Labels: GARBAG (inclusive) .. FIND_HIGHEST_STRING (exclusive)
void GARBAG();

// Labels: STRINI (inclusive) .. STRSPA (exclusive)
void STRINI(std::uint8_t length);

// Labels: MOVINS (inclusive) .. MOVSTR (exclusive)
void MOVINS();

// Labels: FRETMP (inclusive) .. FRETMS (exclusive)
std::uint8_t FRETMP(std::uint16_t descriptorAddress);

// Labels: FRETMS (inclusive) .. CHRSTR (exclusive)
bool FRETMS(std::uint16_t descriptorAddress);

// Labels: CHRSTR (inclusive) .. LEFTSTR (exclusive)
void CHRSTR();

// Labels: LEFTSTR (inclusive) .. RIGHTSTR (exclusive)
void LEFTSTR();

// Labels: RIGHTSTR (inclusive) .. MIDSTR (exclusive)
void RIGHTSTR();

// Labels: MIDSTR (inclusive) .. SUBSTRING_SETUP (exclusive)
void MIDSTR();

// Labels: LEN (inclusive) .. GETSTR (exclusive)
void LEN();

// Labels: GETSTR (inclusive) .. ASC (exclusive)
std::uint8_t GETSTR();

// Labels: ASC (inclusive) .. GOIQ (exclusive)
void ASC();

} // namespace applesoft::asm_port
