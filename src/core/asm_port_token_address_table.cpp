// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: TOKEN_ADDRESS_TABLE (inclusive) .. UNFNC (exclusive)
// Name normalization: none

#include "core/asm_port_token_address_table.hpp"

namespace applesoft::asm_port {

bool CLEAR();
bool NEW();
void LIST();
void RESTORE();
void STOP();
void ENDX();
void CONT();
void LOAD();
void SAVE();
void RUN();
void GOSUB();
void GOTO();
void POP();
void DATA();
void GET();
void LET();
void IF();
void REM();
void ONGOTO();

// ---------------------------------------------------------------------------
// Stub implementations for statement handlers not yet ported.
// Each will be replaced when its label range is converted.
// ---------------------------------------------------------------------------

void FOR();
static void NEXT()             {} // TODO(asm-port): NEXT
static void INPUT()            {} // TODO(asm-port): INPUT
static void DEL()              {} // TODO(asm-port): DEL
static void DIM()              {} // TODO(asm-port): DIM
static void READ()             {} // TODO(asm-port): READ
static void GR()               {} // TODO(asm-port): GR
static void TEXT()             {} // TODO(asm-port): TEXT
static void PR_NUMBER()        {} // TODO(asm-port): PR_NUMBER
static void IN_NUMBER()        {} // TODO(asm-port): IN_NUMBER
static void CALL()             {} // TODO(asm-port): CALL
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
static void TRACE()            {} // TODO(asm-port): TRACE
static void NOTRACE()          {} // TODO(asm-port): NOTRACE
static void NORMAL()           {} // TODO(asm-port): NORMAL
static void INVERSE()          {} // TODO(asm-port): INVERSE
static void FLASH()            {} // TODO(asm-port): FLASH
static void COLOR()            {} // TODO(asm-port): COLOR
static void VTAB()             {} // TODO(asm-port): VTAB
static void HIMEM()            {} // TODO(asm-port): HIMEM
static void LOMEM()            {} // TODO(asm-port): LOMEM
static void ONERR()            {} // TODO(asm-port): ONERR
static void RESUME()           {} // TODO(asm-port): RESUME
static void RECALL()           {} // TODO(asm-port): RECALL
static void STORE()            {} // TODO(asm-port): STORE
static void SPEED()            {} // TODO(asm-port): SPEED
static void AMPERSAND_VECTOR() {} // TODO(asm-port): & dispatch (JMP vector at $03F5)
static void WAIT()             {} // TODO(asm-port): WAIT
static void DEF()              {} // TODO(asm-port): DEF
static void POKE()             {} // TODO(asm-port): POKE
static void PRINT()            {} // TODO(asm-port): PRINT
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
        NEXT,             // [2]  $82...130...NEXT
        DATA,             // [3]  $83...131...DWTA
        INPUT,            // [4]  $84...132...INPUT
        DEL,              // [5]  $85...133...DEL
        DIM,              // [6]  $86...134...DIM
        READ,             // [7]  $87...135...READ
        GR,               // [8]  $88...136...GR
        TEXT,             // [9]  $89...137...TEXT
        PR_NUMBER,        // [10] $8A...138...PR#
        IN_NUMBER,        // [11] $8B...139...IN#
        CALL,             // [12] $8C...140...CALL
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
        NORMAL,           // [29] $9D...157...NORMAL
        INVERSE,          // [30] $9E...158...INVERSE
        FLASH,            // [31] $9F...159...FLASH
        COLOR,            // [32] $A0...160...COLOR=
        POP,              // [33] $A1...161...POP
        VTAB,             // [34] $A2...162...VTAB
        HIMEM,            // [35] $A3...163...HIMEM:
        LOMEM,            // [36] $A4...164...LOMEM:
        ONERR,            // [37] $A5...165...ONERR
        RESUME,           // [38] $A6...166...RESUME
        RECALL,           // [39] $A7...167...RECALL
        STORE,            // [40] $A8...168...STORE
        SPEED,            // [41] $A9...169...SPEED=
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
        WAIT,             // [53] $B5...181...WAIT
        LOAD,             // [54] $B6...182...LOAD
        SAVE,             // [55] $B7...183...SAVE
        DEF,              // [56] $B8...184...DEF
        POKE,             // [57] $B9...185...POKE
        PRINT,            // [58] $BA...186...PRINT
        CONT,             // [59] $BB...187...CONT
        LIST,             // [60] $BC...188...LIST
        CLEAR_Handler,    // [61] $BD...189...CLEAR
        GET_Handler,      // [62] $BE...190...GET
        NEW_Handler,      // [63] $BF...191...NEW
    };
    return table[index];
}

} // namespace applesoft::asm_port
