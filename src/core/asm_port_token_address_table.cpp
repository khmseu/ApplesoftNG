// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: TOKEN_ADDRESS_TABLE (inclusive) .. UNFNC (exclusive)
// Name normalization: none

#include "core/asm_port_token_address_table.hpp"
#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
std::uint8_t CHRGET();

bool CLEAR();
bool NEW();
void LIST();
void RESTORE();
void STOP();
void ENDX();
void NEXT();
void CONT();
void LOAD();
void SAVE();
void RUN();
void GOSUB();
void GOTO();
void POP();
void DATA();
void GET();
void INPUT();
void READ();
void DIM();
void LET();
void PR_NUMBER();
void IN_NUMBER();
void IF();
void REM();
void ONGOTO();
void ONERR();
void RESUME();
void DEF();
void POKE();
void WAIT();
void PRINT(std::uint8_t a);
void CALL();
void NORMAL();
void INVERSE();
void FLASH();
void COLOR();
void VTAB();
void HIMEM();
void LOMEM();
void SPEED();

// ---------------------------------------------------------------------------
// Stub implementations for statement handlers not yet ported.
// Each will be replaced when its label range is converted.
// ---------------------------------------------------------------------------

void FOR();
static void NEXT_Handler()     { NEXT(); }
static void INPUT_Handler()    { INPUT(); }
static void DEL()              {} // TODO(asm-port): DEL
static void DIM_Handler()      { DIM(); }
static void READ_Handler()     { READ(); }
static void GR()               {} // TODO(asm-port): GR
static void TEXT()             {} // TODO(asm-port): TEXT
static void PR_NUMBER_Handler(){ PR_NUMBER(); }
static void IN_NUMBER_Handler(){ IN_NUMBER(); }
static void CALL_Handler()     { CALL(); }
static void PLOT()             {} // TODO(asm-port): PLOT
static void HLIN()             {} // TODO(asm-port): HLIN
static void VLIN()             {} // TODO(asm-port): VLIN
static void HGR2()             {} // TODO(asm-port): HGR2
static void HGR()              {} // TODO(asm-port): HGR
static void HCOLOR()           {} // TODO(asm-port): HCOLOR
static void HPLOT()            {} // TODO(asm-port): HPLOT
static void DRAW()             {} // TODO(asm-port): DRAW
static void XDRAW()            {} // TODO(asm-port): XDRAW
static void HTAB()             {} // TODO(asm-port): HTAB
static void HOME()             {} // TODO(asm-port): HOME monitor routine alias for MON_HOME.
static void MON_HOME() {
    HOME();
}
// TODO(asm-port): MON_HOME (monitor clear-screen at $FC58)
static void ROT()              {} // TODO(asm-port): ROT
static void SCALE()            {} // TODO(asm-port): SCALE
static void SHLOAD()           {} // TODO(asm-port): SHLOAD
// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: TRACE (inclusive) .. NORMAL (exclusive)
// Name normalization: none. TRACE: sec; ror TRCFLG → bit 7 set (trace on).
//                           NOTRACE: clc; ror TRCFLG → bit 7 clear (trace off).
static void TRACE() {
    const std::uint8_t trcflg = ReadZeroPageByte(ApplesoftVariables::ZP_TRCFLG);
    WriteZeroPageByte(ApplesoftVariables::ZP_TRCFLG,
                      static_cast<std::uint8_t>((trcflg >> 1u) | 0x80u));
}
static void NOTRACE() {
    const std::uint8_t trcflg = ReadZeroPageByte(ApplesoftVariables::ZP_TRCFLG);
    WriteZeroPageByte(ApplesoftVariables::ZP_TRCFLG,
                      static_cast<std::uint8_t>(trcflg >> 1u));
}
static void NORMAL_Handler()   { NORMAL(); }
static void INVERSE_Handler()  { INVERSE(); }
static void FLASH_Handler()    { FLASH(); }
static void COLOR_Handler()    { COLOR(); }
static void VTAB_Handler()     { VTAB(); }
static void HIMEM_Handler()    { HIMEM(); }
static void LOMEM_Handler()    { LOMEM(); }
static void ONERR_Handler()    { ONERR(); }
static void RESUME_Handler()   { RESUME(); }
static void RECALL()           {} // TODO(asm-port): RECALL
static void STORE()            {} // TODO(asm-port): STORE
static void SPEED_Handler()    { SPEED(); }
static void AMPERSAND_VECTOR() {} // TODO(asm-port): & dispatch (JMP vector at $03F5)
static void WAIT_Handler()     { WAIT(); }
static void DEF_Handler()      { DEF(); }
static void POKE_Handler()     { POKE(); }
static void PRINT_Handler()    { PRINT(CHRGET()); }
static void CLEAR_Handler()    { CLEAR(); }
static void GET_Handler()      { GET(); }
static void NEW_Handler()      { NEW(); }

// ---------------------------------------------------------------------------
// Branch table for Applesoft statement tokens $80–$BF.
// Index = token - $80.
// The -1 RTS-dispatch artifact from the 6502 source is dropped; plain
// function pointers are used and the caller is responsible for invoking them.
// ---------------------------------------------------------------------------
TOKEN_ADDRESS_TABLE_fn TOKEN_ADDRESS_TABLE(std::size_t index) {
    static constexpr TOKEN_ADDRESS_TABLE_fn table[] = {
        ENDX,             // [0]  $80...128...END
        FOR,              // [1]  $81...129...FOR
        NEXT_Handler,     // [2]  $82...130...NEXT
        DATA,             // [3]  $83...131...DWTA
        INPUT_Handler,    // [4]  $84...132...INPUT
        DEL,              // [5]  $85...133...DEL
        DIM_Handler,      // [6]  $86...134...DIM
        READ_Handler,     // [7]  $87...135...READ
        GR,               // [8]  $88...136...GR
        TEXT,             // [9]  $89...137...TEXT
        PR_NUMBER_Handler,// [10] $8A...138...PR#
        IN_NUMBER_Handler,// [11] $8B...139...IN#
        CALL_Handler,     // [12] $8C...140...CALL
        PLOT,             // [13] $8D...141...PLOT
        HLIN,             // [14] $8E...142...HLIN
        VLIN,             // [15] $8F...143...VLIN
        HGR2,             // [16] $90...144...HGR2
        HGR,              // [17] $91...145...HGR
        HCOLOR,           // [18] $92...146...HCOLOR=
        HPLOT,            // [19] $93...147...HPLOT
        DRAW,             // [20] $94...148...DRAW
        XDRAW,            // [21] $95...149...XDRAW
        HTAB,             // [22] $96...150...HTAB
        MON_HOME,         // [23] $97...151...HOME
        ROT,              // [24] $98...152...ROT=
        SCALE,            // [25] $99...153...SCALE=
        SHLOAD,           // [26] $9A...154...SHLOAD
        TRACE,            // [27] $9B...155...TRACE
        NOTRACE,          // [28] $9C...156...NOTRACE
        NORMAL_Handler,   // [29] $9D...157...NORMAL
        INVERSE_Handler,  // [30] $9E...158...INVERSE
        FLASH_Handler,    // [31] $9F...159...FLASH
        COLOR_Handler,    // [32] $A0...160...COLOR=
        POP,              // [33] $A1...161...POP
        VTAB_Handler,     // [34] $A2...162...VTAB
        HIMEM_Handler,    // [35] $A3...163...HIMEM:
        LOMEM_Handler,    // [36] $A4...164...LOMEM:
        ONERR_Handler,    // [37] $A5...165...ONERR
        RESUME_Handler,   // [38] $A6...166...RESUME
        RECALL,           // [39] $A7...167...RECALL
        STORE,            // [40] $A8...168...STORE
        SPEED_Handler,    // [41] $A9...169...SPEED=
        LET,              // [42] $AA...170...LET
        GOTO,             // [43] $AB...171...GOTO
        RUN,              // [44] $AC...172...RUN
        IF,               // [45] $AD...173...IF
        RESTORE,          // [46] $AE...174...RESTORE
        AMPERSAND_VECTOR, // [47] $AF...175...&
        GOSUB,            // [48] $B0...176...GOSUB
        POP,              // [49] $B1...177...RETURN (same handler as POP)
        REM,              // [50] $B2...178...REM
        STOP,             // [51] $B3...179...STOP
        ONGOTO,           // [52] $B4...180...ON
        WAIT_Handler,     // [53] $B5...181...WAIT
        LOAD,             // [54] $B6...182...LOAD
        SAVE,             // [55] $B7...183...SAVE
        DEF_Handler,      // [56] $B8...184...DEF
        POKE_Handler,     // [57] $B9...185...POKE
        PRINT_Handler,    // [58] $BA...186...PRINT
        CONT,             // [59] $BB...187...CONT
        LIST,             // [60] $BC...188...LIST
        CLEAR_Handler,    // [61] $BD...189...CLEAR
        GET_Handler,      // [62] $BE...190...GET
        NEW_Handler,      // [63] $BF...191...NEW
    };
    return table[index];
}

} // namespace applesoft::asm_port
