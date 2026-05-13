// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: UNFNC (inclusive) .. MATHTBL (exclusive)
// Name normalization: none
//
// UNFNC is a direct-address table (no RTS-dispatch -1 offset) for Applesoft
// unary and built-in function tokens $D2-$EA. The caller dispatches by
// computing (token - 0xD2) as the index and invoking the returned pointer.

#include "core/asm_port_unfnc.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_strlit.hpp"
#include "core/asm_port_strlt2.hpp"

namespace applesoft::asm_port {

std::uint8_t MON_PREAD();
void CONINT();
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void SNGFLT(std::uint8_t value);
void PEEK();

void PDL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PDL (inclusive) .. NXDIM (exclusive)
    // Name normalization: none (assembler label PDL kept verbatim).

    CONINT();
    SNGFLT(MON_PREAD());
}

// ---------------------------------------------------------------------------
// Stub implementations for function handlers not yet ported.
// ---------------------------------------------------------------------------

static void SGN()      {} // TODO(asm-port): SGN        $D2...210
static void INT_fn()   {} // TODO(asm-port): INT        $D3...211  (INT is a C++ keyword; normalized to INT_fn)
static void ABS()      {} // TODO(asm-port): ABS        $D4...212
static void USR()      {} // TODO(asm-port): USR        $D5...213  (user-defined function via zero-page JMP at $0A)
static void FRE()      {} // TODO(asm-port): FRE        $D6...214
static void ERROR()    {} // TODO(asm-port): ERROR/SCRN $D7...215  (SCRN( token dispatches to ERROR handler)
static void PDL_fn()   { PDL(); }
void POS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: POS (inclusive) .. SNGFLT (exclusive)
    // Name normalization: none (assembler label POS kept verbatim).

    SNGFLT(ReadZeroPageByte(ApplesoftVariables::ZP_MON_CH));
}
static void SQR()      {} // TODO(asm-port): SQR        $DA...218
static void RND()      {} // TODO(asm-port): RND        $DB...219
static void LOG()      {} // TODO(asm-port): LOG        $DC...220
static void EXP()      {} // TODO(asm-port): EXP        $DD...221
static void COS()      {} // TODO(asm-port): COS        $DE...222
static void SIN()      {} // TODO(asm-port): SIN        $DF...223
static void TAN()      {} // TODO(asm-port): TAN        $E0...224
static void ATN()      {} // TODO(asm-port): ATN        $E1...225
static void PEEK_fn()  { PEEK(); }
static void LEN_fn()   { LEN(); }
static void STR_fn()   { STR(); }
static void VAL()        {} // TODO(asm-port): VAL        $E5...229
static void ASC_fn()     { ASC(); }
static void CHRSTR_fn()  { CHRSTR(); }
static void LEFTSTR_fn() { LEFTSTR(); }
static void RIGHTSTR_fn(){ RIGHTSTR(); }
static void MIDSTR_fn()  { MIDSTR(); }

// ---------------------------------------------------------------------------
// Dispatch table for Applesoft unary/built-in function tokens $D2-$EA.
// Index = token - $D2. Direct addresses — no RTS-dispatch -1 adjustment.
// ---------------------------------------------------------------------------
UNFNC_fn UNFNC(std::size_t index) {
    static constexpr UNFNC_fn table[] = {
        SGN,      // [0]  $D2...210...SGN
        INT_fn,   // [1]  $D3...211...INT   (INT_fn: INT is a reserved C++ keyword)
        ABS,      // [2]  $D4...212...ABS
        USR,      // [3]  $D5...213...USR
        FRE,      // [4]  $D6...214...FRE
        ERROR,    // [5]  $D7...215...SCRN(
        PDL_fn,   // [6]  $D8...216...PDL
        POS,      // [7]  $D9...217...POS
        SQR,      // [8]  $DA...218...SQR
        RND,      // [9]  $DB...219...RND
        LOG,      // [10] $DC...220...LOG
        EXP,      // [11] $DD...221...EXP
        COS,      // [12] $DE...222...COS
        SIN,      // [13] $DF...223...SIN
        TAN,      // [14] $E0...224...TAN
        ATN,      // [15] $E1...225...ATN
        PEEK_fn,  // [16] $E2...226...PEEK
        LEN_fn,   // [17] $E3...227...LEN
        STR_fn,   // [18] $E4...228...STR$
        VAL,        // [19] $E5...229...VAL
        ASC_fn,     // [20] $E6...230...ASC
        CHRSTR_fn,  // [21] $E7...231...CHR$
        LEFTSTR_fn, // [22] $E8...232...LEFT$
        RIGHTSTR_fn,// [23] $E9...233...RIGHT$
        MIDSTR_fn,  // [24] $EA...234...MID$
    };
    return table[index];
}

} // namespace applesoft::asm_port
