#pragma once

#include <cstdint>

/**
 * Math routines ported from Applesoft AS_BASIC 6502 assembly.
 */

namespace applesoft::asm_port {

void AS_MUL10();
void AS_DIV10();
void AS_NEGATE_FAC();
void AS_ROUND_FAC();
void AS_COPY_FAC_TO_ARG_ROUNDED();
void AS_ADD_EXPONENTS();
void AS_FLOAT(std::int8_t value);
void AS_FLOAT();
void AS_FLOAT_1(std::uint8_t exponent);
void AS_FLOAT_2(std::uint8_t exponent, bool positive);
void AS_FADDT();
void AS_FSUBT();
void AS_FADD();
void AS_FSUB();
void AS_FIN();
void AS_ADDACC();
void AS_ADDACC_WITH_DIGIT(std::uint8_t digit);
void AS_LOAD_ARG_FROM_YA(std::uint16_t address);
void AS_LOAD_ARG_FROM_YA();
void AS_NORMALIZE_FAC_2();
void AS_NORMALIZE_FAC_4(std::uint8_t shiftCount);
void AS_ZERO_FAC();

} // namespace applesoft::asm_port
