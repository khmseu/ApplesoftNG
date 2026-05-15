#include "core/asm_port_frmevl.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_math.hpp"
#include "core/asm_port_mathtbl.hpp"
#include <cstdint>

namespace applesoft::asm_port {

// TODO(asm-port): Implement these dependencies
void FRM_ELEMENT() {
    // Stub for 0x0e60
}

void CHKNUM() {
    // Stub for 0x0d6a
}

void CAT() {
    // Stub for 0x1597
}

// -------------------------------------------------------------------------
// FRMEVL Porting Plan:
// 1. Map zero-page variables (TXTPTR, VALTYP, CPRTYP, CPRMASK, etc.) to ApplesoftVariables.
// 2. Handle the 6502 stack manipulation (PRECEDENCE, OPERATOR_ADDR) using a C++ recursion or explicit stack.
// 3. Port the relational operator parsing loop (FRMEVL_2).
// 4. Implement precedence testing and recursive calls (FRM_RECURSE).
// 5. Preserve the "funny way" RTS via jumping to stacked addresses.
// -------------------------------------------------------------------------

void FRMNUM() {
    FRMEVL();
    CHKNUM();
}

void FRMEVL() {
    auto& vars = variables();

    // FRMEVL: ldx TXTPTR; bne L_FRMEVL_1; dec TXTPTR+1; L_FRMEVL_1: dec TXTPTR
    uint16_t txtptr = vars.readWord(ApplesoftVariables::ZP_TXTPTR);
    txtptr--;
    vars.writeWord(ApplesoftVariables::ZP_TXTPTR, txtptr);

    // Initial precedence = 0
    uint8_t precedence = 0;

    // FRMEVL_1 label loop/entry
    auto frmevl_1 = [&](uint8_t p) {
        // PHA (push CPRTYP or similar? No, the .byt $24 trick skips it)
        // PHA (save last precedence)
        // CHKMEM
        CHKMEM();
        FRM_ELEMENT();
        vars.writeByte(ApplesoftVariables::ZP_CPRTYP, 0);

        while (true) {
            // FRMEVL_2:
            CHRGOT();
            uint8_t token = vars.readByte(ApplesoftVariables::ZP_CHARAC);
            
            // Relational operator loop
            while (true) {
                if (token < 0xCF || token > 0xD1) break; // Not >, =, <
                
                uint8_t cprtyp = vars.readByte(ApplesoftVariables::ZP_CPRTYP);
                uint8_t relop_bit = 0;
                if (token == 0xCF) relop_bit = 1;      // >
                else if (token == 0xD0) relop_bit = 2; // =
                else if (token == 0xD1) relop_bit = 4; // <

                if ((cprtyp & relop_bit) != 0) SYNERR(); // Redundant relop
                vars.writeByte(ApplesoftVariables::ZP_CPRTYP, cprtyp | relop_bit);
                
                CHRGET();
                token = vars.readByte(ApplesoftVariables::ZP_CHARAC);
            }

            uint8_t cprtyp = vars.readByte(ApplesoftVariables::ZP_CPRTYP);
            if (cprtyp != 0) {
                // FRM_RELATIONAL branch
                uint8_t valtyp = vars.readByte(ApplesoftVariables::ZP_VALTYP);
                // (VALTYP) = 0 (NUMERIC), = $FF (STRING) -> lsr then rol with carry (token type)
                // This logic is complex; skipping to precedence test for now.
                // ...
            }

            // Normal Math Precedence check
            if (token < 0xC8 || token > 0xCE) {
                // NOTMATH
                return; // Simplified EXIT/GOEX
            }

            // ... implementation of math dispatch and recursion ...
            // For now, stubbing the loop exit.
            break;
        }
    };

    frmevl_1(precedence);
}

} // namespace applesoft::asm_port
