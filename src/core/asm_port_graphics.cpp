#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
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
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void AS_SYNCHR(std::uint8_t expected);

void AS_NORMAL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_NORMAL (inclusive) .. AS_INVERSE (exclusive)
    // Name normalization: none (assembler label AS_NORMAL kept verbatim).

    constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
    constexpr std::uint8_t kAS_FLASH_BIT = ApplesoftVariables::ZP_AS_FLASH_BIT;

    WriteZeroPageByte(kMON_INVFLG, 0xffu);
    WriteZeroPageByte(kAS_FLASH_BIT, 0x00u);
}

void AS_INVERSE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_INVERSE (inclusive) .. AS_FLASH (exclusive)
    // Name normalization: none (assembler label AS_INVERSE kept verbatim).

    constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
    constexpr std::uint8_t kAS_FLASH_BIT = ApplesoftVariables::ZP_AS_FLASH_BIT;

    WriteZeroPageByte(kMON_INVFLG, 0x3fu);
    WriteZeroPageByte(kAS_FLASH_BIT, 0x00u);
}

void AS_FLASH() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_FLASH (inclusive) .. AS_COLOR (exclusive)
    // Name normalization: none (assembler label AS_FLASH kept verbatim).

    constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
    constexpr std::uint8_t kAS_FLASH_BIT = ApplesoftVariables::ZP_AS_FLASH_BIT;

    WriteZeroPageByte(kMON_INVFLG, 0x7fu);
    WriteZeroPageByte(kAS_FLASH_BIT, 0x40u);
}

void MON_SETCOL(std::uint8_t color) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
    // AS_Labels: SETCOL (inclusive) .. SCRN (exclusive)
    // Name normalization: SETCOL -> MON_SETCOL (monitor label gets MON_ prefix).

    constexpr std::uint8_t kMON_COLOR = ApplesoftVariables::ZP_MON_COLOR;

    // Monitor stores a 4-bit color and mirrors it into both nibbles (17*A mod 16).
    const std::uint8_t nibble = static_cast<std::uint8_t>(color & 0x0fu);
    const std::uint8_t packed = static_cast<std::uint8_t>((nibble << 4u) | nibble);
    WriteZeroPageByte(kMON_COLOR, packed);
}

void MON_TABV(std::uint8_t row_zero_based) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
    // AS_Labels: TABV (inclusive) .. external AS_VTAB jump target (exclusive)
    // Name normalization: TABV -> MON_TABV (monitor label gets MON_ prefix).
    //
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // AS_Labels: AS_VTAB (inclusive) .. ESC1 (exclusive)
    // Name normalization: AS_VTAB logic inlined into MON_TABV.

    constexpr std::uint8_t kMON_WNDLFT = ApplesoftVariables::ZP_MON_WNDLFT;
    constexpr std::uint8_t kMON_CV = ApplesoftVariables::ZP_MON_CV;
    constexpr std::uint8_t kMON_BASL = ApplesoftVariables::ZP_MON_BASL;
    constexpr std::uint8_t kMON_BASH = ApplesoftVariables::ZP_MON_BASH;

    // display1 TABV: sta CV ; jmp AS_VTAB
    WriteZeroPageByte(kMON_CV, row_zero_based);

    // display2 AS_VTAB/AS_VTABZ path:
    // BASCALC computes BASL/BASH for row in CV, then AS_VTAB adds WNDLFT to BASL.
    const std::uint8_t line = ReadZeroPageByte(kMON_CV);
    const bool carryFromAS_Lsr = (line & 0x01u) != 0u;
    const std::uint8_t bash = static_cast<std::uint8_t>(((line >> 1u) & 0x03u) | 0x04u);

    std::uint8_t basl = static_cast<std::uint8_t>(line & 0x18u);
    if (carryFromAS_Lsr) {
        basl = static_cast<std::uint8_t>(basl + 0x80u);
    }
    const std::uint8_t baslBase = basl;
    basl = static_cast<std::uint8_t>((basl << 2u) | baslBase);
    basl = static_cast<std::uint8_t>(basl + ReadZeroPageByte(kMON_WNDLFT));

    WriteZeroPageByte(kMON_BASL, basl);
    WriteZeroPageByte(kMON_BASH, bash);
}

namespace {

void MON_IOPRT(std::uint8_t slot, std::uint8_t vectorBase, std::uint8_t defaultVectorAS_Low) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // AS_Labels: IOPRT (inclusive) .. XBASIC (exclusive)
    // Name normalization: helper name chosen for shared I/O-vector setup body.

    constexpr std::uint8_t kSlotMask = 0x0fu;
    constexpr std::uint8_t kIoBaseHigh = 0xc0u;     // >IOADR
    constexpr std::uint8_t kDefaultHigh = 0xfdu;    // >COUT1 in Apple II+ monitor bank

    std::uint8_t vectorAS_Low = defaultVectorAS_Low;
    std::uint8_t vectorHigh;

    const std::uint8_t slotNibble = static_cast<std::uint8_t>(slot & kSlotMask);
    if (slotNibble == 0u) {
        vectorHigh = kDefaultHigh;
    } else {
        vectorAS_Low = 0u;
        vectorHigh = static_cast<std::uint8_t>(slotNibble | kIoBaseHigh);
    }

    WriteZeroPageByte(vectorBase, vectorAS_Low);
    WriteZeroPageByte(static_cast<std::uint8_t>(vectorBase + 1u), vectorHigh);
}

} // namespace

void MON_INPORT(std::uint8_t slot) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // AS_Labels: INPORT (inclusive) .. SETVID (exclusive)
    // Name normalization: INPORT -> MON_INPORT (monitor label gets MON_ prefix).

    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_KSW = ApplesoftVariables::ZP_MON_KSW;
    constexpr std::uint8_t kKeyinAS_Low = 0x0fu; // <KEYIN from keyin.o65.sym

    WriteZeroPageByte(kMON_A2L, slot);
    MON_IOPRT(ReadZeroPageByte(kMON_A2L), kMON_KSW, kKeyinAS_Low);
}

void MON_OUTPORT(std::uint8_t slot) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // AS_Labels: OUTPORT (inclusive) .. IOPRT (exclusive)
    // Name normalization: OUTPORT -> MON_OUTPORT (monitor label gets MON_ prefix).

    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_CSW = ApplesoftVariables::ZP_MON_CSW;
    constexpr std::uint8_t kCout1AS_Low = 0x62u; // <COUT1 from cmd.o65.sym

    WriteZeroPageByte(kMON_A2L, slot);
    MON_IOPRT(ReadZeroPageByte(kMON_A2L), kMON_CSW, kCout1AS_Low);
}

void MON_PLOT(std::uint8_t y, std::uint8_t x) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
    // AS_Labels: AS_PLOT (inclusive) .. AS_HLINE (exclusive)
    // Name normalization: AS_PLOT -> MON_PLOT (monitor label gets MON_ prefix).
    //
    // Mirrors ROM AS_PLOT/AS_PLOT1 behavior: compute lo-res cell base from Y,
    // then replace only the selected nibble (even row: low, odd row: high).

    constexpr std::uint8_t kMON_COLOR = ApplesoftVariables::ZP_MON_COLOR;

    const std::uint8_t halfRow = static_cast<std::uint8_t>(y >> 1u);
    const bool oddRow = (y & 0x01u) != 0u;

    // Inline GBASCALC for page $0400-$07ff lo-res screen mapping.
    const std::uint8_t gbash = static_cast<std::uint8_t>(((halfRow >> 1u) & 0x03u) | 0x04u);
    std::uint8_t gbasl = static_cast<std::uint8_t>(halfRow & 0x18u);
    if (oddRow) {
        gbasl = static_cast<std::uint8_t>(gbasl + 0x80u);
    }
    const std::uint8_t gbaslBase = gbasl;
    gbasl = static_cast<std::uint8_t>((gbasl << 2u) | gbaslBase);

    const std::uint16_t baseAddress = ApplesoftVariables::makeWord(gbasl, gbash);
    const std::uint16_t screenAddress = static_cast<std::uint16_t>(baseAddress + x);

    const std::uint8_t color = ReadZeroPageByte(kMON_COLOR);
    const std::uint8_t existing = variables_const().readByte(screenAddress);
    const std::uint8_t merged = oddRow
        ? static_cast<std::uint8_t>((existing & 0x0fu) | (color & 0xf0u))
        : static_cast<std::uint8_t>((existing & 0xf0u) | (color & 0x0fu));

    variables().writeByte(screenAddress, merged);
}

void MON_HLINE(std::uint8_t y, std::uint8_t right, std::uint8_t left) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
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
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
    // AS_Labels: AS_VLINEZ (inclusive) .. RTS1 (exclusive)
    // Name normalization: AS_VLINE -> MON_VLINE (monitor label gets MON_ prefix).
    //
    // Bottom endpoint is MON_V2 ($2d), as established by Applesoft AS_LINCOOR.

    constexpr std::uint8_t kMON_V2 = ApplesoftVariables::ZP_MON_V2;
    const std::uint8_t bottom = ReadZeroPageByte(kMON_V2);

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
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // AS_Labels: HOME (inclusive) .. CR (exclusive)
    // Name normalization: HOME -> MON_HOME (monitor label gets MON_ prefix).
    //
    // Monitor HOME initializes cursor to top-left of current window and clears
    // the window to blanks (high-bit set AS_ASCII space).

    constexpr std::uint8_t kMON_WNDLFT = ApplesoftVariables::ZP_MON_WNDLFT;
    constexpr std::uint8_t kMON_WNDWDTH = ApplesoftVariables::ZP_MON_WNDWDTH;
    constexpr std::uint8_t kMON_WNDTOP = ApplesoftVariables::ZP_MON_WNDTOP;
    constexpr std::uint8_t kMON_WNDBTM = ApplesoftVariables::ZP_MON_WNDBTM;
    constexpr std::uint8_t kMON_CH = ApplesoftVariables::ZP_MON_CH;
    constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);

    const std::uint8_t windowTop = ReadZeroPageByte(kMON_WNDTOP);
    const std::uint8_t windowBottom = ReadZeroPageByte(kMON_WNDBTM);
    const std::uint8_t width = ReadZeroPageByte(kMON_WNDWDTH);
    const std::uint8_t left = ReadZeroPageByte(kMON_WNDLFT);

    WriteZeroPageByte(kMON_CH, 0u);

    for (std::uint8_t row = windowTop; row < windowBottom; ++row) {
        MON_TABV(row);
        const std::uint16_t baseAddress = ApplesoftVariables::makeWord(
            ReadZeroPageByte(ApplesoftVariables::ZP_MON_BASL),
            ReadZeroPageByte(ApplesoftVariables::ZP_MON_BASH));

        for (std::uint8_t col = 0u; col < width; ++col) {
            const std::uint16_t address = static_cast<std::uint16_t>(baseAddress + left + col);
            variables().writeByte(address, kBlank);
        }
    }

    MON_TABV(windowTop);
    WriteZeroPageByte(kMON_CH, 0u);
}

void MON_CLREOL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // AS_Labels: CLREOL (inclusive) .. RTS (exclusive)
    // Name normalization: CLREOL -> MON_CLREOL (monitor label gets MON_ prefix).
    //
    // Clears from current cursor position to end of line with high-bit-set spaces.

    constexpr std::uint8_t kMON_CH = ApplesoftVariables::ZP_MON_CH;
    constexpr std::uint8_t kMON_WNDWDTH = ApplesoftVariables::ZP_MON_WNDWDTH;
    constexpr std::uint8_t kMON_BASL = ApplesoftVariables::ZP_MON_BASL;
    constexpr std::uint8_t kMON_BASH = ApplesoftVariables::ZP_MON_BASH;
    constexpr std::uint8_t kBlank = static_cast<std::uint8_t>(' ' | 0x80u);

    const std::uint16_t baseAddress = ApplesoftVariables::makeWord(
        ReadZeroPageByte(kMON_BASL),
        ReadZeroPageByte(kMON_BASH));
    const std::uint8_t startCol = ReadZeroPageByte(kMON_CH);
    const std::uint8_t windowWidth = ReadZeroPageByte(kMON_WNDWDTH);

    for (std::uint8_t col = startCol; col < windowWidth; ++col) {
        const std::uint16_t address = static_cast<std::uint16_t>(baseAddress + col);
        variables().writeByte(address, kBlank);
    }
}

void MON_SETTXT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
    // AS_Labels: SETTXT (inclusive) .. SETGR (exclusive)
    // Name normalization: SETTXT -> MON_SETTXT (monitor label gets MON_ prefix).
    //
    // ROM fall-through: SETTXT unconditionally branches to SETWND; modeled
    // directly here by writing the window fields and tabbing to row 23.

    constexpr std::uint8_t kMON_WNDLFT = ApplesoftVariables::ZP_MON_WNDLFT;
    constexpr std::uint8_t kMON_WNDWDTH = ApplesoftVariables::ZP_MON_WNDWDTH;
    constexpr std::uint8_t kMON_WNDTOP = ApplesoftVariables::ZP_MON_WNDTOP;
    constexpr std::uint8_t kMON_WNDBTM = ApplesoftVariables::ZP_MON_WNDBTM;

    (void)variables_const().readByte(IOPorts::ADDR_SW_TXTSET);

    WriteZeroPageByte(kMON_WNDTOP, 0u);
    WriteZeroPageByte(kMON_WNDLFT, 0u);
    WriteZeroPageByte(kMON_WNDWDTH, 40u);
    WriteZeroPageByte(kMON_WNDBTM, 24u);

    // SETWND tail sets A=23 and jumps to AS_VTAB.
    MON_TABV(23u);
}

void MON_SETGR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
    // AS_Labels: SETGR (inclusive) .. SETWND (exclusive)
    // Name normalization: SETGR -> MON_SETGR (monitor label gets MON_ prefix).
    //
    // Falls through into SETWND in ROM; modeled here by writing the window
    // fields and tabbing to row 23.

    constexpr std::uint8_t kMON_WNDLFT = ApplesoftVariables::ZP_MON_WNDLFT;
    constexpr std::uint8_t kMON_WNDWDTH = ApplesoftVariables::ZP_MON_WNDWDTH;
    constexpr std::uint8_t kMON_WNDTOP = ApplesoftVariables::ZP_MON_WNDTOP;
    constexpr std::uint8_t kMON_WNDBTM = ApplesoftVariables::ZP_MON_WNDBTM;

    (void)variables_const().readByte(IOPorts::ADDR_AS_SW_TXTCLR);
    (void)variables_const().readByte(IOPorts::ADDR_AS_SW_MIXSET);

    // CLRTOP call target is still pending; window state is applied directly here.

    WriteZeroPageByte(kMON_WNDTOP, 20u);
    WriteZeroPageByte(kMON_WNDLFT, 0u);
    WriteZeroPageByte(kMON_WNDWDTH, 40u);
    WriteZeroPageByte(kMON_WNDBTM, 24u);

    MON_TABV(23u);
}

void HOME() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: MON_HOME (inclusive) .. AS_ROT (exclusive)
    // Name normalization: HOME statement maps to MON_HOME monitor routine ($FC58).

    MON_HOME();
}

void AS_GR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_GR (inclusive) .. AS_TEXT (exclusive)
    // Name normalization: none (assembler label AS_GR kept verbatim).
    //
    // lda AS_SW_LORES ($c056): soft-switch read activates lo-res graphics mode.
    // lda AS_SW_MIXSET ($c053): soft-switch read enables lower 4 lines as text.
    // jmp MON_SETGR: monitor SETGR sets up the lo-res graphics window.
    // Soft-switch side-effect reads carry no value; mode is established by MON_SETGR.
    MON_SETGR();
}

void AS_TEXT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_TEXT (inclusive) .. AS_STORE (exclusive)
    // Name normalization: none (assembler label AS_TEXT kept verbatim).
    //
    // lda AS_SW_LOWSCR ($c054): soft-switch read selects display page 1.
    // jmp MON_SETTXT: monitor SETTXT sets the full-screen text window.
    // Soft-switch side-effect read carries no value; state is set by MON_SETTXT.
    MON_SETTXT();
}

void AS_HTAB() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_HTAB (inclusive) .. end of label range (exclusive)
    // Name normalization: none (assembler label AS_HTAB kept verbatim).
    //
    // jsr AS_GETBYT  — evaluate expression; result in X-reg (1-based column).
    // dex         — convert to 0-based.
    // AS_L_HTAB_1: if col >= 40, subtract 40 and emit CR (handles columns > screen width).
    // AS_L_HTAB_2: sta MON_CH ($24) — store final column into cursor position register.
    const std::uint8_t raw = AS_GETBYT();
    std::uint8_t col = static_cast<std::uint8_t>(raw - 1u);  // dex; txa
    while (col >= 40u) {
        col = static_cast<std::uint8_t>(col - 40u);
        AS_CRDO();
    }
    variables().writeByte(ApplesoftVariables::ZP_MON_CH, col);
}

void AS_HCOLOR();
void STHPG();

void AS_HGR2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_HGR2 (inclusive) .. AS_SETHPG (exclusive)
    // Name normalization: none (assembler label AS_HGR2 kept verbatim).

    // AS_HGR2 bit AS_SW_HISCR ; bit AS_SW_MIXCLR ; lda #$40 ; bne AS_SETHPG
    ioPorts().readByte(IOPorts::ADDR_AS_SW_HISCR);
    ioPorts().readByte(IOPorts::ADDR_AS_SW_MIXCLR);
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_PAGE, 0x40u);
    STHPG();
}

void AS_HGR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_HGR (inclusive) .. AS_SETHPG (exclusive)
    // Name normalization: none (assembler label AS_HGR kept verbatim).

    // AS_HGR lda #$20 ; bit AS_SW_LOWSCR ; bit AS_SW_MIXSET
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_PAGE, 0x20u);
    ioPorts().readByte(IOPorts::ADDR_AS_SW_LOWSCR);
    ioPorts().readByte(IOPorts::ADDR_AS_SW_MIXSET);
    STHPG();
}

void STHPG() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_SETHPG (inclusive) .. AS_HCLR (exclusive)
    // Name normalization: AS_SETHPG -> STHPG (normalize based on typical 5-char limit or SET.. prefix).

    // AS_SETHPG sta AS_HGR_PAGE ; lda AS_SW_HIRES ; lda AS_SW_TXTCLR
    // (Note: AS_HGR_PAGE already set by AS_HGR/AS_HGR2 in this port to simplify).
    ioPorts().readByte(IOPorts::ADDR_AS_SW_HIRES);
    ioPorts().readByte(IOPorts::ADDR_AS_SW_TXTCLR);

    // AS_HCLR lda #0 ; sta AS_HGR_BITS
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_BITS, 0x00u);

    // AS_BKGND lda AS_HGR_PAGE ; sta AS_HGR_SHAPE+1 ; ldy #0 ; sta AS_HGR_SHAPE
    const std::uint8_t page = ReadZeroPageByte(ApplesoftVariables::ZP_AS_HGR_PAGE);
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_SHAPE + 1, page);
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_SHAPE, 0x00u);

    // AS_L.AS_BKGND.1 lda AS_HGR_BITS ; sta (AS_HGR_SHAPE),Y ; jsr AS_COLOR.SHIFT ; iny ; bne AS_L.AS_BKGND.1 ...
    // Ported as a simple memory fill for now, ignoring AS_COLOR.SHIFT delay/logic for background clear.
    const std::uint16_t startAddr = static_cast<std::uint16_t>(page << 8u);
    for (std::uint16_t i = 0; i < 0x2000u; ++i) {
        WriteProgramByte(static_cast<std::uint16_t>(startAddr + i), 0u);
    }
}

void AS_HCOLOR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_HCOLOR (inclusive) .. AS_HPLOT (exclusive)
    // Name normalization: none (assembler label AS_HCOLOR kept verbatim).

    const std::uint8_t color = AS_GETBYT();
    if (color > 7u) {
        AS_IQERR();
        return;
    }
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_BITS, color);
}

void AS_ROT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_ROT (inclusive) .. AS_SCALE (exclusive)
    // Name normalization: none (assembler label AS_ROT kept verbatim).

    const std::uint8_t val = AS_GETBYT();
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_ROTATION, val);
}

void AS_SCALE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_SCALE (inclusive) .. AS_DRWPNT (exclusive)
    // Name normalization: none (assembler label AS_SCALE kept verbatim).

    const std::uint8_t val = AS_GETBYT();
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_HGR_SCALE, val);
}

void AS_HPLOT() {
    // TODO(asm-port): port AS_HPLOT label range from Applesoft ROM.
    // Plots a hi-res graphics point.
}

void AS_DRAW() {
    // TODO(asm-port): port AS_DRAW label range from Applesoft ROM.
    // Draws a hi-res shape.
}

void AS_XDRAW() {
    // TODO(asm-port): port AS_XDRAW label range from Applesoft ROM.
    // Draws a hi-res shape with XOR mode.
}

void AS_COLOR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_COLOR (inclusive) .. AS_VTAB (exclusive)
    // Name normalization: none (assembler label AS_COLOR kept verbatim).

    const std::uint8_t color = AS_GETBYT();
    MON_SETCOL(color);
}

void AS_VTAB() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
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
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_HIMEM (inclusive) .. AS_LOMEM (exclusive)
    // Name normalization: none (assembler label AS_HIMEM kept verbatim).

    constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
    constexpr std::uint8_t kAS_STREND = ApplesoftVariables::ZP_AS_STREND;
    constexpr std::uint8_t kAS_MEMSIZ = ApplesoftVariables::ZP_AS_MEMSIZ;
    constexpr std::uint8_t kAS_FRETOP = ApplesoftVariables::ZP_AS_FRETOP;

    AS_FRMNUM();
    AS_GETADR();

    // Check AS_LINNUM >= AS_STREND (must be above string storage)
    const std::uint8_t linnum_lo = ReadZeroPageByte(kAS_LINNUM);
    const std::uint8_t linnum_hi = ReadZeroPageByte(kAS_LINNUM + 1u);
    const std::uint8_t strend_lo = ReadZeroPageByte(kAS_STREND);
    const std::uint8_t strend_hi = ReadZeroPageByte(kAS_STREND + 1u);

    // Compare: if linnum < strend, error
    if (linnum_hi < strend_hi || (linnum_hi == strend_hi && linnum_lo < strend_lo)) {
        AS_MEMERR();
        return;
    }

    // Valid: store to AS_MEMSIZ and AS_FRETOP
    WriteZeroPageByte(kAS_MEMSIZ, linnum_lo);
    WriteZeroPageByte(kAS_MEMSIZ + 1u, linnum_hi);
    WriteZeroPageByte(kAS_FRETOP, linnum_lo);
    WriteZeroPageByte(kAS_FRETOP + 1u, linnum_hi);
}

void AS_LOMEM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_LOMEM (inclusive) .. AS_ONERR (exclusive)
    // Name normalization: none (assembler label AS_LOMEM kept verbatim).

    constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
    constexpr std::uint8_t kAS_MEMSIZ = ApplesoftVariables::ZP_AS_MEMSIZ;
    constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
    constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;

    AS_FRMNUM();
    AS_GETADR();

    // Check AS_LINNUM < AS_MEMSIZ (must be below AS_HIMEM)
    const std::uint8_t linnum_lo = ReadZeroPageByte(kAS_LINNUM);
    const std::uint8_t linnum_hi = ReadZeroPageByte(kAS_LINNUM + 1u);
    const std::uint8_t memsiz_lo = ReadZeroPageByte(kAS_MEMSIZ);
    const std::uint8_t memsiz_hi = ReadZeroPageByte(kAS_MEMSIZ + 1u);

    // If linnum >= memsiz, error
    if (linnum_hi > memsiz_hi || (linnum_hi == memsiz_hi && linnum_lo >= memsiz_lo)) {
        AS_MEMERR();
        return;
    }

    // Check AS_LINNUM > AS_TXTTAB (must be above program text)
    const std::uint8_t txttab_lo = ReadZeroPageByte(kAS_TXTTAB);
    const std::uint8_t txttab_hi = ReadZeroPageByte(kAS_TXTTAB + 1u);

    // If linnum <= txttab, error
    if (linnum_hi < txttab_hi || (linnum_hi == txttab_hi && linnum_lo <= txttab_lo)) {
        AS_MEMERR();
        return;
    }

    // Valid: store to AS_VARTAB and call AS_CLEARC (AS_LOMEM clears variables and arrays)
    WriteZeroPageByte(kAS_VARTAB, linnum_lo);
    WriteZeroPageByte(kAS_VARTAB + 1u, linnum_hi);
    AS_CLEARC();
}

void AS_SPEED() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_SPEED (inclusive) .. AS_TRACE (exclusive)
    // Name normalization: none (assembler label AS_SPEED kept verbatim).

    constexpr std::uint8_t kAS_SPEEDZ = ApplesoftVariables::ZP_AS_SPEEDZ;
    const std::uint8_t speed = AS_GETBYT();

    // ROM computes AS_SPEEDZ = 0x100 - AS_SPEED (via EOR #$FF / INX sequence).
    WriteZeroPageByte(kAS_SPEEDZ, static_cast<std::uint8_t>(0u - speed));
}

std::uint8_t AS_PLOTFNS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_PLOTFNS (inclusive) .. AS_GOERR (exclusive)
    // Name normalization: none (assembler label AS_PLOTFNS kept verbatim).
    //
    // Parses "A,B" with each coordinate constrained to < 48.
    // Stores A in AS_FIRST and mirrors B into MON_H2/MON_V2.

    constexpr std::uint8_t kAS_FIRST = ApplesoftVariables::ZP_AS_FIRST;
    constexpr std::uint8_t kMON_H2 = ApplesoftVariables::ZP_MON_H2;
    constexpr std::uint8_t kMON_V2 = ApplesoftVariables::ZP_MON_V2;
    constexpr std::uint8_t kMaxCoordExclusive = 48u;
    constexpr std::uint8_t kComma = static_cast<std::uint8_t>(',' & 0x7fu);

    const std::uint8_t first = AS_GETBYT();
    if (first >= kMaxCoordExclusive) {
        AS_IQERR();
        return 0u;
    }
    WriteZeroPageByte(kAS_FIRST, first);

    AS_SYNCHR(kComma);

    const std::uint8_t second = AS_GETBYT();
    if (second >= kMaxCoordExclusive) {
        AS_IQERR();
        return 0u;
    }

    WriteZeroPageByte(kMON_H2, second);
    WriteZeroPageByte(kMON_V2, second);
    return second;
}

std::uint8_t AS_LINCOOR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_LINCOOR (inclusive) .. AS_PLOT (exclusive)
    // Name normalization: none (assembler label AS_LINCOOR kept verbatim).
    //
    // Parses "A,B AT C" used by AS_HLIN/AS_VLIN:
    // - normalizes endpoints so AS_FIRST <= MON_H2
    // - requires AT token
    // - returns C coordinate when C < 48

    constexpr std::uint8_t kAS_FIRST = ApplesoftVariables::ZP_AS_FIRST;
    constexpr std::uint8_t kMON_H2 = ApplesoftVariables::ZP_MON_H2;
    constexpr std::uint8_t kMON_V2 = ApplesoftVariables::ZP_MON_V2;
    constexpr std::uint8_t kMaxCoordExclusive = 48u;
    constexpr std::uint8_t kTOKEN_AT = 0xc5u;

    const std::uint8_t bValue = AS_PLOTFNS();
    const std::uint8_t aValue = ReadZeroPageByte(kAS_FIRST);

    if (bValue < aValue) {
        WriteZeroPageByte(kMON_H2, aValue);
        WriteZeroPageByte(kMON_V2, aValue);
        WriteZeroPageByte(kAS_FIRST, bValue);
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
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_PLOT (inclusive) .. AS_HLIN (exclusive)
    // Name normalization: none (assembler label AS_PLOT kept verbatim).

    constexpr std::uint8_t kAS_FIRST = ApplesoftVariables::ZP_AS_FIRST;
    constexpr std::uint8_t kMaxXExclusive = 40u;

    const std::uint8_t yCoord = AS_PLOTFNS();
    const std::uint8_t xCoord = ReadZeroPageByte(kAS_FIRST);

    if (xCoord >= kMaxXExclusive) {
        AS_IQERR();
        return;
    }

    MON_PLOT(yCoord, xCoord);
}

void AS_HLIN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_HLIN (inclusive) .. AS_VLIN (exclusive)
    // Name normalization: none (assembler label AS_HLIN kept verbatim).

    constexpr std::uint8_t kAS_FIRST = ApplesoftVariables::ZP_AS_FIRST;
    constexpr std::uint8_t kMON_H2 = ApplesoftVariables::ZP_MON_H2;
    constexpr std::uint8_t kMaxXExclusive = 40u;

    const std::uint8_t yCoord = AS_LINCOOR();
    const std::uint8_t right = ReadZeroPageByte(kMON_H2);
    if (right >= kMaxXExclusive) {
        AS_IQERR();
        return;
    }

    const std::uint8_t left = ReadZeroPageByte(kAS_FIRST);
    MON_HLINE(yCoord, right, left);
}

void AS_VLIN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // AS_Labels: AS_VLIN (inclusive) .. AS_COLOR (exclusive)
    // Name normalization: none (assembler label AS_VLIN kept verbatim).

    constexpr std::uint8_t kAS_FIRST = ApplesoftVariables::ZP_AS_FIRST;
    constexpr std::uint8_t kMaxXExclusive = 40u;

    const std::uint8_t xCoord = AS_LINCOOR();
    if (xCoord >= kMaxXExclusive) {
        AS_IQERR();
        return;
    }

    const std::uint8_t top = ReadZeroPageByte(kAS_FIRST);
    MON_VLINE(xCoord, top);
}

}  // namespace applesoft::asm_port