#pragma once

/**
 * Expression evaluation routines ported from Applesoft BASIC 6502 assembly.
 * 
 * Labels: FRMEVL (0x0d7b) .. FRM_ELEMENT (0x0e60)
 */

namespace applesoft::asm_port {

/// Evaluates a BASIC expression.
/// [Source: applesoft.o65.lst:2882]
void FRMEVL();

/// Evaluates a numeric-only expression (calls FRMEVL and checks VALTYP).
/// [Source: applesoft.o65.lst:2829]
void FRMNUM();

} // namespace applesoft::asm_port
