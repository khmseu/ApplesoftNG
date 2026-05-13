#include "core/asm_port_error.hpp"
#include "core/applesoft_variables.hpp"

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
    // TODO(asm-port): port MON_SETCOL monitor handler.
    // Placeholder to preserve COLOR statement call flow until monitor integration.
    (void)color;
}

void MON_TABV(std::uint8_t row_zero_based) {
    // TODO(asm-port): port MON_TABV monitor handler.
    // Placeholder to preserve VTAB statement call flow until monitor integration.
    (void)row_zero_based;
}

void MON_INPORT(std::uint8_t slot) {
    // TODO(asm-port): port MON_INPORT monitor handler.
    // Placeholder to preserve IN# statement call flow until monitor integration.
    (void)slot;
}

void MON_OUTPORT(std::uint8_t slot) {
    // TODO(asm-port): port MON_OUTPORT monitor handler.
    // Placeholder to preserve PR# statement call flow until monitor integration.
    (void)slot;
}

void MON_PLOT(std::uint8_t y, std::uint8_t x) {
    // TODO(asm-port): port MON_PLOT monitor handler.
    // Placeholder to preserve PLOT statement call flow until monitor integration.
    (void)y;
    (void)x;
}

void MON_HLINE(std::uint8_t y, std::uint8_t right, std::uint8_t left) {
    // TODO(asm-port): port MON_HLINE monitor handler.
    // Placeholder to preserve HLIN statement call flow until monitor integration.
    (void)y;
    (void)right;
    (void)left;
}

void MON_VLINE(std::uint8_t x, std::uint8_t top) {
    // TODO(asm-port): port MON_VLINE monitor handler.
    // Placeholder to preserve VLIN statement call flow until monitor integration.
    (void)x;
    (void)top;
}

void MON_HOME() {
    // TODO(asm-port): port MON_HOME monitor handler.
    // Placeholder to preserve HOME statement call flow until monitor integration.
}

void MON_SETTXT() {
    // TODO(asm-port): port MON_SETTXT monitor handler.
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst label SETTXT.
    // Sets full-screen text window; reads TXTSET soft-switch then calls SETWND.
}

void MON_SETGR() {
    // TODO(asm-port): port MON_SETGR monitor handler.
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst label SETGR.
    // Sets up lo-res graphics window; reads TXTCLR+MIXSET soft-switches, calls CLRTOP.
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

void HCOLOR() {
    // TODO(asm-port): port HCOLOR label range from Applesoft ROM.
    // Sets hi-res graphics color.
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

}  // namespace applesoft::asm_port