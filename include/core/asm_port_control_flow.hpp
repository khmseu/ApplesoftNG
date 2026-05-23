#pragma once

#include <cstdint>

namespace applesoft::asm_port {

void AS_FCOMP2();
void AS_SETFOR();
bool AS_ISCNTC();
void AS_STOP();
void AS_STOP_impl(bool shouldPrintBreak);
void AS_ENDX();
void AS_ENDX_impl(bool shouldPrintBreak);
void AS_END2_impl(bool shouldPrintBreak);
void AS_END4_impl(bool shouldPrintBreak);
void AS_RESTART();
void AS_CONTROL_C_TYPED();
void AS_CONT();
void AS_GOSUB();
void AS_GO_TO_LINE();
void AS_GOTO();
void AS_RESUME();
void AS_JSYN();
void AS_ONERR();
void AS_RTS_5();
void AS_PULL3();
std::uint8_t AS_REMN();
void AS_FOR();
void AS_NEXT();
void AS_POP();
void AS_RETURN();
void AS_STEP();
void AS_NEWSTT();
void AS_TRACE_();
void AS_GOEND();
void AS_EXECUTE_STATEMENT();
void AS_EXECUTE_STATEMENT_1();
void AS_COLON_();
void AS_IF();
void AS_REM();
void AS_IF_TRUE();
void AS_ONGOTO();

bool IsRunningMode();
bool IsTraceEnabled();
bool IsEndOfAS_LineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadAS_LineNumberFromTextPointer();
void AdvanceTextPointerToNextAS_Line();

} // namespace applesoft::asm_port
