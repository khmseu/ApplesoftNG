#pragma once

#include "core/applesoft_dual_pointer.hpp"

#include <cstddef>
#include <cstdint>

namespace applesoft::asm_port {

void AS_NORMAL();
void AS_INVERSE();
void AS_FLASH();
void MON_SETCOL(std::uint8_t color);
void MON_TABV(std::uint8_t row_zero_based);
void MON_INPORT(std::uint8_t slot);
void MON_OUTPORT(std::uint8_t slot);
void MON_PLOT(std::uint8_t y, std::uint8_t x);
void MON_HLINE(std::uint8_t y, std::uint8_t right, std::uint8_t left);
void MON_VLINE(std::uint8_t x, std::uint8_t top);
void MON_CLRSCR();
std::uint16_t MON_GBASCALC(std::uint8_t y);
void MON1_NXTCOL();
void MON_HOME();
void MON_CLREOL();
void MON_SETTXT();
void MON_SETGR();
void AS_GR();
void AS_TEXT();
void AS_HTAB();
void AS_HGR2();
void AS_HGR();
void AS_HCLR();
void AS_BKGND();
void AS_L_BKGND_1();
void AS_HCOLOR();
void AS_ROT();
void AS_SCALE();
void AS_GGERR();
void AS_HPLOT();
void AS_MOVE_UP_OR_DOWN();
void AS_MOVE_DOWN();
void AS_MOVE_RIGHT();
void AS_MOVE_LEFT_OR_RIGHT();
void AS_LRUD4();
void AS_LRUD3_SETBIT();
void AS_LRUD3_XORBIT();
void AS_LRUD1();
void AS_LRUDX1();
void AS_DRAW1_Internal(bool xdraw);
ApplesoftDualPointer<const std::uint8_t> AS_COSINE_TABLE();
void AS_DRAW();
void AS_XDRAW();
void AS_SHLOAD();
void AS_COLOR();
void AS_VTAB();
void AS_HIMEM();
void AS_LOMEM();
void AS_SPEED();
std::uint8_t AS_PLOTFNS();
void AS_GOERR();
std::uint8_t AS_LINCOOR();
void AS_PLOT();
void AS_HLIN();
void AS_VLIN();

} // namespace applesoft::asm_port
