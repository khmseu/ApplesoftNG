// Source:
// SourceMaterial/Combo/asrom.lst
// Name normalization: none

#include "core/asm_port_token_address_table.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/jump_table.hpp"

namespace applesoft::asm_port {

std::uint8_t AS_CHRGET();

void AS_CLEAR();
bool AS_NEW();
void AS_LIST();
void AS_RESTORE();
void AS_STOP();
void AS_ENDX();
void AS_NEXT();
void AS_CONT();
void AS_LOAD();
void AS_SAVE();
void AS_RUN();
void AS_GOSUB();
void AS_GOTO();
void AS_POP();
void AS_DATA();
void AS_GET();
void AS_INPUT();
void AS_READ();
void AS_DIM();
void AS_DEL();
void AS_LET();
void AS_PR_NUMBER();
void AS_IN_NUMBER();
void AS_PLOT();
void AS_HLIN();
void AS_VLIN();
void AS_HCOLOR();
void AS_HPLOT();
void AS_DRAW();
void AS_XDRAW();
void MON_HOME();
void AS_SHLOAD();
void AS_ROT();
void AS_SCALE();
void AS_HGR2();
void AS_HGR();
void AS_IF();
void AS_REM();
void AS_ONGOTO();
void AS_ONERR();
void AS_RESUME();
void AS_RECALL();
void AS_STORE();
void AS_DEF();
void AS_POKE();
void AS_WAIT();
void AS_PRINT(std::uint8_t a);
void AS_CALL();
void AS_NORMAL();
void AS_INVERSE();
void AS_FLASH();
void AS_COLOR();
void AS_VTAB();
void AS_HIMEM();
void AS_LOMEM();
void AS_SPEED();
void AS_GR();
void AS_TEXT();
void AS_HTAB();

// ---------------------------------------------------------------------------
// Stub implementations for statement handlers not yet ported.
// Each will be replaced when its label range is converted.
// ---------------------------------------------------------------------------

void AS_FOR();
static void AS_NEXT_Handler() { AS_NEXT(); }
static void AS_INPUT_Handler() { AS_INPUT(); }
static void AS_DEL_Handler() { AS_DEL(); }
static void AS_DIM_Handler() { AS_DIM(); }
static void AS_READ_Handler() { AS_READ(); }
static void AS_GR_Handler() { AS_GR(); }
static void AS_TEXT_Handler() { AS_TEXT(); }
static void AS_PR_NUMBER_Handler() { AS_PR_NUMBER(); }
static void AS_IN_NUMBER_Handler() { AS_IN_NUMBER(); }
static void AS_CALL_Handler() { AS_CALL(); }
static void AS_PLOT_Handler() { AS_PLOT(); }
static void AS_HLIN_Handler() { AS_HLIN(); }
static void AS_VLIN_Handler() { AS_VLIN(); }
static void AS_HGR2_Handler() { AS_HGR2(); }
static void AS_HGR_Handler() { AS_HGR(); }
static void AS_HCOLOR_Handler() { AS_HCOLOR(); }
static void AS_HPLOT_Handler() { AS_HPLOT(); }
static void AS_DRAW_Handler() { AS_DRAW(); }
static void AS_XDRAW_Handler() { AS_XDRAW(); }
static void AS_HTAB_Handler() { AS_HTAB(); }
static void HOME_Handler() { MON_HOME(); }
static void AS_ROT_Handler() { AS_ROT(); }
static void AS_SCALE_Handler() { AS_SCALE(); }
static void AS_SHLOAD_Handler() { AS_SHLOAD(); }
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_TRACE (inclusive) .. AS_NORMAL (exclusive)
// Name normalization: none. AS_TRACE: sec; ror AS_TRCFLG → bit 7 set (trace
// on).
//                           AS_NOTRACE: clc; ror AS_TRCFLG → bit 7 clear (trace
//                           off).
static void AS_TRACE() {
  const std::uint8_t trcflg = variables_const().AS_TRCFLG;
  variables().AS_TRCFLG = static_cast<std::uint8_t>((trcflg >> 1u) | 0x80u);
}
static void AS_NOTRACE() {
  const std::uint8_t trcflg = variables_const().AS_TRCFLG;
  variables().AS_TRCFLG = static_cast<std::uint8_t>(trcflg >> 1u);
}
static void AS_NORMAL_Handler() { AS_NORMAL(); }
static void AS_INVERSE_Handler() { AS_INVERSE(); }
static void AS_FLASH_Handler() { AS_FLASH(); }
static void AS_COLOR_Handler() { AS_COLOR(); }
static void AS_VTAB_Handler() { AS_VTAB(); }
static void AS_HIMEM_Handler() { AS_HIMEM(); }
static void AS_LOMEM_Handler() { AS_LOMEM(); }
static void AS_ONERR_Handler() { AS_ONERR(); }
static void AS_RESUME_Handler() { AS_RESUME(); }
static void AS_RECALL_Handler() { AS_RECALL(); }
static void AS_STORE_Handler() { AS_STORE(); }
static void AS_SPEED_Handler() { AS_SPEED(); }
static void AS_AMPERSAND_VECTOR() {
  // '&' dispatch uses a machine-language vector at $03F5 in the ROM model.
  // No machine-code bridge exists in this runtime, so treat it as unsupported.
  ApplesoftNG::ExternalJumpDispatcher::JumpFromInstruction(
      ApplesoftNG::ExternalJumpDispatcher::ADDR_AS_AMPERSAND);
}
void AS_AMPERSAND() {
  // '&' dispatch uses a machine-language vector at $03F5 in the ROM model.
  // No machine-code bridge exists in this runtime, so treat it as unsupported.
  AS_ERROR(AS_ERR_UNDEFSTAT);
}
static void AS_WAIT_Handler() { AS_WAIT(); }
static void AS_DEF_Handler() { AS_DEF(); }
static void AS_POKE_Handler() { AS_POKE(); }
static void AS_PRINT_Handler() { AS_PRINT(AS_CHRGET()); }
static void AS_CLEAR_Handler() { AS_CLEAR(); }
static void AS_GET_Handler() { AS_GET(); }
static void AS_NEW_Handler() { AS_NEW(); }

// ---------------------------------------------------------------------------
// Branch table for Applesoft statement tokens $80–$BF.
// Index = token - $80.
// The -1 RTS-dispatch artifact from the 6502 source is dropped; plain
// function pointers are used and the caller is responsible for invoking them.
// ---------------------------------------------------------------------------
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_TOKEN_ADDRESS_TABLE (inclusive) .. AS_UNFNC (exclusive)
// Name normalization: none
AS_TOKEN_ADDRESS_TABLE_fn AS_TOKEN_ADDRESS_TABLE(std::size_t index) {
  static constexpr AS_TOKEN_ADDRESS_TABLE_fn table[] = {
      AS_ENDX,              // [0]  $80...128...END
      AS_FOR,               // [1]  $81...129...AS_FOR
      AS_NEXT_Handler,      // [2]  $82...130...AS_NEXT
      AS_DATA,              // [3]  $83...131...DWTA
      AS_INPUT_Handler,     // [4]  $84...132...AS_INPUT
      AS_DEL_Handler,       // [5]  $85...133...AS_DEL
      AS_DIM_Handler,       // [6]  $86...134...AS_DIM
      AS_READ_Handler,      // [7]  $87...135...AS_READ
      AS_GR_Handler,        // [8]  $88...136...AS_GR
      AS_TEXT_Handler,      // [9]  $89...137...AS_TEXT
      AS_PR_NUMBER_Handler, // [10] $8A...138...PR#
      AS_IN_NUMBER_Handler, // [11] $8B...139...IN#
      AS_CALL_Handler,      // [12] $8C...140...AS_CALL
      AS_PLOT_Handler,      // [13] $8D...141...AS_PLOT
      AS_HLIN_Handler,      // [14] $8E...142...AS_HLIN
      AS_VLIN_Handler,      // [15] $8F...143...AS_VLIN
      AS_HGR2_Handler,      // [16] $90...144...AS_HGR2
      AS_HGR_Handler,       // [17] $91...145...AS_HGR
      AS_HCOLOR_Handler,    // [18] $92...146...AS_HCOLOR=
      AS_HPLOT_Handler,     // [19] $93...147...AS_HPLOT
      AS_DRAW_Handler,      // [20] $94...148...AS_DRAW
      AS_XDRAW_Handler,     // [21] $95...149...AS_XDRAW
      AS_HTAB_Handler,      // [22] $96...150...AS_HTAB
      HOME_Handler,         // [23] $97...151...HOME
      AS_ROT_Handler,       // [24] $98...152...AS_ROT=
      AS_SCALE_Handler,     // [25] $99...153...AS_SCALE=
      AS_SHLOAD_Handler,    // [26] $9A...154...AS_SHLOAD
      AS_TRACE,             // [27] $9B...155...AS_TRACE
      AS_NOTRACE,           // [28] $9C...156...AS_NOTRACE
      AS_NORMAL_Handler,    // [29] $9D...157...AS_NORMAL
      AS_INVERSE_Handler,   // [30] $9E...158...AS_INVERSE
      AS_FLASH_Handler,     // [31] $9F...159...AS_FLASH
      AS_COLOR_Handler,     // [32] $A0...160...AS_COLOR=
      AS_POP,               // [33] $A1...161...AS_POP
      AS_VTAB_Handler,      // [34] $A2...162...AS_VTAB
      AS_HIMEM_Handler,     // [35] $A3...163...AS_HIMEM:
      AS_LOMEM_Handler,     // [36] $A4...164...AS_LOMEM:
      AS_ONERR_Handler,     // [37] $A5...165...AS_ONERR
      AS_RESUME_Handler,    // [38] $A6...166...AS_RESUME
      AS_RECALL_Handler,    // [39] $A7...167...AS_RECALL
      AS_STORE_Handler,     // [40] $A8...168...AS_STORE
      AS_SPEED_Handler,     // [41] $A9...169...AS_SPEED=
      AS_LET,               // [42] $AA...170...AS_LET
      AS_GOTO,              // [43] $AB...171...AS_GOTO
      AS_RUN,               // [44] $AC...172...AS_RUN
      AS_IF,                // [45] $AD...173...AS_IF
      AS_RESTORE,           // [46] $AE...174...AS_RESTORE
      AS_AMPERSAND_VECTOR,  // [47] $AF...175...&
      AS_GOSUB,             // [48] $B0...176...AS_GOSUB
      AS_POP,           // [49] $B1...177...AS_RETURN (same handler as AS_POP)
      AS_REM,           // [50] $B2...178...AS_REM
      AS_STOP,          // [51] $B3...179...AS_STOP
      AS_ONGOTO,        // [52] $B4...180...ON
      AS_WAIT_Handler,  // [53] $B5...181...AS_WAIT
      AS_LOAD,          // [54] $B6...182...AS_LOAD
      AS_SAVE,          // [55] $B7...183...AS_SAVE
      AS_DEF_Handler,   // [56] $B8...184...AS_DEF
      AS_POKE_Handler,  // [57] $B9...185...AS_POKE
      AS_PRINT_Handler, // [58] $BA...186...AS_PRINT
      AS_CONT,          // [59] $BB...187...AS_CONT
      AS_LIST,          // [60] $BC...188...AS_LIST
      AS_CLEAR_Handler, // [61] $BD...189...AS_CLEAR
      AS_GET_Handler,   // [62] $BE...190...AS_GET
      AS_NEW_Handler,   // [63] $BF...191...AS_NEW
  };
  return table[index];
}

} // namespace applesoft::asm_port
