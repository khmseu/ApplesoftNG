#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"
#include "core/io_ports.hpp"

#include <cstdint>

namespace applesoft::asm_port {

void CRDO();
void FRMNUM();
void GETADR();
std::uint8_t MEMERR();
void CLEARC();
std::uint8_t GETBYT();
void IQERR();
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void SYNCHR(std::uint8_t expected);

// TODO(asm-port): port NORMAL statement behavior (currently display-mode init stub).
void NORMAL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NORMAL (inclusive) .. INVERSE (exclusive)
    // Name normalization: none (assembler label NORMAL kept verbatim).

    constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
    constexpr std::uint8_t kFLASH_BIT = ApplesoftVariables::ZP_FLASH_BIT;

    WriteZeroPageByte(kMON_INVFLG, 0xffu);
    WriteZeroPageByte(kFLASH_BIT, 0x00u);
}

void INVERSE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: INVERSE (inclusive) .. FLASH (exclusive)
    // Name normalization: none (assembler label INVERSE kept verbatim).

    constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
    constexpr std::uint8_t kFLASH_BIT = ApplesoftVariables::ZP_FLASH_BIT;

    WriteZeroPageByte(kMON_INVFLG, 0x3fu);
    WriteZeroPageByte(kFLASH_BIT, 0x00u);
}

void FLASH() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FLASH (inclusive) .. COLOR (exclusive)
    // Name normalization: none (assembler label FLASH kept verbatim).

    constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
    constexpr std::uint8_t kFLASH_BIT = ApplesoftVariables::ZP_FLASH_BIT;

    WriteZeroPageByte(kMON_INVFLG, 0x7fu);
    WriteZeroPageByte(kFLASH_BIT, 0x40u);
}

void MON_SETCOL(std::uint8_t color) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
    // Labels: SETCOL (inclusive) .. SCRN (exclusive)
    // Name normalization: SETCOL -> MON_SETCOL (monitor label gets MON_ prefix).

    constexpr std::uint8_t kMON_COLOR = ApplesoftVariables::ZP_MON_COLOR;

    // Monitor stores a 4-bit color and mirrors it into both nibbles (17*A mod 16).
    const std::uint8_t nibble = static_cast<std::uint8_t>(color & 0x0fu);
    const std::uint8_t packed = static_cast<std::uint8_t>((nibble << 4u) | nibble);
    WriteZeroPageByte(kMON_COLOR, packed);
}

void MON_TABV(std::uint8_t row_zero_based) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
    // Labels: TABV (inclusive) .. external VTAB jump target (exclusive)
    // Name normalization: TABV -> MON_TABV (monitor label gets MON_ prefix).
    //
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display2.o65.lst
    // Labels: VTAB (inclusive) .. ESC1 (exclusive)
    // Name normalization: VTAB logic inlined into MON_TABV.

    constexpr std::uint8_t kMON_WNDLFT = ApplesoftVariables::ZP_MON_WNDLFT;
    constexpr std::uint8_t kMON_CV = ApplesoftVariables::ZP_MON_CV;
    constexpr std::uint8_t kMON_BASL = ApplesoftVariables::ZP_MON_BASL;
    constexpr std::uint8_t kMON_BASH = ApplesoftVariables::ZP_MON_BASH;

    // display1 TABV: sta CV ; jmp VTAB
    WriteZeroPageByte(kMON_CV, row_zero_based);

    // display2 VTAB/VTABZ path:
    // BASCALC computes BASL/BASH for row in CV, then VTAB adds WNDLFT to BASL.
    const std::uint8_t line = ReadZeroPageByte(kMON_CV);
    const bool carryFromLsr = (line & 0x01u) != 0u;
    const std::uint8_t bash = static_cast<std::uint8_t>(((line >> 1u) & 0x03u) | 0x04u);

    std::uint8_t basl = static_cast<std::uint8_t>(line & 0x18u);
    if (carryFromLsr) {
        basl = static_cast<std::uint8_t>(basl + 0x80u);
    }
    const std::uint8_t baslBase = basl;
    basl = static_cast<std::uint8_t>((basl << 2u) | baslBase);
    basl = static_cast<std::uint8_t>(basl + ReadZeroPageByte(kMON_WNDLFT));

    WriteZeroPageByte(kMON_BASL, basl);
    WriteZeroPageByte(kMON_BASH, bash);
}

namespace {

void MON_IOPRT(std::uint8_t slot, std::uint8_t vectorBase, std::uint8_t defaultVectorLow) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: IOPRT (inclusive) .. XBASIC (exclusive)
    // Name normalization: helper name chosen for shared I/O-vector setup body.

    constexpr std::uint8_t kSlotMask = 0x0fu;
    constexpr std::uint8_t kIoBaseHigh = 0xc0u;     // >IOADR
    constexpr std::uint8_t kDefaultHigh = 0xfdu;    // >COUT1 in Apple II+ monitor bank

    std::uint8_t vectorLow = defaultVectorLow;
    std::uint8_t vectorHigh;

    const std::uint8_t slotNibble = static_cast<std::uint8_t>(slot & kSlotMask);
    if (slotNibble == 0u) {
        vectorHigh = kDefaultHigh;
    } else {
        vectorLow = 0u;
        vectorHigh = static_cast<std::uint8_t>(slotNibble | kIoBaseHigh);
    }

    WriteZeroPageByte(vectorBase, vectorLow);
    WriteZeroPageByte(static_cast<std::uint8_t>(vectorBase + 1u), vectorHigh);
}

} // namespace

void MON_INPORT(std::uint8_t slot) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: INPORT (inclusive) .. SETVID (exclusive)
    // Name normalization: INPORT -> MON_INPORT (monitor label gets MON_ prefix).

    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_KSW = ApplesoftVariables::ZP_MON_KSW;
    constexpr std::uint8_t kKeyinLow = 0x0fu; // <KEYIN from keyin.o65.sym

    WriteZeroPageByte(kMON_A2L, slot);
    MON_IOPRT(ReadZeroPageByte(kMON_A2L), kMON_KSW, kKeyinLow);
}

void MON_OUTPORT(std::uint8_t slot) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
    // Labels: OUTPORT (inclusive) .. IOPRT (exclusive)
    // Name normalization: OUTPORT -> MON_OUTPORT (monitor label gets MON_ prefix).

    constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
    constexpr std::uint8_t kMON_CSW = ApplesoftVariables::ZP_MON_CSW;
    constexpr std::uint8_t kCout1Low = 0x62u; // <COUT1 from cmd.o65.sym

    WriteZeroPageByte(kMON_A2L, slot);
    MON_IOPRT(ReadZeroPageByte(kMON_A2L), kMON_CSW, kCout1Low);
}

void MON_PLOT(std::uint8_t y, std::uint8_t x) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
    // Labels: PLOT (inclusive) .. HLINE (exclusive)
    // Name normalization: PLOT -> MON_PLOT (monitor label gets MON_ prefix).
    //
    // Mirrors ROM PLOT/PLOT1 behavior: compute lo-res cell base from Y,
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
    // Labels: HLINE (inclusive) .. VLINEZ (exclusive)
    // Name normalization: HLINE -> MON_HLINE (monitor label gets MON_ prefix).
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
    // Labels: VLINEZ (inclusive) .. RTS1 (exclusive)
    // Name normalization: VLINE -> MON_VLINE (monitor label gets MON_ prefix).
    //
    // Bottom endpoint is MON_V2 ($2d), as established by Applesoft LINCOOR.

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
    // Labels: HOME (inclusive) .. CR (exclusive)
    // Name normalization: HOME -> MON_HOME (monitor label gets MON_ prefix).
    //
    // Monitor HOME initializes cursor to top-left of current window and clears
    // the window to blanks (high-bit set ASCII space).

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
    // Labels: CLREOL (inclusive) .. RTS (exclusive)
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
    // Labels: SETTXT (inclusive) .. SETGR (exclusive)
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

    // SETWND tail sets A=23 and jumps to VTAB.
    MON_TABV(23u);
}

void MON_SETGR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
    // Labels: SETGR (inclusive) .. SETWND (exclusive)
    // Name normalization: SETGR -> MON_SETGR (monitor label gets MON_ prefix).
    //
    // Falls through into SETWND in ROM; modeled here by writing the window
    // fields and tabbing to row 23.

    constexpr std::uint8_t kMON_WNDLFT = ApplesoftVariables::ZP_MON_WNDLFT;
    constexpr std::uint8_t kMON_WNDWDTH = ApplesoftVariables::ZP_MON_WNDWDTH;
    constexpr std::uint8_t kMON_WNDTOP = ApplesoftVariables::ZP_MON_WNDTOP;
    constexpr std::uint8_t kMON_WNDBTM = ApplesoftVariables::ZP_MON_WNDBTM;

    (void)variables_const().readByte(IOPorts::ADDR_SW_TXTCLR);
    (void)variables_const().readByte(IOPorts::ADDR_SW_MIXSET);

    // CLRTOP call target is still pending; window state is applied directly here.

    WriteZeroPageByte(kMON_WNDTOP, 20u);
    WriteZeroPageByte(kMON_WNDLFT, 0u);
    WriteZeroPageByte(kMON_WNDWDTH, 40u);
    WriteZeroPageByte(kMON_WNDBTM, 24u);

    MON_TABV(23u);
}

void HOME() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MON_HOME (inclusive) .. ROT (exclusive)
    // Name normalization: HOME statement maps to MON_HOME monitor routine ($FC58).

    MON_HOME();
}

void GR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GR (inclusive) .. TEXT (exclusive)
    // Name normalization: none (assembler label GR kept verbatim).
    //
    // lda SW_LORES ($c056): soft-switch read activates lo-res graphics mode.
    // lda SW_MIXSET ($c053): soft-switch read enables lower 4 lines as text.
    // jmp MON_SETGR: monitor SETGR sets up the lo-res graphics window.
    // Soft-switch side-effect reads carry no value; mode is established by MON_SETGR.
    MON_SETGR();
}

void TEXT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: TEXT (inclusive) .. STORE (exclusive)
    // Name normalization: none (assembler label TEXT kept verbatim).
    //
    // lda SW_LOWSCR ($c054): soft-switch read selects display page 1.
    // jmp MON_SETTXT: monitor SETTXT sets the full-screen text window.
    // Soft-switch side-effect read carries no value; state is set by MON_SETTXT.
    MON_SETTXT();
}

void HTAB() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HTAB (inclusive) .. end of label range (exclusive)
    // Name normalization: none (assembler label HTAB kept verbatim).
    //
    // jsr GETBYT  — evaluate expression; result in X-reg (1-based column).
    // dex         — convert to 0-based.
    // L_HTAB_1: if col >= 40, subtract 40 and emit CR (handles columns > screen width).
    // L_HTAB_2: sta MON_CH ($24) — store final column into cursor position register.
    const std::uint8_t raw = GETBYT();
    std::uint8_t col = static_cast<std::uint8_t>(raw - 1u);  // dex; txa
    while (col >= 40u) {
        col = static_cast<std::uint8_t>(col - 40u);
        CRDO();
    }
    variables().writeByte(ApplesoftVariables::ZP_MON_CH, col);
}

void HCOLOR();
void STHPG();

void HGR2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HGR2 (inclusive) .. SETHPG (exclusive)
    // Name normalization: none (assembler label HGR2 kept verbatim).

    // HGR2 bit SW_HISCR ; bit SW_MIXCLR ; lda #$40 ; bne SETHPG
    ioPorts().readByte(IOPorts::ADDR_SW_HISCR);
    ioPorts().readByte(IOPorts::ADDR_SW_MIXCLR);
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_PAGE, 0x40u);
    STHPG();
}

void HGR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HGR (inclusive) .. SETHPG (exclusive)
    // Name normalization: none (assembler label HGR kept verbatim).

    // HGR lda #$20 ; bit SW_LOWSCR ; bit SW_MIXSET
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_PAGE, 0x20u);
    ioPorts().readByte(IOPorts::ADDR_SW_LOWSCR);
    ioPorts().readByte(IOPorts::ADDR_SW_MIXSET);
    STHPG();
}

void STHPG() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SETHPG (inclusive) .. HCLR (exclusive)
    // Name normalization: SETHPG -> STHPG (normalize based on typical 5-char limit or SET.. prefix).

    // SETHPG sta HGR_PAGE ; lda SW_HIRES ; lda SW_TXTCLR
    // (Note: HGR_PAGE already set by HGR/HGR2 in this port to simplify).
    ioPorts().readByte(IOPorts::ADDR_SW_HIRES);
    ioPorts().readByte(IOPorts::ADDR_SW_TXTCLR);

    // HCLR lda #0 ; sta HGR_BITS
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_BITS, 0x00u);

    // BKGND lda HGR_PAGE ; sta HGR_SHAPE+1 ; ldy #0 ; sta HGR_SHAPE
    const std::uint8_t page = ReadZeroPageByte(ApplesoftVariables::ZP_HGR_PAGE);
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_SHAPE + 1, page);
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_SHAPE, 0x00u);

    // L.BKGND.1 lda HGR_BITS ; sta (HGR_SHAPE),Y ; jsr COLOR.SHIFT ; iny ; bne L.BKGND.1 ...
    // Ported as a simple memory fill for now, ignoring COLOR.SHIFT delay/logic for background clear.
    const std::uint16_t startAddr = static_cast<std::uint16_t>(page << 8u);
    for (std::uint16_t i = 0; i < 0x2000u; ++i) {
        WriteProgramByte(static_cast<std::uint16_t>(startAddr + i), 0u);
    }
}

void HCOLOR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HCOLOR (inclusive) .. HPLOT (exclusive)
    // Name normalization: none (assembler label HCOLOR kept verbatim).

    const std::uint8_t color = GETBYT();
    if (color > 7u) {
        IQERR();
        return;
    }
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_BITS, color);
}

void ROT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ROT (inclusive) .. SCALE (exclusive)
    // Name normalization: none (assembler label ROT kept verbatim).

    const std::uint8_t val = GETBYT();
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_ROTATION, val);
}

void SCALE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SCALE (inclusive) .. DRWPNT (exclusive)
    // Name normalization: none (assembler label SCALE kept verbatim).

    const std::uint8_t val = GETBYT();
    WriteZeroPageByte(ApplesoftVariables::ZP_HGR_SCALE, val);
}

void HPLOT() {
    // TODO(asm-port): port HPLOT label range from Applesoft ROM.
    // Plots a hi-res graphics point.
}

void DRAW() {
    // TODO(asm-port): port DRAW label range from Applesoft ROM.
    // Draws a hi-res shape.
}

void XDRAW() {
    // TODO(asm-port): port XDRAW label range from Applesoft ROM.
    // Draws a hi-res shape with XOR mode.
}

void COLOR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: COLOR (inclusive) .. VTAB (exclusive)
    // Name normalization: none (assembler label COLOR kept verbatim).

    const std::uint8_t color = GETBYT();
    MON_SETCOL(color);
}

void VTAB() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: VTAB (inclusive) .. SPEED (exclusive)
    // Name normalization: none (assembler label VTAB kept verbatim).

    const std::uint8_t line = GETBYT();
    if (line == 0u || line > 24u) {
        IQERR();
        return;
    }

    // ROM uses 1-based VTAB input and passes 0-based row to MON_TABV.
    MON_TABV(static_cast<std::uint8_t>(line - 1u));
}

void HIMEM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HIMEM (inclusive) .. LOMEM (exclusive)
    // Name normalization: none (assembler label HIMEM kept verbatim).

    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kSTREND = ApplesoftVariables::ZP_STREND;
    constexpr std::uint8_t kMEMSIZ = ApplesoftVariables::ZP_MEMSIZ;
    constexpr std::uint8_t kFRETOP = ApplesoftVariables::ZP_FRETOP;

    FRMNUM();
    GETADR();

    // Check LINNUM >= STREND (must be above string storage)
    const std::uint8_t linnum_lo = ReadZeroPageByte(kLINNUM);
    const std::uint8_t linnum_hi = ReadZeroPageByte(kLINNUM + 1u);
    const std::uint8_t strend_lo = ReadZeroPageByte(kSTREND);
    const std::uint8_t strend_hi = ReadZeroPageByte(kSTREND + 1u);

    // Compare: if linnum < strend, error
    if (linnum_hi < strend_hi || (linnum_hi == strend_hi && linnum_lo < strend_lo)) {
        MEMERR();
        return;
    }

    // Valid: store to MEMSIZ and FRETOP
    WriteZeroPageByte(kMEMSIZ, linnum_lo);
    WriteZeroPageByte(kMEMSIZ + 1u, linnum_hi);
    WriteZeroPageByte(kFRETOP, linnum_lo);
    WriteZeroPageByte(kFRETOP + 1u, linnum_hi);
}

void LOMEM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LOMEM (inclusive) .. ONERR (exclusive)
    // Name normalization: none (assembler label LOMEM kept verbatim).

    constexpr std::uint8_t kLINNUM = ApplesoftVariables::ZP_LINNUM;
    constexpr std::uint8_t kMEMSIZ = ApplesoftVariables::ZP_MEMSIZ;
    constexpr std::uint8_t kVARTAB = ApplesoftVariables::ZP_VARTAB;
    constexpr std::uint8_t kTXTTAB = ApplesoftVariables::ZP_TXTTAB;

    FRMNUM();
    GETADR();

    // Check LINNUM < MEMSIZ (must be below HIMEM)
    const std::uint8_t linnum_lo = ReadZeroPageByte(kLINNUM);
    const std::uint8_t linnum_hi = ReadZeroPageByte(kLINNUM + 1u);
    const std::uint8_t memsiz_lo = ReadZeroPageByte(kMEMSIZ);
    const std::uint8_t memsiz_hi = ReadZeroPageByte(kMEMSIZ + 1u);

    // If linnum >= memsiz, error
    if (linnum_hi > memsiz_hi || (linnum_hi == memsiz_hi && linnum_lo >= memsiz_lo)) {
        MEMERR();
        return;
    }

    // Check LINNUM > TXTTAB (must be above program text)
    const std::uint8_t txttab_lo = ReadZeroPageByte(kTXTTAB);
    const std::uint8_t txttab_hi = ReadZeroPageByte(kTXTTAB + 1u);

    // If linnum <= txttab, error
    if (linnum_hi < txttab_hi || (linnum_hi == txttab_hi && linnum_lo <= txttab_lo)) {
        MEMERR();
        return;
    }

    // Valid: store to VARTAB and call CLEARC (LOMEM clears variables and arrays)
    WriteZeroPageByte(kVARTAB, linnum_lo);
    WriteZeroPageByte(kVARTAB + 1u, linnum_hi);
    CLEARC();
}

void SPEED() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SPEED (inclusive) .. TRACE (exclusive)
    // Name normalization: none (assembler label SPEED kept verbatim).

    constexpr std::uint8_t kSPEEDZ = ApplesoftVariables::ZP_SPEEDZ;
    const std::uint8_t speed = GETBYT();

    // ROM computes SPEEDZ = 0x100 - SPEED (via EOR #$FF / INX sequence).
    WriteZeroPageByte(kSPEEDZ, static_cast<std::uint8_t>(0u - speed));
}

std::uint8_t PLOTFNS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PLOTFNS (inclusive) .. GOERR (exclusive)
    // Name normalization: none (assembler label PLOTFNS kept verbatim).
    //
    // Parses "A,B" with each coordinate constrained to < 48.
    // Stores A in FIRST and mirrors B into MON_H2/MON_V2.

    constexpr std::uint8_t kFIRST = ApplesoftVariables::ZP_FIRST;
    constexpr std::uint8_t kMON_H2 = ApplesoftVariables::ZP_MON_H2;
    constexpr std::uint8_t kMON_V2 = ApplesoftVariables::ZP_MON_V2;
    constexpr std::uint8_t kMaxCoordExclusive = 48u;
    constexpr std::uint8_t kComma = static_cast<std::uint8_t>(',' & 0x7fu);

    const std::uint8_t first = GETBYT();
    if (first >= kMaxCoordExclusive) {
        IQERR();
        return 0u;
    }
    WriteZeroPageByte(kFIRST, first);

    SYNCHR(kComma);

    const std::uint8_t second = GETBYT();
    if (second >= kMaxCoordExclusive) {
        IQERR();
        return 0u;
    }

    WriteZeroPageByte(kMON_H2, second);
    WriteZeroPageByte(kMON_V2, second);
    return second;
}

std::uint8_t LINCOOR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: LINCOOR (inclusive) .. PLOT (exclusive)
    // Name normalization: none (assembler label LINCOOR kept verbatim).
    //
    // Parses "A,B AT C" used by HLIN/VLIN:
    // - normalizes endpoints so FIRST <= MON_H2
    // - requires AT token
    // - returns C coordinate when C < 48

    constexpr std::uint8_t kFIRST = ApplesoftVariables::ZP_FIRST;
    constexpr std::uint8_t kMON_H2 = ApplesoftVariables::ZP_MON_H2;
    constexpr std::uint8_t kMON_V2 = ApplesoftVariables::ZP_MON_V2;
    constexpr std::uint8_t kMaxCoordExclusive = 48u;
    constexpr std::uint8_t kTOKEN_AT = 0xc5u;

    const std::uint8_t bValue = PLOTFNS();
    const std::uint8_t aValue = ReadZeroPageByte(kFIRST);

    if (bValue < aValue) {
        WriteZeroPageByte(kMON_H2, aValue);
        WriteZeroPageByte(kMON_V2, aValue);
        WriteZeroPageByte(kFIRST, bValue);
    }

    SYNCHR(kTOKEN_AT);

    const std::uint8_t cValue = GETBYT();
    if (cValue >= kMaxCoordExclusive) {
        IQERR();
        return 0u;
    }

    return cValue;
}

void PLOT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PLOT (inclusive) .. HLIN (exclusive)
    // Name normalization: none (assembler label PLOT kept verbatim).

    constexpr std::uint8_t kFIRST = ApplesoftVariables::ZP_FIRST;
    constexpr std::uint8_t kMaxXExclusive = 40u;

    const std::uint8_t yCoord = PLOTFNS();
    const std::uint8_t xCoord = ReadZeroPageByte(kFIRST);

    if (xCoord >= kMaxXExclusive) {
        IQERR();
        return;
    }

    MON_PLOT(yCoord, xCoord);
}

void HLIN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HLIN (inclusive) .. VLIN (exclusive)
    // Name normalization: none (assembler label HLIN kept verbatim).

    constexpr std::uint8_t kFIRST = ApplesoftVariables::ZP_FIRST;
    constexpr std::uint8_t kMON_H2 = ApplesoftVariables::ZP_MON_H2;
    constexpr std::uint8_t kMaxXExclusive = 40u;

    const std::uint8_t yCoord = LINCOOR();
    const std::uint8_t right = ReadZeroPageByte(kMON_H2);
    if (right >= kMaxXExclusive) {
        IQERR();
        return;
    }

    const std::uint8_t left = ReadZeroPageByte(kFIRST);
    MON_HLINE(yCoord, right, left);
}

void VLIN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: VLIN (inclusive) .. COLOR (exclusive)
    // Name normalization: none (assembler label VLIN kept verbatim).

    constexpr std::uint8_t kFIRST = ApplesoftVariables::ZP_FIRST;
    constexpr std::uint8_t kMaxXExclusive = 40u;

    const std::uint8_t xCoord = LINCOOR();
    if (xCoord >= kMaxXExclusive) {
        IQERR();
        return;
    }

    const std::uint8_t top = ReadZeroPageByte(kFIRST);
    MON_VLINE(xCoord, top);
}

}  // namespace applesoft::asm_port