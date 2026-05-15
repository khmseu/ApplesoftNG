#include "core/asm_port_chrget.hpp"
#include "core/applesoft_variables.hpp"

#include <cstdint>

namespace applesoft::asm_port {

// Memory access helpers (defined in asm_port_core.cpp or local declarations)
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
std::uint8_t ReadProgramByte(std::uint16_t address);

/**
 * @brief AS_Logic for AS_CHRGOT and AS_CHRGET.
 * 
 * Historical implementation (AS_GENERIC_CHRGET):
 * 210b: e6 b8     inc AS_TXTPTR
 * 210d: d0 02     bne +2
 * 210f: e6 b9     inc AS_TXTPTR+1
 * 2111: ad .. ..  lda AS_TXTPTR_VALUE
 * 2114: c9 3a     cmp #':'        ; End of statement
 * 2116: b0 0a     bcs RTS         ; >= ':' (incl. lowercase/tokens) -> Not a digit
 * 2118: c9 20     cmp #' '        ; Space
 * 211a: f0 ef     beq AS_CHRGET      ; Skip spaces
 * 211c: 38        sec
 * 211d: e9 30     sbc #'0'        ; Digit check
 * 211f: 38        sec
 * 2120: e9 d0     sbc #$d0        ; Final range check / flag setting
 * 2122: 60        rts
 */
static std::uint8_t AS_CHRGET_INTERNAL(bool increment) {
    constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
    
    std::uint16_t txtptr = ReadZeroPageWord(kAS_TXTPTR);

    while (true) {
        if (increment) {
            txtptr++;
            WriteZeroPageWord(kAS_TXTPTR, txtptr);
        }
        
        std::uint8_t c = ReadProgramByte(txtptr);
        
        if (c == ' ') {
            increment = true; // Always increment after seeing a space
            continue;
        }

        // Flags are set based on character:
        // Carry: set if NOT a digit ('0'-'9'), clear if IS a digit.
        // Zero: set if ':' (0x3A) or NULL (0x00), clear otherwise.
        // Actually, the 6502 code does:
        // CMP #':'
        // BCS RTS (Carry set if >= ':', which includes ':', ';', '<', '=', '>', '?', '@', 'A'...)
        // But ':' is the only one that needs the Zero flag set.
        
        // AS_Let's re-simulate the flags.
        // C-reg (Carry) logic in 6502:
        // SBC #$30 (sbc '0')
        // SBC #$D0 (sbc -0x30)
        // A simple version:
        // Carry is clear if ('0' <= c <= '9')
        // Zero is set if (c == ':' || c == 0)
        
        // We don't return processor flags directly in C++, but callers should
        // use helper functions to check these conditions if they are porting
        // branch logic.
        
        return c;
    }
}

std::uint8_t AS_CHRGET() {
    return AS_CHRGET_INTERNAL(true);
}

std::uint8_t AS_CHRGOT() {
    return AS_CHRGET_INTERNAL(false);
}

} // namespace applesoft::asm_port
