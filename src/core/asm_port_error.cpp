#include "core/asm_port_error.hpp"
#include "core/asm_port_print.hpp"
#include "platform/asm_port_outdo.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_reason.hpp"
#include "core/io_ports.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_token_address_table.hpp"
#include "core/asm_port_token_name_table.hpp"
#include "core/asm_port_mathtbl.hpp"

#include <array>
#include <optional>
#include <string_view>

namespace applesoft::asm_port {

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
    return static_cast<std::uint8_t>(lhs + rhs);
}
void SetStackPointer(std::uint8_t value);
void PushByteToStack(std::uint8_t value);

void SCRTCH();
void RESTART();
void CRDO();
void FRMNUM();
void GETADR();
std::uint8_t MEMERR();
void CLEARC();
std::uint8_t GETBYT();
void IQERR();

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
void NORMAL();
void INVERSE();
void FLASH();
void HOME();
void GR();
void TEXT();
void HTAB();
void HCOLOR();
void HPLOT();
void DRAW();
void XDRAW();
void COLOR();
void VTAB();
void HIMEM();
void LOMEM();
void SPEED();
std::uint8_t PLOTFNS();
std::uint8_t LINCOOR();


void CONTROL_C_TYPED();
bool SETPTRS();
void COLD_START();

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);

std::uint8_t ReadStackPointer();
void PopReturnAddress();
void PushByteToStack(std::uint8_t value);
void PushTextPointerAddress();
void PushCurrentLineNumber();
void PushTokenTo(std::uint8_t token);
void ApplyFacSign();
void SetBranchTargetToSTEP();
void LOAD_FAC_FROM_YA();
void SYNCHR(std::uint8_t expected);
void CHKNUM();
void CHKSTR();
void FRMNUM();
void SIGN();
void FRM_STACK_2();
void FRM_STACK_3();
void MON_WRITE();
void MON_READ();
void VARTIO();
void PROGIO();
std::uint8_t CurrentStatementChar();
void ADDON(std::uint8_t offset);
std::uint8_t DATAN();
void GOEND();
bool IsEndOfLineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadLineNumberFromTextPointer();
void AdvanceTextPointerToNextLine();
void DATA();
bool IsRunningMode();
bool IsTraceEnabled();
bool IsStatementEndOfParsedInput();
std::uint8_t REMN();
bool FL1(std::uint16_t startAddress);
bool FL1(std::uint8_t startLo, std::uint8_t startHi);
std::uint8_t PopByteFromStack();
std::uint8_t PeekTopControlTokenAfterGTFORPNT();
void PULL3();
void RTS_5();
void OUTSP();
void EXECUTE_STATEMENT();
void EXECUTE_STATEMENT_1();
void PushForPntFrame();

std::uint8_t CHRGOT();
void LINGET();
void SYNERR();
void LINPRT();
void INPRT();
void PrintDecimalUnsigned(std::uint16_t value);
// void OUTDO();
std::uint8_t MEMERR();

void LET();
void LET2(std::uint8_t savedValTypPlus1);
void PUTSTR();
void IF();
void REM();
void IF_TRUE();
void ONGOTO();
void FOR();
void NEXT();
void STEP();
void NEWSTT();
void TRACE_();
void FRMEVL();
std::uint8_t GETBYT();
std::uint8_t GTNUM();
std::uint8_t COMBYTE();
void GETADR();
void GO_TO_LINE();
void GOTO();
void PEEK();
void POKE();
void WAIT();
void RTS_10();
// std::uint16_t PTRGET();
bool CHKVAL(std::uint8_t savedValTyp);
void ERRDIR();
void ROUND_FAC();
void AYINT();
void SETFOR();
void STRINI(std::uint8_t length);
void MOVINS();
bool FRETMS(std::uint16_t descriptorAddress);
std::uint8_t FRETMP(std::uint16_t descriptorAddress);
void GARBAG();
void SCREEN();
void UNARY();
void RELOPS();
void STRCMP();
void NUMCMP();
void CMPDONE();
void CHKCOM();
void CHKOPN();
void CHKCLS();
void PDL();
void PTRGET3();
void BADNAM();
void NAMOK();
void BASIC();
void BASIC2();
void FNC_();
void PARCHK();
void STORE_FACDB_YX_ROUNDED();
void MAKINT();
void MKINT();
void MI1();
void MI2();
void SUBERR();
void IQERR();
void JER();
void USE_OLD_ARRAY();
void MAKE_NEW_ARRAY();
void FIND_ARRAY_ELEMENT();
void FAE_1();
void GSE();
void GME();
void SYNERR();
void STOP_impl(bool shouldPrintBreak);
void GIVAYF(std::int16_t value);

void PRINT_ERROR_LINNUM();
void PRINT_ERROR_LINNUM(std::string_view prefix);


std::int8_t gNumericCompareResult = 0;
bool gNumericCompareCarry = false;
std::uint8_t gFloatInput = 0;

std::uint8_t gPendingErrorCode = ERR_SYNTAX;

std::uint8_t gJerErrorCode = ERR_SYNTAX;


// void OUTDO() {
//     // TODO(asm-port): write the current output character from the Applesoft line
//     // printer to the console.
// }

void PopReturnAddress();
void PushByteToStack(std::uint8_t value);
void PushWordToStack(std::uint16_t value);
std::uint16_t PopWordFromStack();
void PushTextPointerAddress();
void PushCurrentLineNumber();
void PushTokenTo(std::uint8_t token);


// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PEEK (inclusive) .. POKE (exclusive)
// Name normalization: none (assembler label PEEK kept verbatim).
} // namespace applesoft::asm_port
