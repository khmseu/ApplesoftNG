#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/io_ports.hpp"

#include <cstdint>

namespace applesoft::asm_port {

void AS_CRDO();
void AS_FRMNUM();
void AS_GETADR();
std::uint8_t AS_MEMERR();
void AS_CLEARC();
std::uint8_t AS_GETBYT();
void AS_IQERR();
std::uint8_t AS_CHRGOT();
void AS_SYNCHR(std::uint8_t expected);

void AS_NORMAL() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_NORMAL (inclusive) .. AS_INVERSE (exclusive)
  // Name normalization: none (assembler label AS_NORMAL kept verbatim).

  variables().MON_INVFLG = 0xffu;
  variables().AS_FLASH_BIT = 0x00u;
}

void AS_INVERSE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_INVERSE (inclusive) .. AS_FLASH (exclusive)
  // Name normalization: none (assembler label AS_INVERSE kept verbatim).

  variables().MON_INVFLG = 0x3fu;
  variables().AS_FLASH_BIT = 0x00u;
}

void AS_FLASH() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FLASH (inclusive) .. AS_COLOR (exclusive)
  // Name normalization: none (assembler label AS_FLASH kept verbatim).

  variables().MON_INVFLG = 0x7fu;
  variables().AS_FLASH_BIT = 0x40u;
}

void MON_SETCOL(std::uint8_t color) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
  // AS_Labels: SETCOL (inclusive) .. SCRN (exclusive)
  // Name normalization: SETCOL -> MON_SETCOL (monitor label gets MON_ prefix).

  // Monitor stores a 4-bit color and mirrors it into both nibbles (17*A mod
  // 16).
  const std::uint8_t nibble = static_cast<std::uint8_t>(color & 0x0fu);
  const std::uint8_t packed =
      static_cast<std::uint8_t>((nibble << 4u) | nibble);
  variables().MON_COLOR = packed;
}

void MON_TABV(std::uint8_t row_zero_based) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
  // AS_Labels: TABV (inclusive) .. external AS_VTAB jump target (exclusive)
  // Name normalization: TABV -> MON_TABV (monitor label gets MON_ prefix).
  //
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
  // AS_Labels: AS_VTAB (inclusive) .. ESC1 (exclusive)
  // Name normalization: AS_VTAB logic inlined into MON_TABV.

  // display1 TABV: sta CV ; jmp AS_VTAB
  variables().MON_CV = row_zero_based;

  // display2 AS_VTAB/AS_VTABZ path:
  // BASCALC computes BASL/BASH for row in CV, then AS_VTAB adds WNDLFT to BASL.
  const std::uint8_t line = variables_const().MON_CV;
  const bool carryFromAS_Lsr = (line & 0x01u) != 0u;
  const std::uint8_t bash =
      static_cast<std::uint8_t>(((line >> 1u) & 0x03u) | 0x04u);

  std::uint8_t basl = static_cast<std::uint8_t>(line & 0x18u);
  if (carryFromAS_Lsr) {
    basl = static_cast<std::uint8_t>(basl + 0x80u);
  }
  const std::uint8_t baslBase = basl;
  basl = static_cast<std::uint8_t>((basl << 2u) | baslBase);
  basl = static_cast<std::uint8_t>(basl + variables_const().MON_WNDLFT);

  variables().MON_BASL = ApplesoftVariables::makeWord(basl, bash);
}

namespace {

void MON_IOPRT(std::uint8_t slot, std::uint8_t vectorBase,
               std::uint8_t defaultVectorAS_Low) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: IOPRT (inclusive) .. XBASIC (exclusive)
  // Name normalization: helper name chosen for shared I/O-vector setup body.

  constexpr std::uint8_t kSlotMask = 0x0fu;
  constexpr std::uint8_t kIoBaseHigh = 0xc0u; // >IOADR
  constexpr std::uint8_t kDefaultHigh =
      0xfdu; // >COUT1 in Apple II+ monitor bank

  std::uint8_t vectorAS_Low = defaultVectorAS_Low;
  std::uint8_t vectorHigh;

  const std::uint8_t slotNibble = static_cast<std::uint8_t>(slot & kSlotMask);
  if (slotNibble == 0u) {
    vectorHigh = kDefaultHigh;
  } else {
    vectorAS_Low = 0u;
    vectorHigh = static_cast<std::uint8_t>(slotNibble | kIoBaseHigh);
  }

  variables().writeByte(vectorBase, vectorAS_Low);
  variables().writeByte(static_cast<std::uint8_t>(vectorBase + 1u), vectorHigh);
}

} // namespace

void MON_INPORT(std::uint8_t slot) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: INPORT (inclusive) .. SETVID (exclusive)
  // Name normalization: INPORT -> MON_INPORT (monitor label gets MON_ prefix).

  constexpr std::uint8_t kMON_KSW = ApplesoftVariables::ZP_MON_KSW;
  constexpr std::uint8_t kKeyinAS_Low = 0x0fu; // <KEYIN from keyin.o65.sym

  ApplesoftVariables::setLowByte(variables().MON_A2, slot);
  MON_IOPRT(ApplesoftVariables::lowByte(variables_const().MON_A2), kMON_KSW,
            kKeyinAS_Low);
}

void MON_OUTPORT(std::uint8_t slot) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: OUTPORT (inclusive) .. IOPRT (exclusive)
  // Name normalization: OUTPORT -> MON_OUTPORT (monitor label gets MON_
  // prefix).

  constexpr std::uint8_t kMON_CSW = ApplesoftVariables::ZP_MON_CSW;
  constexpr std::uint8_t kCout1AS_Low = 0x62u; // <COUT1 from cmd.o65.sym

  ApplesoftVariables::setLowByte(variables().MON_A2, slot);
  MON_IOPRT(ApplesoftVariables::lowByte(variables_const().MON_A2), kMON_CSW,
            kCout1AS_Low);
}

void MON_PLOT(std::uint8_t y, std::uint8_t x) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
  // AS_Labels: AS_PLOT (inclusive) .. AS_HLINE (exclusive)
  // Name normalization: AS_PLOT -> MON_PLOT (monitor label gets MON_ prefix).
  //
  // Mirrors ROM AS_PLOT/AS_PLOT1 behavior: compute lo-res cell base from Y,
  // then replace only the selected nibble (even row: low, odd row: high).

  const std::uint8_t halfRow = static_cast<std::uint8_t>(y >> 1u);
  const bool oddRow = (y & 0x01u) != 0u;

  // Inline GBASCALC for page $0400-$07ff lo-res screen mapping.
  const std::uint8_t gbash =
      static_cast<std::uint8_t>(((halfRow >> 1u) & 0x03u) | 0x04u);
  std::uint8_t gbasl = static_cast<std::uint8_t>(halfRow & 0x18u);
  if (oddRow) {
    gbasl = static_cast<std::uint8_t>(gbasl + 0x80u);
  }
  const std::uint8_t gbaslBase = gbasl;
  gbasl = static_cast<std::uint8_t>((gbasl << 2u) | gbaslBase);

  const std::uint16_t baseAddress = ApplesoftVariables::makeWord(gbasl, gbash);
  const std::uint16_t screenAddress =
      static_cast<std::uint16_t>(baseAddress + x);

  const std::uint8_t color = variables_const().MON_COLOR;
  const std::uint8_t existing = variables_const().readByte(screenAddress);
  const std::uint8_t merged =
      oddRow ? static_cast<std::uint8_t>((existing & 0x0fu) | (color & 0xf0u))
             : static_cast<std::uint8_t>((existing & 0xf0u) | (color & 0x0fu));

  variables().writeByte(screenAddress, merged);
}

void MON_HLINE(std::uint8_t y, std::uint8_t right, std::uint8_t left) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
  // AS_Labels: AS_HLINE (inclusive) .. AS_VLINEZ (exclusive)
  // Name normalization: AS_HLINE -> MON_HLINE (monitor label gets MON_ prefix).
  //
  // Draws a horizontal run from X=left through X=right at row Y.

  std::uint8_t x = left;
  for (;;) {
    MON_PLOT(y, x);
    if (x >= right) {
      break;
    }
    x = static_cast<std::uint8_t>(x + 1u);
  }
}

void MON_VLINE(std::uint8_t x, std::uint8_t top) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
  // AS_Labels: AS_VLINEZ (inclusive) .. RTS1 (exclusive)
  // Name normalization: AS_VLINE -> MON_VLINE (monitor label gets MON_ prefix).
  //
  // Bottom endpoint is MON_V2 ($2d), as established by Applesoft AS_LINCOOR.

  const std::uint8_t bottom = variables_const().MON_V2;

  std::uint8_t y = top;
  for (;;) {
    MON_PLOT(y, x);
    if (y >= bottom) {
      break;
    }
    y = static_cast<std::uint8_t>(y + 1u);
  }
}

void MON_HOME() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
  // AS_Labels: HOME (inclusive) .. CR (exclusive)
  // Name normalization: HOME -> MON_HOME (monitor label gets MON_ prefix).
  //
  // Monitor HOME initializes cursor to top-left of current window and clears
  // the window to blanks (high-bit set AS_ASCII space).

  constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);

  const std::uint8_t windowTop = variables_const().MON_WNDTOP;
  const std::uint8_t windowBottom = variables_const().MON_WNDBTM;
  const std::uint8_t width = variables_const().MON_WNDWDTH;
  const std::uint8_t left = variables_const().MON_WNDLFT;

  variables().MON_CH = 0u;

  for (std::uint8_t row = windowTop; row < windowBottom; ++row) {
    MON_TABV(row);
    const std::uint16_t baseAddress = variables_const().MON_BASL;

    for (std::uint8_t col = 0u; col < width; ++col) {
      const std::uint16_t address =
          static_cast<std::uint16_t>(baseAddress + left + col);
      variables().writeByte(address, kBlank);
    }
  }

  MON_TABV(windowTop);
  variables().MON_CH = 0u;
}

void MON_CLREOL() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
  // AS_Labels: CLREOL (inclusive) .. RTS (exclusive)
  // Name normalization: CLREOL -> MON_CLREOL (monitor label gets MON_ prefix).
  //
  // Clears from current cursor position to end of line with high-bit-set
  // spaces.

  constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);

  const std::uint16_t baseAddress = variables_const().MON_BASL;
  const std::uint8_t startCol = variables_const().MON_CH;
  const std::uint8_t windowWidth = variables_const().MON_WNDWDTH;

  for (std::uint8_t col = startCol; col < windowWidth; ++col) {
    const std::uint16_t address = static_cast<std::uint16_t>(baseAddress + col);
    variables().writeByte(address, kBlank);
  }
}

void MON_SETTXT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
  // AS_Labels: SETTXT (inclusive) .. SETGR (exclusive)
  // Name normalization: SETTXT -> MON_SETTXT (monitor label gets MON_ prefix).
  //
  // ROM fall-through: SETTXT unconditionally branches to SETWND; modeled
  // directly here by writing the window fields and tabbing to row 23.

  (void)variables_const().readByte(IOPorts::ADDR_SW_TXTSET);

  variables().MON_WNDTOP = 0u;
  variables().MON_WNDLFT = 0u;
  variables().MON_WNDWDTH = 40u;
  variables().MON_WNDBTM = 24u;

  // SETWND tail sets A=23 and jumps to AS_VTAB.
  MON_TABV(23u);
}

void MON_SETGR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
  // AS_Labels: SETGR (inclusive) .. SETWND (exclusive)
  // Name normalization: SETGR -> MON_SETGR (monitor label gets MON_ prefix).
  //
  // Falls through into SETWND in ROM; modeled here by writing the window
  // fields and tabbing to row 23.

  (void)variables_const().readByte(IOPorts::ADDR_AS_SW_TXTCLR);
  (void)variables_const().readByte(IOPorts::ADDR_AS_SW_MIXSET);

  // CLRTOP call target is still pending; window state is applied directly here.

  variables().MON_WNDTOP = 20u;
  variables().MON_WNDLFT = 0u;
  variables().MON_WNDWDTH = 40u;
  variables().MON_WNDBTM = 24u;

  MON_TABV(23u);
}

void HOME() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: MON_HOME (inclusive) .. AS_ROT (exclusive)
  // Name normalization: HOME statement maps to MON_HOME monitor routine
  // ($FC58).

  MON_HOME();
}

void AS_GR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GR (inclusive) .. AS_TEXT (exclusive)
  // Name normalization: none (assembler label AS_GR kept verbatim).
  //
  // lda AS_SW_LORES ($c056): soft-switch read activates lo-res graphics mode.
  // lda AS_SW_MIXSET ($c053): soft-switch read enables lower 4 lines as text.
  // jmp MON_SETGR: monitor SETGR sets up the lo-res graphics window.
  // Soft-switch side-effect reads carry no value; mode is established by
  // MON_SETGR.
  MON_SETGR();
}

void AS_TEXT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_TEXT (inclusive) .. AS_STORE (exclusive)
  // Name normalization: none (assembler label AS_TEXT kept verbatim).
  //
  // lda AS_SW_LOWSCR ($c054): soft-switch read selects display page 1.
  // jmp MON_SETTXT: monitor SETTXT sets the full-screen text window.
  // Soft-switch side-effect read carries no value; state is set by MON_SETTXT.
  MON_SETTXT();
}

void AS_HTAB() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HTAB (inclusive) .. end of label range (exclusive)
  // Name normalization: none (assembler label AS_HTAB kept verbatim).
  //
  // jsr AS_GETBYT  — evaluate expression; result in X-reg (1-based column).
  // dex         — convert to 0-based.
  // AS_L_HTAB_1: if col >= 40, subtract 40 and emit CR (handles columns >
  // screen width). AS_L_HTAB_2: sta MON_CH ($24) — store final column into
  // cursor position register.
  const std::uint8_t raw = AS_GETBYT();
  std::uint8_t col = static_cast<std::uint8_t>(raw - 1u); // dex; txa
  while (col >= 40u) {
    col = static_cast<std::uint8_t>(col - 40u);
    AS_CRDO();
  }
  variables().MON_CH = col;
}

void AS_HCOLOR();
void STHPG();

void AS_HGR2() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HGR2 (inclusive) .. AS_SETHPG (exclusive)
  // Name normalization: none (assembler label AS_HGR2 kept verbatim).

  // AS_HGR2 bit AS_SW_HISCR ; bit AS_SW_MIXCLR ; lda #$40 ; bne AS_SETHPG
  ioPorts().readByte(IOPorts::ADDR_AS_SW_HISCR);
  ioPorts().readByte(IOPorts::ADDR_AS_SW_MIXCLR);
  variables().AS_HGR_PAGE = 0x40u;
  STHPG();
}

void AS_HGR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HGR (inclusive) .. AS_SETHPG (exclusive)
  // Name normalization: none (assembler label AS_HGR kept verbatim).

  // AS_HGR lda #$20 ; bit AS_SW_LOWSCR ; bit AS_SW_MIXSET
  variables().AS_HGR_PAGE = 0x20u;
  ioPorts().readByte(IOPorts::ADDR_AS_SW_LOWSCR);
  ioPorts().readByte(IOPorts::ADDR_AS_SW_MIXSET);
  STHPG();
}

void STHPG() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SETHPG (inclusive) .. AS_HCLR (exclusive)
  // Name normalization: AS_SETHPG -> STHPG (normalize based on typical 5-char
  // limit or SET.. prefix).

  // AS_SETHPG sta AS_HGR_PAGE ; lda AS_SW_HIRES ; lda AS_SW_TXTCLR
  // (Note: AS_HGR_PAGE already set by AS_HGR/AS_HGR2 in this port to simplify).
  ioPorts().readByte(IOPorts::ADDR_AS_SW_HIRES);
  ioPorts().readByte(IOPorts::ADDR_AS_SW_TXTCLR);

  // AS_HCLR lda #0 ; sta AS_HGR_BITS
  variables().AS_HGR_BITS = 0x00u;

  // AS_BKGND lda AS_HGR_PAGE ; sta AS_HGR_SHAPE+1 ; ldy #0 ; sta AS_HGR_SHAPE
  const std::uint8_t page = variables_const().AS_HGR_PAGE;
  variables().AS_HGR_SHAPE = ApplesoftVariables::makeWord(0x00u, page);

  // AS_L.AS_BKGND.1 lda AS_HGR_BITS ; sta (AS_HGR_SHAPE),Y ; jsr AS_COLOR.SHIFT
  // ; iny ; bne AS_L.AS_BKGND.1 ... Ported as a simple memory fill for now,
  // ignoring AS_COLOR.SHIFT delay/logic for background clear.
  const std::uint16_t startAddr = static_cast<std::uint16_t>(page << 8u);
  for (std::uint16_t i = 0; i < 0x2000u; ++i) {
    WriteProgramByte(static_cast<std::uint16_t>(startAddr + i), 0u);
  }
}

void AS_HCOLOR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HCOLOR (inclusive) .. AS_HPLOT (exclusive)
  // Name normalization: none (assembler label AS_HCOLOR kept verbatim).

  static constexpr std::uint8_t kColorTable[8] = {0x00u, 0x2au, 0x55u, 0x7fu,
                                                  0x80u, 0xaau, 0xd5u, 0xffu};

  const std::uint8_t color = AS_GETBYT();
  if (color > 7u) {
    AS_IQERR();
    return;
  }

  const std::uint8_t pattern = kColorTable[color];
  variables().AS_HGR_COLOR = pattern;
  variables().AS_HGR_BITS = pattern;
}

void AS_ROT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ROT (inclusive) .. AS_SCALE (exclusive)
  // Name normalization: none (assembler label AS_ROT kept verbatim).

  const std::uint8_t val = AS_GETBYT();
  variables().AS_HGR_ROTATION = val;
}

void AS_SCALE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SCALE (inclusive) .. AS_DRWPNT (exclusive)
  // Name normalization: none (assembler label AS_SCALE kept verbatim).

  const std::uint8_t val = AS_GETBYT();
  variables().AS_HGR_SCALE = val;
}

namespace {

struct HiResCoordinates {
  std::uint16_t x;
  std::uint8_t y;
  bool valid;
};

void AS_HPOSN(const HiResCoordinates &point) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: HPOSN (inclusive) .. HPLOT0 (exclusive)
  // Computes hi-res cursor address and bit state for the given coordinate.

  static constexpr std::uint8_t kMaskTable[7] = {0x81u, 0x82u, 0x84u, 0x88u,
                                                 0x90u, 0xa0u, 0xc0u};

  const std::uint16_t pageBase =
      static_cast<std::uint16_t>(variables_const().AS_HGR_PAGE << 8u);
  const std::uint16_t rowOffset = static_cast<std::uint16_t>(
      (static_cast<std::uint16_t>(point.y & 0x07u) << 10u) +
      (static_cast<std::uint16_t>(point.y & 0x38u) << 4u) +
      (static_cast<std::uint16_t>((point.y >> 6u) & 0x03u) * 0x28u));
  const std::uint16_t rowBase =
      static_cast<std::uint16_t>(pageBase + rowOffset);

  const std::uint8_t horiz = static_cast<std::uint8_t>(point.x / 7u);
  const std::uint8_t mask = kMaskTable[point.x % 7u];

  variables().AS_HGR_X = point.x;
  variables().AS_HGR_Y = point.y;

  variables().MON_GBASL = rowBase;
  variables().AS_HGR_HORIZ = horiz;
  // MON_HMASK shares $30 storage with MON_COLOR in ROM.
  variables().MON_COLOR = mask;

  // HPOSN seeds HGR_BITS from HGR_COLOR and rotates pattern on odd byte
  // columns.
  std::uint8_t hgrBits = variables_const().AS_HGR_COLOR;
  if ((horiz & 0x01u) != 0u) {
    hgrBits = static_cast<std::uint8_t>(hgrBits << 1u);
    if (hgrBits < 0xc0u) {
      hgrBits =
          static_cast<std::uint8_t>(variables_const().AS_HGR_COLOR ^ 0x7fu);
    }
  }
  variables().AS_HGR_BITS = hgrBits;
}

} // namespace

HiResCoordinates AS_HFNS() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: HFNS (inclusive) .. GGERR (exclusive)
  // Parses hi-res coordinates from TXTPTR and validates range X<280, Y<192.

  constexpr std::uint16_t kMaxXExclusive = 280u;
  constexpr std::uint8_t kMaxYExclusive = 192u;
  constexpr std::uint8_t kComma = static_cast<std::uint8_t>(',' & 0x7fu);

  AS_FRMNUM();
  AS_GETADR();

  const std::uint16_t x = variables_const().AS_LINNUM;
  if (x >= kMaxXExclusive) {
    AS_IQERR();
    return {0u, 0u, false};
  }

  AS_SYNCHR(kComma);

  const std::uint8_t y = AS_GETBYT();
  if (y >= kMaxYExclusive) {
    AS_IQERR();
    return {0u, 0u, false};
  }

  return {x, y, true};
}

void AS_HPLOT0(const HiResCoordinates &point) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: HPLOT0 (inclusive) .. MOVE_LEFT_OR_RIGHT (exclusive)
  // Plots one hi-res pixel using current HGR bit pattern.

  AS_HPOSN(point);

  const std::uint16_t rowBase = variables_const().MON_GBASL;
  const std::uint8_t horiz = variables_const().AS_HGR_HORIZ;
  const std::uint16_t pixelAddress =
      static_cast<std::uint16_t>(rowBase + horiz);

  const std::uint8_t hgrBits = variables_const().AS_HGR_BITS;
  // MON_HMASK shares $30 storage with MON_COLOR in ROM.
  const std::uint8_t mask = variables_const().MON_COLOR;
  const std::uint8_t existing = variables_const().readByte(pixelAddress);
  const std::uint8_t updated =
      static_cast<std::uint8_t>(((hgrBits ^ existing) & mask) ^ existing);
  variables().writeByte(pixelAddress, updated);
}

void AS_HGLIN(const HiResCoordinates &start, const HiResCoordinates &target) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: HGLIN (inclusive) .. COSINE_TABLE (exclusive)
  // Draws a line from current point to target point.
  // Mirrors key ROM line-state bookkeeping in
  // HGR_DX/HGR_DY/HGR_E/HGR_QUADRANT/HGR_COUNT.

  auto absInt = [](int v) -> int { return v < 0 ? -v : v; };

  int x0 = static_cast<int>(start.x);
  int y0 = static_cast<int>(start.y);
  const int x1 = static_cast<int>(target.x);
  const int y1 = static_cast<int>(target.y);

  const int dx = absInt(x1 - x0);
  const int sx = (x0 < x1) ? 1 : -1;
  const int dy = -absInt(y1 - y0);
  const int sy = (y0 < y1) ? 1 : -1;
  int err = dx + dy;

  // Seed ROM-style line state.
  variables().AS_HGR_DX = static_cast<std::uint16_t>(dx);
  variables().AS_HGR_DY = static_cast<std::uint8_t>(
      0u - static_cast<std::uint8_t>(absInt(y1 - y0)) - 1u);
  variables().AS_HGR_E = static_cast<std::uint8_t>(dx);
  variables().AS_HGR_COUNT = static_cast<std::uint8_t>(
      (static_cast<std::uint16_t>(dx + absInt(y1 - y0))) >> 8u);

  // Bit 7 follows horizontal direction (right=1, left=0); bit 0 follows
  // vertical direction (down=1, up=0).
  const std::uint8_t quadrant = static_cast<std::uint8_t>(
      ((sx > 0) ? 0x80u : 0x00u) | ((sy > 0) ? 0x01u : 0x00u));
  variables().AS_HGR_QUADRANT = quadrant;

  // ROM stores target endpoint in HGR_X/HGR_Y during HGLIN setup.
  variables().AS_HGR_X = target.x;
  variables().AS_HGR_Y = target.y;

  while (x0 != x1 || y0 != y1) {
    const int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }

    // Keep HGR_E synchronized with the current signed error accumulator.
    variables().AS_HGR_E = static_cast<std::uint8_t>(err);

    AS_HPLOT0(
        {static_cast<std::uint16_t>(x0), static_cast<std::uint8_t>(y0), true});
  }
}

void AS_HPLOT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HPLOT (inclusive) .. AS_HCOLOR (exclusive)
  // Plots a hi-res point or line for HPLOT forms, including repeated TO
  // clauses.

  constexpr std::uint8_t kTOKEN_TO = 0xc1;

  HiResCoordinates current{variables_const().AS_HGR_X,
                           variables_const().AS_HGR_Y, true};

  const std::uint8_t first_token = AS_CHRGOT();
  if (first_token != kTOKEN_TO) {
    current = AS_HFNS();
    if (!current.valid) {
      return;
    }
    AS_HPLOT0(current);
  }

  while (AS_CHRGOT() == kTOKEN_TO) {
    AS_SYNCHR(kTOKEN_TO);
    const HiResCoordinates target = AS_HFNS();
    if (!target.valid) {
      return;
    }
    AS_HGLIN(current, target);
    current = target;
  }
}

namespace {

void AS_DRWPNT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: DRWPNT (inclusive) .. DRAW (exclusive)
  // Name normalization: none (assembler label DRWPNT is prefixed with AS_ in
  // C++).
  //
  // Set up shape pointer for DRAW or XDRAW statement.
  // Looks up shape table to find requested shape data location.
  // If "AT" phrase present, gets XY coordinates and sets cursor position.
  // Returns with HGR_SHAPE pointing to shape data, HGR_ROTATION containing
  // rotation value.

  constexpr std::uint8_t kAT_TOKEN = 0xc5u; // TOKENDB for "AT"

  // Get shape number in X register.
  const std::uint8_t shape_num = AS_GETBYT();

  // Copy shape table pointer from HGR_SHAPE_PNTR to working HGR_SHAPE (16-bit
  // copy).
  const std::uint8_t shape_tbl_lo =
      ApplesoftVariables::lowByte(variables_const().AS_HGR_SHAPE_PNTR);
  const std::uint8_t shape_tbl_hi =
      ApplesoftVariables::highByte(variables_const().AS_HGR_SHAPE_PNTR);
  variables().AS_HGR_SHAPE = variables_const().AS_HGR_SHAPE_PNTR;

  const std::uint16_t shape_tbl_ptr =
      (static_cast<std::uint16_t>(shape_tbl_hi) << 8u) | shape_tbl_lo;

  // Compare requested shape number with count of shapes in table (first byte).
  const std::uint8_t num_shapes = variables_const().readByte(
      static_cast<std::uint8_t>(shape_tbl_ptr & 0xFFu));
  if (shape_num > num_shapes) {
    // Shape number too large; signal error.
    AS_IQERR();
    return; // AS_IQERR does not return, but provide explicit return for safety.
  }

  // Compute shape offset: double shape number to create 2-byte table index.
  const std::uint16_t shape_index = static_cast<std::uint16_t>(shape_num) * 2u;

  // Look up shape offset in table using the doubled index.
  // Shape offset table starts after the count byte.
  const std::uint16_t table_entry_addr = shape_tbl_ptr + 1u + shape_index;

  // Read 2-byte offset from table.
  // Low byte: (HGR_SHAPE),Y where Y = shape_index
  // High byte: next byte
  const std::uint8_t offset_lo = variables_const().readByte(
      static_cast<std::uint8_t>(table_entry_addr & 0xFFu));
  const std::uint8_t offset_hi = variables_const().readByte(
      static_cast<std::uint8_t>((table_entry_addr + 1u) & 0xFFu));
  const std::uint16_t shape_offset =
      (static_cast<std::uint16_t>(offset_hi) << 8u) | offset_lo;

  // Add offset to shape table pointer to get actual shape data address (16-bit
  // write).
  const std::uint16_t shape_data_addr = shape_tbl_ptr + shape_offset;
  variables().AS_HGR_SHAPE = shape_data_addr;

  // Check for optional "AT" phrase.
  const std::uint8_t ch = AS_CHRGOT();
  if (ch == kAT_TOKEN) {
    // "AT" phrase found; scan over it, get coordinates, and position cursor.
    AS_SYNCHR(kAT_TOKEN); // Scan over "AT"
    const auto point =
        AS_HFNS();   // Get X- and Y-coordinates to start drawing at
    AS_HPOSN(point); // Set up cursor position
  }
}

void AS_LRUD4();
void AS_LRUD3();

void AS_MOVE_UP() {
  // Ported from MOVE_UP in monitor/paddles.o65.lst
  // Decrement HGR_Y (ABCDEFGH) logic
  // Simplified: Apple II hi-res vertical lines are complex,
  // but we can compute the new address from a new Y.
  std::uint8_t y = variables_const().AS_HGR_Y;
  if (y > 0) {
    y--;
  } else {
    y = 191;
  }
  variables().AS_HGR_Y = y;
  // Refresh GBAS via HPOSN-like logic (simplified: call HPOSN for the new Y)
  std::uint16_t x = variables_const().AS_HGR_X;
  AS_HPOSN({x, y, true});
}

void AS_MOVE_DOWN() {
  std::uint8_t y = variables_const().AS_HGR_Y;
  if (y < 191) {
    y++;
  } else {
    y = 0;
  }
  variables().AS_HGR_Y = y;
  std::uint16_t x = variables_const().AS_HGR_X;
  AS_HPOSN({x, y, true});
}

void AS_MOVE_RIGHT() {
  std::uint16_t x = variables_const().AS_HGR_X;
  if (x < 279) {
    x++;
  } else {
    x = 0;
  }
  variables().AS_HGR_X = x;
  std::uint8_t y = variables_const().AS_HGR_Y;
  AS_HPOSN({x, y, true});
}

void AS_MOVE_LEFT() {
  std::uint16_t x = variables_const().AS_HGR_X;
  if (x > 0) {
    x--;
  } else {
    x = 279;
  }
  variables().AS_HGR_X = x;
  std::uint8_t y = variables_const().AS_HGR_Y;
  AS_HPOSN({x, y, true});
}

void AS_LRUD4() {
  std::uint8_t dir = static_cast<std::uint8_t>(
      (ApplesoftVariables::highByte(variables_const().AS_HGR_DX) +
       variables_const().AS_HGR_QUADRANT) &
      0x03u);

  // Original mapping from applesoft.o65.lst:
  // 00 -- UP
  // 01 -- DOWN
  // 02 -- RIGHT
  // 03 -- LEFT
  switch (dir) {
  case 0:
    AS_MOVE_UP();
    break;
  case 1:
    AS_MOVE_DOWN();
    break;
  case 2:
    AS_MOVE_RIGHT();
    break;
  case 3:
    AS_MOVE_LEFT();
    break;
  }
}

void AS_LRUD3_SETBIT() {
  std::uint8_t horiz = variables_const().AS_HGR_HORIZ;
  std::uint8_t mask = variables_const().MON_COLOR;
  std::uint16_t gbas = variables_const().MON_GBASL;
  std::uint8_t screen = variables().pointer(gbas).read(horiz);

  if (screen & mask) {
    variables().AS_HGR_COLLISIONS =
        static_cast<std::uint8_t>(variables_const().AS_HGR_COLLISIONS + 1u);
  }
  variables().pointer(gbas).write(screen | mask, horiz);
}

void AS_LRUD3_XORBIT() {
  std::uint8_t horiz = variables_const().AS_HGR_HORIZ;
  std::uint8_t mask = variables_const().MON_COLOR;
  std::uint16_t gbas = variables_const().MON_GBASL;
  std::uint8_t screen = variables().pointer(gbas).read(horiz);

  variables().pointer(gbas).write(screen ^ (mask & 0x7Fu), horiz);
}

void AS_LRUD1() {
  if (ApplesoftVariables::highByte(variables_const().AS_HGR_DX) & 0x04u) {
    AS_LRUD3_SETBIT();
  }
  AS_LRUD4();
}

void AS_LRUDX1() {
  if (ApplesoftVariables::highByte(variables_const().AS_HGR_DX) & 0x04u) {
    AS_LRUD3_XORBIT();
  }
  AS_LRUD4();
}

static constexpr std::uint8_t kCosineTable[] = {
    0xff, 0xfe, 0xfa, 0xf4, 0xec, 0xe1, 0xd4, 0xc5, 0xb4,
    0xa1, 0x8d, 0x78, 0x61, 0x49, 0x31, 0x18, 0xff};

void AS_DRAW1_Internal(bool xdraw) {
  std::uint8_t rotation = variables_const().AS_HGR_ROTATION;
  variables().AS_HGR_QUADRANT = static_cast<std::uint8_t>(rotation >> 4u);

  std::uint8_t trigIndex = rotation & 0x0Fu;
  ApplesoftVariables::setLowByte(variables().AS_HGR_DX,
                                 kCosineTable[trigIndex]);
  variables().AS_HGR_DY =
      static_cast<std::uint8_t>(kCosineTable[15 - trigIndex] + 1u);

  variables().AS_HGR_COLLISIONS = 0u;

  std::uint16_t shapeAddr = variables_const().AS_HGR_SHAPE;

  while (true) {
    std::uint8_t shapeByte = variables().pointer(shapeAddr).read(0);
    if (shapeByte == 0)
      break;

    ApplesoftVariables::setHighByte(variables().AS_HGR_DX, shapeByte);

    for (int i = 0; i < 3; ++i) {
      std::uint8_t vector = static_cast<std::uint8_t>(
          ApplesoftVariables::highByte(variables_const().AS_HGR_DX) & 0x07u);
      if (vector == 0 && i > 0)
        break; // End of byte

      // Move/Plot based on vector
      // This is simplified: the 3-bit vectors translate to moves.
      // Vector 0-7: 0=Up, 1=Right, 2=Down, 3=Left, 4=Up+Plot...
      // Actually DRAW1 uses the bit patterns to call LRUD.

      std::uint8_t scale = variables_const().AS_HGR_SCALE;
      while (scale--) {
        if (xdraw)
          AS_LRUDX1();
        else
          AS_LRUD1();
      }

      // Shift to next vector in byte
      ApplesoftVariables::setHighByte(
          variables().AS_HGR_DX,
          static_cast<std::uint8_t>(
              ApplesoftVariables::highByte(variables_const().AS_HGR_DX) >> 3u));
    }
    shapeAddr++;
    variables().AS_HGR_SHAPE = shapeAddr;
  }
}

void AS_DRAW1() { AS_DRAW1_Internal(false); }

void AS_XDRAW1() { AS_DRAW1_Internal(true); }

} // namespace

void AS_DRAW() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_DRAW (inclusive) .. AS_XDRAW (exclusive)
  // Name normalization: DRAW -> AS_DRAW (applesoft virtual prefix).
  // DRAW statement: parse/prepare shape draw point, then dispatch to DRAW1.

  AS_DRWPNT();
  AS_DRAW1();
}

void AS_XDRAW() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_XDRAW (inclusive) .. AS_SHLOAD (exclusive)
  // Name normalization: XDRAW -> AS_XDRAW (applesoft virtual prefix).
  // XDRAW statement: parse/prepare shape draw point, then dispatch to XDRAW1.

  AS_DRWPNT();
  AS_XDRAW1();
}

void AS_COLOR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_COLOR (inclusive) .. AS_VTAB (exclusive)
  // Name normalization: none (assembler label AS_COLOR kept verbatim).

  const std::uint8_t color = AS_GETBYT();
  MON_SETCOL(color);
}

void AS_VTAB() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_VTAB (inclusive) .. AS_SPEED (exclusive)
  // Name normalization: none (assembler label AS_VTAB kept verbatim).

  const std::uint8_t line = AS_GETBYT();
  if (line == 0u || line > 24u) {
    AS_IQERR();
    return;
  }

  // ROM uses 1-based AS_VTAB input and passes 0-based row to MON_TABV.
  MON_TABV(static_cast<std::uint8_t>(line - 1u));
}

void AS_HIMEM() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HIMEM (inclusive) .. AS_LOMEM (exclusive)
  // Name normalization: none (assembler label AS_HIMEM kept verbatim).

  AS_FRMNUM();
  AS_GETADR();

  // Check AS_LINNUM >= AS_STREND (must be above string storage)
  const std::uint8_t linnum_lo =
      ApplesoftVariables::lowByte(variables_const().AS_LINNUM);
  const std::uint8_t linnum_hi =
      ApplesoftVariables::highByte(variables_const().AS_LINNUM);
  const std::uint8_t strend_lo =
      ApplesoftVariables::lowByte(variables_const().AS_STREND);
  const std::uint8_t strend_hi =
      ApplesoftVariables::highByte(variables_const().AS_STREND);

  // Compare: if linnum < strend, error
  if (linnum_hi < strend_hi ||
      (linnum_hi == strend_hi && linnum_lo < strend_lo)) {
    AS_MEMERR();
    return;
  }

  // Valid: store to AS_MEMSIZ and AS_FRETOP
  variables().AS_MEMSIZ = variables_const().AS_LINNUM;
  variables().AS_FRETOP = variables_const().AS_LINNUM;
}

void AS_LOMEM() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_LOMEM (inclusive) .. AS_ONERR (exclusive)
  // Name normalization: none (assembler label AS_LOMEM kept verbatim).

  AS_FRMNUM();
  AS_GETADR();

  // Check AS_LINNUM < AS_MEMSIZ (must be below AS_HIMEM)
  const std::uint8_t linnum_lo =
      ApplesoftVariables::lowByte(variables_const().AS_LINNUM);
  const std::uint8_t linnum_hi =
      ApplesoftVariables::highByte(variables_const().AS_LINNUM);
  const std::uint8_t memsiz_lo =
      ApplesoftVariables::lowByte(variables_const().AS_MEMSIZ);
  const std::uint8_t memsiz_hi =
      ApplesoftVariables::highByte(variables_const().AS_MEMSIZ);

  // If linnum >= memsiz, error
  if (linnum_hi > memsiz_hi ||
      (linnum_hi == memsiz_hi && linnum_lo >= memsiz_lo)) {
    AS_MEMERR();
    return;
  }

  // Check AS_LINNUM > AS_TXTTAB (must be above program text)
  const std::uint8_t txttab_lo =
      ApplesoftVariables::lowByte(variables_const().AS_TXTTAB);
  const std::uint8_t txttab_hi =
      ApplesoftVariables::highByte(variables_const().AS_TXTTAB);

  // If linnum <= txttab, error
  if (linnum_hi < txttab_hi ||
      (linnum_hi == txttab_hi && linnum_lo <= txttab_lo)) {
    AS_MEMERR();
    return;
  }

  // Valid: store to AS_VARTAB and call AS_CLEARC (AS_LOMEM clears variables and
  // arrays)
  variables().AS_VARTAB = variables_const().AS_LINNUM;
  AS_CLEARC();
}

void AS_SPEED() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SPEED (inclusive) .. AS_TRACE (exclusive)
  // Name normalization: none (assembler label AS_SPEED kept verbatim).

  const std::uint8_t speed = AS_GETBYT();

  // ROM computes AS_SPEEDZ = 0x100 - AS_SPEED (via EOR #$FF / INX sequence).
  variables().AS_SPEEDZ = static_cast<std::uint8_t>(0u - speed);
}

std::uint8_t AS_PLOTFNS() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PLOTFNS (inclusive) .. AS_GOERR (exclusive)
  // Name normalization: none (assembler label AS_PLOTFNS kept verbatim).
  //
  // Parses "A,B" with each coordinate constrained to < 48.
  // Stores A in AS_FIRST and mirrors B into MON_H2/MON_V2.

  constexpr std::uint8_t kMaxCoordExclusive = 48u;
  constexpr std::uint8_t kComma = static_cast<std::uint8_t>(',' & 0x7fu);

  const std::uint8_t first = AS_GETBYT();
  if (first >= kMaxCoordExclusive) {
    AS_IQERR();
    return 0u;
  }
  variables().AS_FIRST = first;

  AS_SYNCHR(kComma);

  const std::uint8_t second = AS_GETBYT();
  if (second >= kMaxCoordExclusive) {
    AS_IQERR();
    return 0u;
  }

  variables().MON_H2 = second;
  variables().MON_V2 = second;
  return second;
}

std::uint8_t AS_LINCOOR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_LINCOOR (inclusive) .. AS_PLOT (exclusive)
  // Name normalization: none (assembler label AS_LINCOOR kept verbatim).
  //
  // Parses "A,B AT C" used by AS_HLIN/AS_VLIN:
  // - normalizes endpoints so AS_FIRST <= MON_H2
  // - requires AT token
  // - returns C coordinate when C < 48

  constexpr std::uint8_t kMaxCoordExclusive = 48u;
  constexpr std::uint8_t kTOKEN_AT = 0xc5u;

  const std::uint8_t bValue = AS_PLOTFNS();
  const std::uint8_t aValue = variables_const().AS_FIRST;

  if (bValue < aValue) {
    variables().MON_H2 = aValue;
    variables().MON_V2 = aValue;
    variables().AS_FIRST = bValue;
  }

  AS_SYNCHR(kTOKEN_AT);

  const std::uint8_t cValue = AS_GETBYT();
  if (cValue >= kMaxCoordExclusive) {
    AS_IQERR();
    return 0u;
  }

  return cValue;
}

void AS_PLOT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PLOT (inclusive) .. AS_HLIN (exclusive)
  // Name normalization: none (assembler label AS_PLOT kept verbatim).

  constexpr std::uint8_t kMaxXExclusive = 40u;

  const std::uint8_t yCoord = AS_PLOTFNS();
  const std::uint8_t xCoord = variables_const().AS_FIRST;

  if (xCoord >= kMaxXExclusive) {
    AS_IQERR();
    return;
  }

  MON_PLOT(yCoord, xCoord);
}

void AS_HLIN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HLIN (inclusive) .. AS_VLIN (exclusive)
  // Name normalization: none (assembler label AS_HLIN kept verbatim).

  constexpr std::uint8_t kMaxXExclusive = 40u;

  const std::uint8_t yCoord = AS_LINCOOR();
  const std::uint8_t right = variables_const().MON_H2;
  if (right >= kMaxXExclusive) {
    AS_IQERR();
    return;
  }

  const std::uint8_t left = variables_const().AS_FIRST;
  MON_HLINE(yCoord, right, left);
}

void AS_VLIN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_VLIN (inclusive) .. AS_COLOR (exclusive)
  // Name normalization: none (assembler label AS_VLIN kept verbatim).

  constexpr std::uint8_t kMaxXExclusive = 40u;

  const std::uint8_t xCoord = AS_LINCOOR();
  if (xCoord >= kMaxXExclusive) {
    AS_IQERR();
    return;
  }

  const std::uint8_t top = variables_const().AS_FIRST;
  MON_VLINE(xCoord, top);
}
} // namespace applesoft::asm_port
