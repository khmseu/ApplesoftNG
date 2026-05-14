#include "core/applesoft_variables.hpp"
#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void ERROR(std::uint8_t error_code);
void LOAD_ARG_FROM_YA();
void COPY_ARG_TO_FAC();
void FADDT();

// Forward declarations of subroutines used within FADD/FSUB
void SHIFT_RIGHT();
void SHIFT_RIGHT_4();
void COMPLEMENT_FAC();
void NORMALIZE_FAC_1();
void NORMALIZE_FAC_2();
void NORMALIZE_FAC_5();

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
