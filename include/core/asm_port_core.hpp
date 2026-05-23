#pragma once

#include <cstdint>

namespace applesoft::asm_port {

void AS_GENERIC_END();
void MON_BELL();
std::int8_t MON_INSDS1();
void MON_MON();
void MON_RESET2();
void MON_OLDBRK();
void MON_REGDSP();

void AS_COLD_START();
void AS_ERRDIR();
void AS_UNDFNC();
void AS_BADNAM();
void AS_BASIC();
void AS_BASIC2();
void AS_GME();
void AS_SUBERR();
void AS_GSE();
void AS_FAE_1();
void AS_GETARY();
void AS_GETARY2();
void AS_C_ZERO();
void AS_USE_OLD_ARRAY();
void AS_MAKE_NEW_ARRAY();
void AS_FIND_ARRAY_ELEMENT();
std::uint16_t AS_MULTIPLY_SUBSCRIPT(std::uint8_t descriptorOffset);
std::uint16_t AS_MULTIPLY_SUBS_1(std::uint8_t multiplierHigh);
void AS_SNGFLT(std::uint8_t value);
void AS_OR();
void AS_ANDOP();
void AS_FALSE();
void AS_TRUE();
void AS_SET_VARPNT_AND_YA();
void AS_MAKE_NEW_VARIABLE();
void AS_NAME_NOT_FOUND();
void AS_PTRGET3();
void AS_PTRGET2();
std::uint16_t AS_PTRGET();
void AS_ARRAY();
void AS_DIM();
void AS_NXDIM();
void AS_PTRGET4();
void AS_NUMCMP();
void AS_CMPDONE();
void AS_AYINT();
void AS_HANDLERR();
void AS_SCREEN();
void AS_UNARY();
void AS_RELOPS();
void AS_STRCMP();
void AS_FRE();
void AS_GIVAYF(std::int16_t value);
void AS_FNC_();
void AS_DEF();
void AS_FUNCT();
void AS_FNCDATA();
void AS_FRMEVL();
void AS_SNTXERR();
bool AS_NOTMATH(std::uint8_t token);
std::int8_t AS_FCOMP(std::uint16_t argAddress);

std::uint8_t MON_SCRN(std::uint8_t row, std::uint8_t column);
void MON_IRQ();
std::uint8_t MON_PREAD();

void SetTextPointer(std::uint16_t address);
void ClearErrFlag();
void MarkDirectMode();
void SetPendingErrorCode(std::uint8_t errorCode);
std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);
bool IsOnErr();
bool IsDirectMode();

} // namespace applesoft::asm_port
