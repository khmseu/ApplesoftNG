#include "core/applesoft_variables.hpp"
#include "core/asm_port_math.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_error_handling.hpp"
#include <cstdint>

namespace applesoft::asm_port {

extern std::uint8_t ReadZeroPageByte(std::uint8_t address);
extern void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
extern void LOAD_ARG_FROM_YA();
extern void COPY_ARG_TO_FAC();

// Forward declarations of subroutines used within FADD/FSUB
void SHIFT_RIGHT();
void SHIFT_RIGHT_4();
void COMPLEMENT_FAC();
void NORMALIZE_FAC_1();
void NORMALIZE_FAC_2();
void NORMALIZE_FAC_5();
void ROUND_FAC();
void COPY_FAC_TO_ARG_ROUNDED();
void FLOAT();
void FLOAT_1(std::uint8_t exponent);
void FLOAT_2(std::uint8_t exponent, bool positive);
void MULTIPLY_FAC_BY_TEN();
void DIVIDE_FAC_BY_TEN();
void ADD_EXPONENTS();
void ADD_EXPONENTS_1();
void NEGATE_FAC();
void INCREMENT_FAC_MANTISSA();
void INCREMENT_MANTISSA();

/**
 * FSUB: FAC = (Y,A) - FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: FSUB (inclusive) .. FSUBT (exclusive)
 */
void FSUB() {
    LOAD_ARG_FROM_YA();
    // Falls through to FSUBT
}

/**
 * FSUBT: FAC = ARG - FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: FSUBT (inclusive) .. FADD_1 (exclusive)
 */
void FSUBT() {
    std::uint8_t fac_sign = ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN);
    fac_sign ^= 0xFF;
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, fac_sign);
    
    std::uint8_t arg_sign = ReadZeroPageByte(ApplesoftVariables::ZP_ARG_SIGN);
    WriteZeroPageByte(ApplesoftVariables::ZP_SGNCPR, fac_sign ^ arg_sign);
    
    FADDT();
}

/**
 * FADD: FAC = (Y,A) + FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: FADD (inclusive) .. FADDT (exclusive)
 */
void FADD() {
    LOAD_ARG_FROM_YA();
    FADDT();
}

/**
 * FADDT: FAC = ARG + FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: FADDT (inclusive) .. FADD_2 (exclusive)
 */
void FADDT() {
    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC) == 0) {
        COPY_ARG_TO_FAC();
        return;
    }
    
    std::uint8_t fac_ext = ReadZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION);
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG_EXTENSION, fac_ext);
    
    // Continue porting here...
}

/**
 * FADD_2: Entry point for adding A to FAC (used by MUL10)
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: FADD_2 (inclusive) .. FADD_3 (exclusive)
 */
void FADD_2(std::uint8_t exponent) {
    if (exponent == 0) return;
    
    std::uint8_t fac_exp = ReadZeroPageByte(ApplesoftVariables::ZP_FAC);
    std::int16_t diff = (std::int16_t)exponent - (std::int16_t)fac_exp;
    
    if (diff == 0) {
        // FADD_3 logic would follow
    } else if (diff < 0) {
        // ARG has smaller exponent
        // L_FADD_2_1 logic
    } else {
        // FAC has smaller exponent
        WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exponent);
        WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, ReadZeroPageByte(ApplesoftVariables::ZP_ARG_SIGN));
        // ... (complex alignment and subtraction logic)
    }
    // This is a partial stub to satisfy MUL10 for now.
}

/**
 * MUL10: Multiply FAC by 10
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: MUL10 (inclusive) .. L_MUL10_1 (exclusive)
 */
void MUL10() {
    COPY_FAC_TO_ARG_ROUNDED();
    std::uint8_t exp = ReadZeroPageByte(ApplesoftVariables::ZP_FAC);
    if (exp == 0) return; // FAC=0
    
    if (exp > 253) { // exp + 2 > 255
        ERROR(0x45); // OVERFLOW (ERR_OVERFLOW)
        return;
    }
    
    exp += 2; // FAC * 4
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exp);
    WriteZeroPageByte(ApplesoftVariables::ZP_SGNCPR, 0);
    
    FADD_2(exp); // (FAC*4) + (FAC*1) = FAC*5
    
    exp = ReadZeroPageByte(ApplesoftVariables::ZP_FAC);
    if (exp == 255) {
        ERROR(0x45); // OVERFLOW
        return;
    }
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exp + 1); // FAC*5 * 2 = FAC*10
}

/**
 * CON_TEN: Floating point constant 10 ($84, $20, $00, $00, $00)
 */
const std::uint8_t CON_TEN[5] = {0x84, 0x20, 0x00, 0x00, 0x00};

/**
 * DIV10: Divide FAC by 10
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: DIV10 (inclusive) .. DIV (exclusive)
 */
void DIV10() {
    COPY_FAC_TO_ARG_ROUNDED();
    // Load CON_TEN into FAC (simulated LOAD_FAC_FROM_YA)
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, CON_TEN[0]);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 1, CON_TEN[1]);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 2, CON_TEN[2]);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 3, CON_TEN[3]);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 4, CON_TEN[4]);
    
    // FDIVT would be called here
    // FDIVT(); // Divide ARG by FAC
}

/**
 * NEGATE_FAC (NEGOP): Negate the value in FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: NEGOP (inclusive) .. RTS_18 (exclusive)
 */
void NEGATE_FAC() {
    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC) == 0) return;
    std::uint8_t sign = ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, sign ^ 0xFF);
}

/**
 * COPY_FAC_TO_ARG_ROUNDED: Round FAC and copy to ARG
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: COPY_FAC_TO_ARG_ROUNDED (inclusive) .. RTS_14 (exclusive)
 */
void COPY_FAC_TO_ARG_ROUNDED() {
    ROUND_FAC();
    // Copy 6 bytes (exp + 4 mantissa + sign)
    for (int i = 0; i < 6; ++i) {
        std::uint8_t val = ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + i));
        WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_ARG + i), val);
    }
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION, 0);
}

/**
 * ROUND_FAC: Round FAC using extension byte
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: ROUND_FAC (inclusive) .. RTS_14 (exclusive)
 */
void ROUND_FAC() {
    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC) == 0) return;
    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION) >= 0x80) {
        INCREMENT_MANTISSA();
    }
}

/**
 * INCREMENT_MANTISSA: Increment FAC mantissa and re-normalize if carry
 */
void INCREMENT_MANTISSA() {
    INCREMENT_FAC_MANTISSA();
    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC + 1) == 0) {
        // High byte zeroed by carry, needs re-normalization
        // jmp NORMALIZE_FAC_6
    }
}

/**
 * INCREMENT_FAC_MANTISSA: Add carry to FAC mantissa
 */
void INCREMENT_FAC_MANTISSA() {
    for (int i = 4; i >= 1; --i) {
        std::uint8_t addr = static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + i);
        std::uint8_t val = ReadZeroPageByte(addr) + 1;
        WriteZeroPageByte(addr, val);
        if (val != 0) break;
    }
}

/**
 * FLOAT: Convert signed byte in A to FAC
 */
void FLOAT(std::int8_t value) {
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 1, static_cast<std::uint8_t>(value));
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 2, 0);
    FLOAT_1(0x88);
}

/**
 * FLOAT_1: Float unsigned 16-bit value in FAC_1,2 with exponent in X
 */
void FLOAT_1(std::uint8_t exponent) {
    std::uint8_t hi = ReadZeroPageByte(ApplesoftVariables::ZP_FAC + 1);
    bool positive = (hi & 0x80) == 0;
    // ROL hi logic follows
    FLOAT_2(exponent, positive);
}

/**
 * FLOAT_2: Float unsigned 16-bit value in FAC_1,2 with exponent and sign
 */
void FLOAT_2(std::uint8_t exponent, bool positive) {
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 4, 0);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC + 3, 0);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exponent);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION, 0);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, positive ? 0 : 0xFF);
    // jmp NORMALIZE_FAC_1
}

// Forward declarations for internal math routines
static void SHIFT_RIGHT(std::uint8_t count);
static void SHIFT_RIGHT_4(std::uint8_t count);
static void COMPLEMENT_FAC();
static void COMPLEMENT_FAC_MANTISSA();
static void INCREMENT_FAC_MANTISSA();
static void NORMALIZE_FAC_1();
static void NORMALIZE_FAC_2();
static void NORMALIZE_FAC_5();
static void NORMALIZE_FAC_6();
static void COPY_ARG_TO_FAC();
static void ZERO_FAC();

/**
 * FSUB: FAC = ARG - FAC
 */
void FSUB() {
    NEGATE_FAC();
    FADD();
}

/**
 * FADD: FAC = (Y,A) + FAC
 */
void FADD() {
    // If we have (Y,A) address, LOAD_ARG_FROM_YA then FADDT
    // For now, assume ARG is already loaded if called with no arguments
    FADDT();
}

/**
 * FADDT: FAC = ARG + FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: FADDT (inclusive) .. RTS_12 (exclusive)
 */
void FADDT() {
    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC) == 0) {
        COPY_ARG_TO_FAC();
        return;
    }

    std::uint8_t fac_ext = ReadZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION);
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG_EXTENSION, fac_ext);
    
    std::uint8_t arg_exp = ReadZeroPageByte(ApplesoftVariables::ZP_ARG);
    if (arg_exp == 0) return; // RTS_10

    int diff = static_cast<int>(arg_exp) - static_cast<int>(ReadZeroPageByte(ApplesoftVariables::ZP_FAC));
    if (diff == 0) {
        // FADD_3
    } else if (diff < 0) {
        // ARG has smaller exponent, shift ARG
        if (diff < -8) {
            // FADD_1
            SHIFT_RIGHT(static_cast<std::uint8_t>(-diff));
        } else {
            // FADD_2_2
            std::uint8_t shift = static_cast<std::uint8_t>(-diff);
            std::uint8_t ext = ReadZeroPageByte(ApplesoftVariables::ZP_ARG_EXTENSION);
            // ...
        }
    } else {
        // FAC has smaller exponent, shift FAC
        WriteZeroPageByte(ApplesoftVariables::ZP_FAC, arg_exp);
        std::uint8_t arg_sign = ReadZeroPageByte(ApplesoftVariables::ZP_ARG_SIGN);
        WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, arg_sign);
        
        // Complement diff because CARRY was set (SEC; SBC)
        std::uint8_t count = static_cast<std::uint8_t>((~static_cast<std::uint8_t>(diff)) + 1);
        // ...
    }
}

/**
 * LOAD_ARG_FROM_YA: Unpack number at (Y,A) into ARG
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: LOAD_ARG_FROM_YA (inclusive) .. RTS_REG (exclusive)
 */
void LOAD_ARG_FROM_YA() {
    std::uint16_t addr = (static_cast<std::uint16_t>(GetRegY()) << 8) | GetRegA();
    LOAD_ARG_FROM_YA(addr);
}

void LOAD_ARG_FROM_YA(std::uint16_t address) {
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG + 4, ReadMemory(address + 4));
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG + 3, ReadMemory(address + 3));
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG + 2, ReadMemory(address + 2));
    
    std::uint8_t arg_sign = ReadMemory(address + 1);
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG_SIGN, arg_sign);
    WriteZeroPageByte(ApplesoftVariables::ZP_SGNCPR, arg_sign ^ ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN));
    
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG_MANTISSA, arg_sign | 0x80); // Implicit bit
    
    WriteZeroPageByte(ApplesoftVariables::ZP_ARG, ReadMemory(address + 0));
}

/**
 * SHIFT_RIGHT: Main entry to right shift
 */
static void SHIFT_RIGHT(std::uint8_t count) {
    // ...
}

static void SHIFT_RIGHT_4(std::uint8_t count) {
    // ...
}

static void COMPLEMENT_FAC() {
    std::uint8_t sign = ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, sign ^ 0xFF);
}

static void COMPLEMENT_FAC_MANTISSA() {
    for (int i = 1; i <= 4; ++i) {
        std::uint8_t addr = static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + i);
        WriteZeroPageByte(addr, ReadZeroPageByte(addr) ^ 0xFF);
    }
    std::uint8_t ext = ReadZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION, ext ^ 0xFF);
    
    // inc ext
    ext = ReadZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION) + 1;
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION, ext);
    if (ext == 0) {
        INCREMENT_FAC_MANTISSA();
    }
}

static void COPY_ARG_TO_FAC() {
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, ReadZeroPageByte(ApplesoftVariables::ZP_ARG_SIGN));
    for (int i = 0; i < 5; ++i) {
        WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + i), 
                         ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_ARG + i)));
    }
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION, 0);
}

static void ZERO_FAC() {
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, 0);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, 0);
}

void ADDACC_WITH_DIGIT(std::uint8_t digit);

/**
 * FIN ($1C4A)
 * Port of Convert String to FP Value in FAC.
 */
void FIN() {
    // Clear working area ($99...$A3)
    // $99: TMPEXP
    // $9A: EXPON
    // $9B: DPFLG
    // $9C: EXPSGN
    // $9D-$A1: FAC
    // $A2: FAC_SIGN
    // $A3: SERLEN
    for (int i = 0x99; i <= 0xA3; ++i) {
        WriteZeroPageByte(static_cast<std::uint8_t>(i), 0);
    }

    std::uint8_t ch = ReadZeroPageByte(ApplesoftVariables::ZP_CHARAC);
    // Note: CHRGET already populated CHARAC and set C flag based on if it's a digit.
    // In our C++ port, we'll use CHRGET() to get the current state.
    
    // Check if current char is a digit
    if (ch >= '0' && ch <= '9') {
        goto FIN_2;
    }

    if (ch == '-') {
        WriteZeroPageByte(0xA3, 0xFF); // SERLEN is $A3
        goto FIN_1;
    }

    if (ch != '+') {
        goto FIN_3;
    }

FIN_1:
    ch = CHRGET();

FIN_2:
    if (ch >= '0' && ch <= '9') {
        goto FIN_9;
    }

FIN_3:
    if (ch == '.') {
        goto FIN_10;
    }

    if (ch != 'E') {
        goto FIN_7;
    }

    ch = CHRGET();
    if (ch >= '0' && ch <= '9') {
        goto FIN_5;
    }

    if (ch == '-') { // Token - is $D0, minus is $2D. CHRGET handles tokens?
        goto L_FIN_3_1;
    }

    if (ch == '+') {
        goto FIN_4;
    }
    
    goto FIN_6;

L_FIN_3_1:
    // EXPSGN is $9C
    WriteZeroPageByte(0x9C, 0xFF);

FIN_4:
    ch = CHRGET();

FIN_5:
    if (ch >= '0' && ch <= '9') {
        goto GETEXP;
    }

FIN_6:
    if (ReadZeroPageByte(0x9C) & 0x80) {
        std::uint8_t expon = ReadZeroPageByte(0x9A);
        WriteZeroPageByte(0x9A, static_cast<std::uint8_t>(0 - expon));
    }

FIN_7:
    // Number terminated, adjust exponent
    goto FIN_8;

FIN_10:
    // DPFLG is $9B. Use BIT/ROR logic: first time sets $80, second time sets $C0
    {
        std::uint8_t dpflg = ReadZeroPageByte(0x9B);
        if (dpflg & 0x80) {
            // Second decimal point terminates number
            goto FIN_7;
        }
        WriteZeroPageByte(0x9B, 0x80);
        goto FIN_1;
    }

FIN_8:
    {
        std::int8_t expon = static_cast<std::int8_t>(ReadZeroPageByte(0x9A));
        std::int8_t tmpexp = static_cast<std::int8_t>(ReadZeroPageByte(0x99));
        expon -= tmpexp;
        WriteZeroPageByte(0x9A, static_cast<std::uint8_t>(expon));
        
        while (expon != 0) {
            if (expon < 0) {
                DIV10();
                expon++;
            } else {
                MUL10();
                expon--;
            }
        }
        
        if (ReadZeroPageByte(0xA3) & 0x80) { // SERLEN is $A3
            NEGATE_FAC();
        }
    }
    return;

FIN_9:
    // Accumulate digit into FAC
    {
        std::uint8_t digit = ch;
        if (ReadZeroPageByte(0x9B) & 0x80) {
            // Count fractional digit
            WriteZeroPageByte(0x99, ReadZeroPageByte(0x99) + 1);
        }
        MUL10();
        ADDACC_WITH_DIGIT(digit - '0');
        goto FIN_1;
    }

GETEXP:
    // Accumulate exponent digit
    {
        std::uint16_t expon = ReadZeroPageByte(0x9A);
        if (expon >= 10) {
            if (!(ReadZeroPageByte(0x9C) & 0x80)) {
                ERROR(0x10); // OVERFLOW (ERR_OVERFLOW is $10 or similar)
            }
            // Large negative exponent makes FAC=0 eventually
            WriteZeroPageByte(0x9A, 100); // Caps it
        } else {
            expon = expon * 10 + (ch - '0');
            WriteZeroPageByte(0x9A, static_cast<std::uint8_t>(expon));
        }
        goto FIN_4;
    }
}

/**
 * ADDACC ($1CD5)
 * Add (A) to FAC.
 */
void ADDACC_WITH_DIGIT(std::uint8_t digit) {
    COPY_FAC_TO_ARG_ROUNDED();
    FLOAT(static_cast<std::int8_t>(digit));
    // SGNCPR is $AB
    WriteZeroPageByte(0xAB, ReadZeroPageByte(ApplesoftVariables::ZP_ARG_SIGN) ^ ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN));
    FADDT();
}

/**
 * ADD_EXPONENTS: FAC_EXP = FAC_EXP + ARG_EXP
 */
void ADD_EXPONENTS() {
    std::uint16_t sum = (std::uint16_t)ReadZeroPageByte(ApplesoftVariables::ZP_FAC) + ReadZeroPageByte(ApplesoftVariables::ZP_ARG);
    if (sum > 0xFF) {
        ERROR(0x45); // OVERFLOW
        return;
    }
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, static_cast<std::uint8_t>(sum));
}

/**
 * LOAD_ARG_FROM_YA: Unpack number at Y,A into ARG
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: LOAD_ARG_FROM_YA (inclusive) .. FMULT (exclusive)
 */
void LOAD_ARG_FROM_YA(std::uint16_t address) {
    // Unpack 5-byte float from (Y,A) into ARG
    // Original uses INDEX ($5E) to store the pointer.
    // For now we'll simulate the byte-by-byte move.
    for (int i = 0; i < 5; ++i) {
        // ... (complex due to byte ordering and sign bit manipulation)
    }
}

/**
 * COPY_ARG_TO_FAC: Copy ARG into FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: COPY_ARG_TO_FAC (inclusive) .. MFA (exclusive)
 */
void COPY_ARG_TO_FAC() {
    std::uint8_t arg_sign = ReadZeroPageByte(ApplesoftVariables::ZP_ARG_SIGN);
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN, arg_sign);
    
    // Copy 5 bytes from ARG to FAC (exponent + 4 mantissa bytes)
    for (int i = 0; i < 5; ++i) {
        std::uint8_t val = ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_ARG + i));
        WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + i), val);
    }
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC_EXTENSION, 0);
}

// TODO: Implement the rest of the FADD/FSUB subroutines and logic.
// This requires porting SHIFT_RIGHT, NORMALIZE_FAC, etc.

} // namespace applesoft::asm_port
