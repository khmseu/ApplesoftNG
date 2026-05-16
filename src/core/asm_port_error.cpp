#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_mathtbl.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_reason.hpp"
#include "core/asm_port_stack.hpp"
#include "core/asm_port_token_address_table.hpp"
#include "core/asm_port_token_name_table.hpp"
#include "core/io_ports.hpp"
#include "platform/asm_port_outdo.hpp"

#include <array>
#include <optional>
#include <string_view>

namespace applesoft::asm_port {

constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
  return static_cast<std::uint8_t>(lhs + rhs);
}
void AS_SCRTCH();
void AS_RESTART();
void AS_CRDO();
void AS_FRMNUM();
void AS_GETADR();
std::uint8_t AS_MEMERR();
void AS_CLEARC();
std::uint8_t AS_GETBYT();
void AS_IQERR();

void MON_SETCOL(std::uint8_t color);
void MON_TABV(std::uint8_t row_zero_based);
void MON_INPORT(std::uint8_t slot);
void MON_OUTPORT(std::uint8_t slot);
void MON_PLOT(std::uint8_t y, std::uint8_t x);
void MON_HLINE(std::uint8_t y, std::uint8_t right, std::uint8_t left);
void MON_VLINE(std::uint8_t x, std::uint8_t top);
void MON_HOME();
void MON_SETTXT();
void MON_SETGR();
void AS_NORMAL();
void AS_INVERSE();
void AS_FLASH();
void HOME();
void AS_GR();
void AS_TEXT();
void AS_HTAB();
void AS_HCOLOR();
void AS_HPLOT();
void AS_DRAW();
void AS_XDRAW();
void AS_COLOR();
void AS_VTAB();
void AS_HIMEM();
void AS_LOMEM();
void AS_SPEED();
std::uint8_t AS_PLOTFNS();
std::uint8_t AS_LINCOOR();

void AS_CONTROL_C_TYPED();
bool AS_SETPTRS();
void AS_COLD_START();

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);

void ApplyFacSign();
void SetBranchTargetToAS_STEP();
void AS_LOAD_FAC_FROM_YA();
void AS_SYNCHR(std::uint8_t expected);
void AS_CHKNUM();
void AS_CHKSTR();
void AS_FRMNUM();
void AS_SIGN();
void AS_FRM_STACK_2();
void AS_FRM_STACK_3();
void MON_WRITE();
void MON_READ();
void AS_VARTIO();
void AS_PROGIO();
std::uint8_t CurrentStatementChar();
void AS_ADDON(std::uint8_t offset);
std::uint8_t AS_DATAN();
void AS_GOEND();
bool IsEndOfAS_LineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadAS_LineNumberFromTextPointer();
void AdvanceTextPointerToNextAS_Line();
void AS_DATA();
bool IsRunningMode();
bool IsTraceEnabled();
bool IsStatementEndOfParsedInput();
std::uint8_t AS_REMN();
bool AS_FL1(std::uint16_t startAddress);
bool AS_FL1(std::uint8_t startAS_Lo, std::uint8_t startHi);
std::uint8_t PeekTopControlTokenAfterAS_GTFORPNT();
void AS_PULL3();
void AS_RTS_5();
void AS_OUTSP();
void AS_EXECUTE_STATEMENT();
void AS_EXECUTE_STATEMENT_1();
void PushForPntFrame();

std::uint8_t AS_CHRGOT();
void AS_LINGET();
void AS_SYNERR();
void AS_LINPRT();
void AS_INPRT();
void PrintDecimalUnsigned(std::uint16_t value);
// void AS_OUTDO();
std::uint8_t AS_MEMERR();

void AS_LET();
void AS_LET2(std::uint8_t savedValTypPlus1);
void AS_PUTSTR();
void AS_IF();
void AS_REM();
void AS_IF_TRUE();
void AS_ONGOTO();
void AS_FOR();
void AS_NEXT();
void AS_STEP();
void AS_NEWSTT();
void AS_TRACE_();
void AS_FRMEVL();
std::uint8_t AS_GETBYT();
std::uint8_t AS_GTNUM();
std::uint8_t AS_COMBYTE();
void AS_GETADR();
void AS_GO_TO_LINE();
void AS_GOTO();
void AS_PEEK();
void AS_POKE();
void AS_WAIT();
void AS_RTS_10();
// std::uint16_t AS_PTRGET();
bool AS_CHKVAL(std::uint8_t savedValTyp);
void AS_ERRDIR();
void AS_ROUND_FAC();
void AS_AYINT();
void AS_SETFOR();
void AS_STRINI(std::uint8_t length);
void AS_MOVINS();
bool AS_FRETMS(std::uint16_t descriptorAddress);
std::uint8_t AS_FRETMP(std::uint16_t descriptorAddress);
void AS_GARBAG();
void AS_SCREEN();
void AS_UNARY();
void AS_RELOPS();
void AS_STRCMP();
void AS_NUMCMP();
void AS_CMPDONE();
void AS_CHKCOM();
void AS_CHKOPN();
void AS_CHKCLS();
void AS_PDL();
void AS_PTRGET3();
void AS_BADNAM();
void AS_NAMOK();
void AS_BASIC();
void AS_BASIC2();
void AS_FNC_();
void AS_PARCHK();
void AS_STORE_FACDB_YX_ROUNDED();
void AS_MAKINT();
void AS_MKINT();
void AS_MI1();
void AS_MI2();
void AS_SUBERR();
void AS_IQERR();
void AS_JER();
void AS_USE_OLD_ARRAY();
void AS_MAKE_NEW_ARRAY();
void AS_FIND_ARRAY_ELEMENT();
void AS_FAE_1();
void AS_GSE();
void AS_GME();
void AS_SYNERR();
void AS_STOP_impl(bool shouldPrintBreak);
void AS_GIVAYF(std::int16_t value);

void AS_PRINT_ERROR_LINNUM();
void AS_PRINT_ERROR_LINNUM(std::string_view prefix);

std::int8_t gNumericCompareResult = 0;
bool gNumericCompareCarry = false;
std::uint8_t gFloatInput = 0;

std::uint8_t gPendingErrorCode = AS_ERR_SYNTAX;

std::uint8_t gJerErrorCode = AS_ERR_SYNTAX;

// void AS_OUTDO() {
//     // TODO(asm-port): write the current output character from the Applesoft
//     line
//     // printer to the console.
// }

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_PEEK (inclusive) .. AS_POKE (exclusive)
// Name normalization: none (assembler label AS_PEEK kept verbatim).
} // namespace applesoft::asm_port
