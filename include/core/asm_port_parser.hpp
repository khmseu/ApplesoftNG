#pragma once

#include <cstdint>

namespace applesoft::asm_port {

void AS_SYNCHR(std::uint8_t expected);
void AS_CHKNUM();
void AS_CHKSTR();
void AS_CHKCOM();
void AS_FRMNUM();
void AS_PARCHK();
void AS_STORE_FACDB_YX_ROUNDED();
void AS_CHKCLS();
void AS_CHKOPN();
void AS_LINGET();
void AS_GTBYTC();
std::uint8_t AS_GETBYT();
void AS_CONINT();
std::uint8_t AS_GTNUM();
std::uint8_t AS_COMBYTE();
void AS_QINT();
void AS_GETADR();
bool AS_CHKVAL(std::uint8_t savedValTyp);
void AS_NEG32768();
void AS_MAKINT();
void AS_MI1();
void AS_MI2();
bool AS_ISLETC();
void AS_MKINT();
void AS_NAMOK();

bool IsStatementEndOfParsedInput();
std::uint8_t CurrentStatementChar();

} // namespace applesoft::asm_port
