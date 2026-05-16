#include "core/asm_port_math.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_error_handling.hpp"
#include <cstdint>

namespace applesoft::asm_port {

extern std::uint8_t ReadZeroPageByte(std::uint8_t address);
extern void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
extern std::uint8_t ReadProgramByte(std::uint16_t address);
extern void AS_ERROR(std::uint8_t error_code);
extern void AS_LOAD_ARG_FROM_YA();
extern void AS_COPY_ARG_TO_FAC();
extern std::uint8_t gFloatInput;

// Forward declarations of subroutines used within AS_FADD/AS_FSUB
void AS_SHIFT_RIGHT();
void AS_SHIFT_RIGHT_4();
void AS_COMPLEMENT_FAC();
void AS_NORMALIZE_FAC_1();
void AS_NORMALIZE_FAC_2();
void AS_NORMALIZE_FAC_4(std::uint8_t shiftCount);
void AS_NORMALIZE_FAC_5();
void AS_ZERO_FAC();
void AS_ROUND_FAC();
void AS_COPY_FAC_TO_ARG_ROUNDED();
void AS_FLOAT();
void AS_FLOAT_1(std::uint8_t exponent);
void AS_FLOAT_2(std::uint8_t exponent, bool positive);
void MULTIPLY_FAC_BY_TEN();
void AS_DIVIDE_FAC_BY_TEN();
void AS_ADD_EXPONENTS();
void AS_ADD_EXPONENTS_1();
void AS_NEGATE_FAC();
void AS_INCREMENT_FAC_MANTISSA();
void AS_INCREMENT_MANTISSA();

void AS_LOAD_ARG_FROM_YA() {
  const std::uint16_t address = static_cast<std::uint16_t>(
      (static_cast<std::uint16_t>(variables_const().MON_DEBUG_REG_Y) << 8u) |
      variables_const().MON_DEBUG_REG_A);
  AS_LOAD_ARG_FROM_YA(address);
}

void AS_FLOAT() { AS_FLOAT(static_cast<std::int8_t>(gFloatInput)); }

/**
 * AS_FSUB: AS_FAC = (Y,A) - AS_FAC
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_FSUB (inclusive) .. AS_FSUBT (exclusive)
 */
void AS_FSUB() {
  AS_LOAD_ARG_FROM_YA();
  // Falls through to AS_FSUBT
}

/**
 * AS_FSUBT: AS_FAC = AS_ARG - AS_FAC
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_FSUBT (inclusive) .. AS_FADD_1 (exclusive)
 */
void AS_FSUBT() {
  std::uint8_t fac_sign = variables_const().AS_FAC_SIGN;
  fac_sign ^= 0xFF;
  variables().AS_FAC_SIGN = fac_sign;

  std::uint8_t arg_sign = variables_const().AS_ARG[5];
  variables().AS_SGNCPR = fac_sign ^ arg_sign;

  AS_FADDT();
}

/**
 * AS_FADD: AS_FAC = (Y,A) + AS_FAC
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_FADD (inclusive) .. AS_FADDT (exclusive)
 */
void AS_FADD() {
  AS_LOAD_ARG_FROM_YA();
  AS_FADDT();
}

/**
 * AS_FADDT: AS_FAC = AS_ARG + AS_FAC
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_FADDT (inclusive) .. AS_FADD_2 (exclusive)
 */
void AS_FADDT() {
  if (variables_const().AS_FAC[0] == 0) {
    AS_COPY_ARG_TO_FAC();
    return;
  }

  std::uint8_t fac_ext = variables_const().AS_FAC_EXTENSION;
  variables().AS_ARG_EXTENSION = fac_ext;

  // Continue porting here...
}

/**
 * AS_FADD_2: Entry point for adding A to AS_FAC (used by AS_MUL10)
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_FADD_2 (inclusive) .. AS_FADD_3 (exclusive)
 */
void AS_FADD_2(std::uint8_t exponent) {
  if (exponent == 0)
    return;

  std::uint8_t fac_exp = variables_const().AS_FAC[0];
  std::int16_t diff = (std::int16_t)exponent - (std::int16_t)fac_exp;

  if (diff == 0) {
    // AS_FADD_3 logic would follow
  } else if (diff < 0) {
    // AS_ARG has smaller exponent
    // AS_L_FADD_2_1 logic
  } else {
    // AS_FAC has smaller exponent
    variables().AS_FAC[0] = exponent;
    variables().AS_FAC_SIGN = variables_const().AS_ARG[5];
    // ... (complex alignment and subtraction logic)
  }
  // This is a partial stub to satisfy AS_MUL10 for now.
}

/**
 * AS_MUL10: Multiply AS_FAC by 10
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_MUL10 (inclusive) .. AS_L_MUL10_1 (exclusive)
 */
void AS_MUL10() {
  AS_COPY_FAC_TO_ARG_ROUNDED();
  std::uint8_t exp = variables_const().AS_FAC[0];
  if (exp == 0)
    return; // AS_FAC=0

  if (exp > 253) {  // exp + 2 > 255
    AS_ERROR(0x45); // AS_OVERFLOW (AS_ERR_OVERFLOW)
    return;
  }

  exp += 2; // AS_FAC * 4
  variables().AS_FAC[0] = exp;
  variables().AS_SGNCPR = 0;

  AS_FADD_2(exp); // (AS_FAC*4) + (AS_FAC*1) = AS_FAC*5

  exp = variables_const().AS_FAC[0];
  if (exp == 255) {
    AS_ERROR(0x45); // AS_OVERFLOW
    return;
  }
  variables().AS_FAC[0] = exp + 1; // AS_FAC*5 * 2 = AS_FAC*10
}

/**
 * AS_CON_TEN: Floating point constant 10 ($84, $20, $00, $00, $00)
 */
const std::uint8_t AS_CON_TEN[5] = {0x84, 0x20, 0x00, 0x00, 0x00};

/**
 * AS_DIV10: Divide AS_FAC by 10
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_DIV10 (inclusive) .. AS_DIV (exclusive)
 */
void AS_DIV10() {
  AS_COPY_FAC_TO_ARG_ROUNDED();
  // AS_Load AS_CON_TEN into AS_FAC (simulated AS_LOAD_FAC_FROM_YA)
  variables().AS_FAC[0] = AS_CON_TEN[0];
  variables().AS_FAC[1] = AS_CON_TEN[1];
  variables().AS_FAC[2] = AS_CON_TEN[2];
  variables().AS_FAC[3] = AS_CON_TEN[3];
  variables().AS_FAC[4] = AS_CON_TEN[4];

  // AS_FDIVT would be called here
  // AS_FDIVT(); // Divide AS_ARG by AS_FAC
}

/**
 * AS_NEGATE_FAC (AS_NEGOP): Negate the value in AS_FAC
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_NEGOP (inclusive) .. AS_RTS_18 (exclusive)
 */
void AS_NEGATE_FAC() {
  if (variables_const().AS_FAC[0] == 0)
    return;
  std::uint8_t sign = variables_const().AS_FAC_SIGN;
  variables().AS_FAC_SIGN = sign ^ 0xFF;
}

/**
 * AS_COPY_FAC_TO_ARG_ROUNDED: Round AS_FAC and copy to AS_ARG
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_COPY_FAC_TO_ARG_ROUNDED (inclusive) .. AS_RTS_14 (exclusive)
 */
void AS_COPY_FAC_TO_ARG_ROUNDED() {
  AS_ROUND_FAC();
  // Copy 6 bytes (exp + 4 mantissa + sign)
  for (int i = 0; i < 5; ++i) {
    variables().AS_ARG[i] = variables_const().AS_FAC[i];
  }
  variables().AS_ARG[5] = variables_const().AS_FAC_SIGN;
  variables().AS_FAC_EXTENSION = 0;
}

/**
 * AS_ROUND_FAC: Round AS_FAC using extension byte
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_ROUND_FAC (inclusive) .. AS_RTS_14 (exclusive)
 */
void AS_ROUND_FAC() {
  if (variables_const().AS_FAC[0] == 0)
    return;
  if (variables_const().AS_FAC_EXTENSION >= 0x80) {
    AS_INCREMENT_MANTISSA();
  }
}

/**
 * AS_INCREMENT_MANTISSA: Increment AS_FAC mantissa and re-normalize if carry
 */
void AS_INCREMENT_MANTISSA() {
  AS_INCREMENT_FAC_MANTISSA();
  if (variables_const().AS_FAC[1] == 0) {
    // High byte zeroed by carry, needs re-normalization
    // jmp AS_NORMALIZE_FAC_6
  }
}

/**
 * AS_INCREMENT_FAC_MANTISSA: Add carry to AS_FAC mantissa
 */
void AS_INCREMENT_FAC_MANTISSA() {
  for (int i = 4; i >= 1; --i) {
    std::uint8_t val = variables_const().AS_FAC[i] + 1;
    variables().AS_FAC[i] = val;
    if (val != 0)
      break;
  }
}

/**
 * AS_FLOAT: Convert signed byte in A to AS_FAC
 */
void AS_FLOAT(std::int8_t value) {
  variables().AS_FAC[1] = static_cast<std::uint8_t>(value);
  variables().AS_FAC[2] = 0;
  AS_FLOAT_1(0x88);
}

/**
 * AS_FLOAT_1: Float unsigned 16-bit value in AS_FAC_1,2 with exponent in X
 */
void AS_FLOAT_1(std::uint8_t exponent) {
  std::uint8_t hi = variables_const().AS_FAC[1];
  bool positive = (hi & 0x80) == 0;
  // ROL hi logic follows
  AS_FLOAT_2(exponent, positive);
}

/**
 * AS_FLOAT_2: Float unsigned 16-bit value in AS_FAC_1,2 with exponent and sign
 */
void AS_FLOAT_2(std::uint8_t exponent, bool positive) {
  variables().AS_FAC[4] = 0;
  variables().AS_FAC[3] = 0;
  variables().AS_FAC[0] = exponent;
  variables().AS_FAC_EXTENSION = 0;
  variables().AS_FAC_SIGN = positive ? 0 : 0xFF;
  // jmp AS_NORMALIZE_FAC_1
}

void AS_ADDACC_WITH_DIGIT(std::uint8_t digit);

/**
 * AS_FIN ($1C4A)
 * Port of Convert String to FP Value in AS_FAC.
 */
void AS_FIN() {
  // Clear working area ($99...$A3)
  // $99: AS_TMPEXP
  // $9A: AS_EXPON
  // $9B: AS_DPFLG
  // $9C: AS_EXPSGN
  // $9D-$A1: AS_FAC
  // $A2: AS_FAC_SIGN
  // $A3: AS_SERLEN
  for (int i = 0x99; i <= 0xA3; ++i) {
    WriteZeroPageByte(static_cast<std::uint8_t>(i), 0);
  }

  std::uint8_t ch = variables_const().AS_CHARAC;
  // Note: AS_CHRGET already populated AS_CHARAC and set C flag based on if it's
  // a digit. In our C++ port, we'll use AS_CHRGET() to get the current state.

  // Check if current char is a digit
  if (ch >= '0' && ch <= '9') {
    goto AS_FIN_2;
  }

  if (ch == '-') {
    WriteZeroPageByte(0xA3, 0xFF); // AS_SERLEN is $A3
    goto AS_FIN_1;
  }

  if (ch != '+') {
    goto AS_FIN_3;
  }

AS_FIN_1:
  ch = AS_CHRGET();

AS_FIN_2:
  if (ch >= '0' && ch <= '9') {
    goto AS_FIN_9;
  }

AS_FIN_3:
  if (ch == '.') {
    goto AS_FIN_10;
  }

  if (ch != 'E') {
    goto AS_FIN_7;
  }

  ch = AS_CHRGET();
  if (ch >= '0' && ch <= '9') {
    goto AS_FIN_5;
  }

  if (ch == '-') { // Token - is $D0, minus is $2D. AS_CHRGET handles tokens?
    goto AS_L_FIN_3_1;
  }

  if (ch == '+') {
    goto AS_FIN_4;
  }

  goto AS_FIN_6;

AS_L_FIN_3_1:
  // AS_EXPSGN is $9C
  WriteZeroPageByte(0x9C, 0xFF);

AS_FIN_4:
  ch = AS_CHRGET();

AS_FIN_5:
  if (ch >= '0' && ch <= '9') {
    goto AS_GETEXP;
  }

AS_FIN_6:
  if (ReadZeroPageByte(0x9C) & 0x80) {
    std::uint8_t expon = ReadZeroPageByte(0x9A);
    WriteZeroPageByte(0x9A, static_cast<std::uint8_t>(0 - expon));
  }

AS_FIN_7:
  // Number terminated, adjust exponent
  goto AS_FIN_8;

AS_FIN_10:
  // AS_DPFLG is $9B. Use BIT/ROR logic: first time sets $80, second time sets
  // $C0
  {
    std::uint8_t dpflg = ReadZeroPageByte(0x9B);
    if (dpflg & 0x80) {
      // Second decimal point terminates number
      goto AS_FIN_7;
    }
    WriteZeroPageByte(0x9B, 0x80);
    goto AS_FIN_1;
  }

AS_FIN_8: {
  std::int8_t expon = static_cast<std::int8_t>(ReadZeroPageByte(0x9A));
  std::int8_t tmpexp = static_cast<std::int8_t>(ReadZeroPageByte(0x99));
  expon -= tmpexp;
  WriteZeroPageByte(0x9A, static_cast<std::uint8_t>(expon));

  while (expon != 0) {
    if (expon < 0) {
      AS_DIV10();
      expon++;
    } else {
      AS_MUL10();
      expon--;
    }
  }

  if (ReadZeroPageByte(0xA3) & 0x80) { // AS_SERLEN is $A3
    AS_NEGATE_FAC();
  }
}
  return;

AS_FIN_9:
  // Accumulate digit into AS_FAC
  {
    std::uint8_t digit = ch;
    if (ReadZeroPageByte(0x9B) & 0x80) {
      // Count fractional digit
      WriteZeroPageByte(0x99, ReadZeroPageByte(0x99) + 1);
    }
    AS_MUL10();
    AS_ADDACC_WITH_DIGIT(digit - '0');
    goto AS_FIN_1;
  }

AS_GETEXP:
  // Accumulate exponent digit
  {
    std::uint16_t expon = ReadZeroPageByte(0x9A);
    if (expon >= 10) {
      if (!(ReadZeroPageByte(0x9C) & 0x80)) {
        AS_ERROR(0x10); // AS_OVERFLOW (AS_ERR_OVERFLOW is $10 or similar)
      }
      // AS_Large negative exponent makes AS_FAC=0 eventually
      WriteZeroPageByte(0x9A, 100); // Caps it
    } else {
      expon = expon * 10 + (ch - '0');
      WriteZeroPageByte(0x9A, static_cast<std::uint8_t>(expon));
    }
    goto AS_FIN_4;
  }
}

/**
 * AS_ADDACC ($1CD5)
 * Add (A) to AS_FAC.
 */
void AS_ADDACC_WITH_DIGIT(std::uint8_t digit) {
  AS_COPY_FAC_TO_ARG_ROUNDED();
  AS_FLOAT(static_cast<std::int8_t>(digit));
  // AS_SGNCPR is $AB
  variables().AS_SGNCPR =
      static_cast<std::uint8_t>(variables_const().AS_ARG[5] ^
                                variables_const().AS_FAC_SIGN);
  AS_FADDT();
}

/**
 * AS_ADD_EXPONENTS: AS_FAC_EXP = AS_FAC_EXP + AS_ARG_EXP
 */
void AS_ADD_EXPONENTS() {
  std::uint16_t sum =
      (std::uint16_t)variables_const().AS_FAC[0] + variables_const().AS_ARG[0];
  if (sum > 0xFF) {
    AS_ERROR(0x45); // AS_OVERFLOW
    return;
  }
  variables().AS_FAC[0] = static_cast<std::uint8_t>(sum);
}

/**
 * AS_LOAD_ARG_FROM_YA: Unpack number at Y,A into AS_ARG
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_LOAD_ARG_FROM_YA (inclusive) .. AS_FMULT (exclusive)
 */
void AS_LOAD_ARG_FROM_YA(std::uint16_t address) {
  // Unpack 5-byte float from (Y,A) into AS_ARG
  // Original uses AS_INDEX ($5E) to store the pointer.
  // For now we'll simulate the byte-by-byte move.
  for (int i = 0; i < 5; ++i) {
    // ... (complex due to byte ordering and sign bit manipulation)
  }
}

/**
 * AS_COPY_ARG_TO_FAC: Copy AS_ARG into AS_FAC
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_COPY_ARG_TO_FAC (inclusive) .. AS_MFA (exclusive)
 */
void AS_COPY_ARG_TO_FAC() {
  std::uint8_t arg_sign = variables_const().AS_ARG[5];
  variables().AS_FAC_SIGN = arg_sign;

  // Copy 5 bytes from AS_ARG to AS_FAC (exponent + 4 mantissa bytes)
  for (int i = 0; i < 5; ++i) {
    variables().AS_FAC[i] = variables_const().AS_ARG[i];
  }
  variables().AS_FAC_EXTENSION = 0;
}

/**
 * AS_NORMALIZE_FAC_2: Left-normalize FAC by whole-byte shifts.
 * Source:
 * SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * AS_Labels: AS_NORMALIZE_FAC_2 (inclusive) .. AS_ZERO_FAC (exclusive)
 *
 * - Entry: FAC mantissa may have leading zero bytes in FAC[1..4]+extension.
 * - Fast 8-bit shuffles shift mantissa bytes left, zeroing the extension byte.
 * - Shift count (A reg) accumulated: 8 per iteration, max 4 iterations = 32.
 * - When FAC[1] becomes non-zero, delegates to AS_NORMALIZE_FAC_4 for
 *   bit-level normalization with the accumulated shift count.
 * - After 32 shifts with FAC[1] still zero, falls through to AS_ZERO_FAC.
 */
void AS_NORMALIZE_FAC_2() {
  // ldy #0 / tya / clc: A=0, Y=0, carry clear
  std::uint8_t shiftCount = 0u;

  do {
    const std::uint8_t msb = variables_const().AS_FAC[1]; // ldx FAC+1
    if (msb != 0u) {                                      // bne NORMALIZE_FAC_4
      // Some 1-bits present; hand off to bit-level normalization.
      AS_NORMALIZE_FAC_4(shiftCount);
      return;
    }
    // FAC[1] still zero: fast 8-bit left shuffle of mantissa + extension.
    variables().AS_FAC[1] =
        variables_const().AS_FAC[2]; // ldx FAC+2 / stx FAC+1
    variables().AS_FAC[2] =
        variables_const().AS_FAC[3]; // ldx FAC+3 / stx FAC+2
    variables().AS_FAC[3] =
        variables_const().AS_FAC[4]; // ldx FAC+4 / stx FAC+3
    variables().AS_FAC[4] =
        variables_const().AS_FAC_EXTENSION; // ldx FAC_EXTENSION / stx FAC+4
    variables().AS_FAC_EXTENSION = 0u;      // sty FAC_EXTENSION (Y=0)
    shiftCount = static_cast<std::uint8_t>(shiftCount + 8u); // adc #8
  } while (shiftCount != 32u); // cmp #32 / bne L_NORMALIZE_FAC_2_1

  // YES, VALUE OF FAC IS ZERO: all 32-bit mantissa exhausted.
  // Fall through to ZERO_FAC.
  AS_ZERO_FAC();
}

// TODO(asm-port): AS_NORMALIZE_FAC_4 — bit-level normalization after
// byte-level shifts. Receives shift_count (number of 8-bit shifts already done
// by NORMALIZE_FAC_2 or accumulated bit count from NORMALIZE_FAC_3).
// Source: applesoft.o65.lst label NORMALIZE_FAC_4 @ 0x1880.
// Bit-shifts FAC left until FAC[1] bit-7 = 1, adjusts exponent, handles
// underflow by calling AS_ZERO_FAC, then falls through to AS_NORMALIZE_FAC_5.
void AS_NORMALIZE_FAC_4([[maybe_unused]] std::uint8_t shiftCount) {}

// TODO(asm-port): AS_ZERO_FAC — load A=0 and fall through to
// STA_IN_FAC_SIGN_AND_EXP.
// Source: applesoft.o65.lst label ZERO_FAC @ 0x184e.
// Stores 0 into FAC (exponent) and FAC_SIGN; returns.
void AS_ZERO_FAC() {
  variables().AS_FAC[0] = 0u;   // sta FAC
  variables().AS_FAC_SIGN = 0u; // sta FAC_SIGN
  // rts
}

// TODO: Implement the rest of the AS_FADD/AS_FSUB subroutines and logic.
// This requires porting AS_SHIFT_RIGHT, AS_NORMALIZE_FAC, etc.

} // namespace applesoft::asm_port
