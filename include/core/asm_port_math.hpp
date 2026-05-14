#pragma once

#include <cstdint>

/**
 * Math routines ported from Applesoft BASIC 6502 assembly.
 */

namespace applesoft::asm_port {

void MUL10();
void DIV10();
void NEGATE_FAC();
void ROUND_FAC();
void COPY_FAC_TO_ARG_ROUNDED();
void ADD_EXPONENTS();
void FLOAT(std::int8_t value);
void FLOAT();
void FLOAT_1(std::uint8_t exponent);
void FLOAT_2(std::uint8_t exponent, bool positive);
void FADDT();
void FADD();
void FSUB();
void FIN();
void ADDACC();
void ADDACC_WITH_DIGIT(std::uint8_t digit);
void LOAD_ARG_FROM_YA(std::uint16_t address);
void LOAD_ARG_FROM_YA();

} // namespace applesoft::asm_port
