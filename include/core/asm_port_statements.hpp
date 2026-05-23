#pragma once

#include <cstdint>

namespace applesoft::asm_port {

bool AS_NEW_impl();
void AS_SCRTCH_impl();
bool AS_SETPTRS_impl();

bool AS_NEW();
bool AS_SETPTRS();
void AS_SCRTCH();
void AS_RUN();
void AS_PEEK();
void AS_POKE();
void AS_WAIT();
void AS_RTS_10();
void AS_ADDON(std::uint8_t offset);
std::uint8_t AS_DATAN();
void AS_DATA();
void AS_LET();
void AS_LET2(std::uint8_t savedValTypPlus1);
void AS_PUTSTR();
void AS_PARSE_INPUT_LINE();
void AS_FIX_LINKS();
bool AS_FNDLIN();
bool AS_FL1(std::uint16_t startAddress);
void AS_DEL();
void AS_LIST();
void AS_STORE();
void AS_L_STORE_1();
void AS_RECALL();
void AS_L_RECALL_1();
void AS_GETARYPT();
void AS_TAPEPNT();
void AS_SAVE();
void AS_LOAD();
void AS_VARTIO();
void AS_PROGIO();
void AS_CALL();
void AS_IN_NUMBER();
void AS_PR_NUMBER();

void MON_WRITE();
void MON_READ();
void MON_RD2();
void MON_RD2BIT();
void MON_HEADR(std::uint8_t delay_code);
void MON_RD3();
bool MON_RDBIT();
std::uint8_t MON_RDBYTE();
bool MON_NXTA1();
void MON_RESTORE();
void MON_PRERR();

void HandleNumberedAS_Line();

} // namespace applesoft::asm_port
