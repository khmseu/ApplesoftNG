#include "core/applesoft_variables.hpp"
#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void ERROR(std::uint8_t error_code);
void LOAD_ARG_FROM_YA();
void COPY_ARG_TO_FAC();

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
    
    // Status is set by LDA FAC in original, used by JMP FADDT
    // We just call FADDT directly.
}

/**
 * FADD: FAC = (Y,A) + FAC
 * Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
 * Labels: FADD (inclusive) .. FADDT (exclusive)
 */
void FADD() {
    LOAD_ARG_FROM_YA();
    // Falls through to FADDT
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
    
    // Original uses X as pointer to ARG or FAC for shifting.
    // For now we'll implement the logic more directly if possible.
}

// TODO: Implement the rest of the FADD/FSUB subroutines and logic.
// This requires porting SHIFT_RIGHT, NORMALIZE_FAC, etc.

} // namespace applesoft::asm_port
