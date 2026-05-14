// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MATHTBL (inclusive) .. TOKEN_NAME_TABLE (exclusive)
// Name normalization: OR -> OR_op (OR is a C++ keyword).
//
// MATHTBL is a 10-entry table mixing a one-byte precedence code with a
// two-byte RTS-dispatch handler address for each math operator token $C8-$D1.
// Sub-labels M_NEG (index 7), MEQUU (index 8), M_REL (index 9) mark entries
// that the interpreter jumps to directly; exposed as M_NEG_IDX etc. in the header.
// The -1 RTS-dispatch offset is dropped; callers invoke entry.handler directly.

#include "core/asm_port_mathtbl.hpp"
#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {

void OR();
void RELOPS();

// ---------------------------------------------------------------------------
// Stub implementations for math operator handlers not yet ported.
// ---------------------------------------------------------------------------

static void FADDT()  {} // TODO(asm-port): FADDT  $C8...200...+
static void FSUBT()  {} // TODO(asm-port): FSUBT  $C9...201...-
static void FMULTT() {} // TODO(asm-port): FMULTT $CA...202...*
static void FDIVT()  {} // TODO(asm-port): FDIVT  $CB...203.../
static void FPWRT()  {} // TODO(asm-port): FPWRT  $CC...204...^

void OR_op() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: OR (inclusive) .. ANDOP (exclusive)
    // Name normalization: OR -> OR_op in MATHTBL dispatch (OR is table label).

    OR();
}

void NEGOP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEGOP (inclusive) .. CON_LOG_E (exclusive)
    // Name normalization: none (assembler label NEGOP kept verbatim).

    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kFAC_SIGN = ApplesoftVariables::ZP_FAC_SIGN;

    // ROM: if FAC exponent is zero, value is 0 so sign toggle is skipped.
    if (variables_const().readByte(kFAC) == 0u) {
        return;
    }

    const std::uint8_t sign = variables_const().readByte(kFAC_SIGN);
    variables().writeByte(kFAC_SIGN, static_cast<std::uint8_t>(sign ^ 0xffu));
}

static void EQUOP()  {} // TODO(asm-port): EQUOP  $D0...208...=

// ---------------------------------------------------------------------------
// Math operator table: precedence + handler for tokens $C8-$D1.
// Index = token - $C8.
// ---------------------------------------------------------------------------
MathTblEntry MATHTBL(std::size_t index) {
    static constexpr MathTblEntry table[] = {
        { P_ADD, FADDT  }, // [0] M_ADD  $C8...200...+
        { P_ADD, FSUBT  }, // [1]        $C9...201...-
        { P_MUL, FMULTT }, // [2]        $CA...202...*
        { P_MUL, FDIVT  }, // [3]        $CB...203.../
        { P_PWR, FPWRT  }, // [4]        $CC...204...^
        { P_AND, ANDOP  }, // [5]        $CD...205...AND
        { P_OR,  OR_op  }, // [6]        $CE...206...OR
        { P_NEQ, NEGOP  }, // [7] M_NEG  $CF...207...>
        { P_NEQ, EQUOP  }, // [8] MEQUU  $D0...208...=
        { P_REL, RELOPS }, // [9] M_REL  $D1...209...< (dispatches to core RELOPS)
    };
    return table[index];
}

} // namespace applesoft::asm_port
