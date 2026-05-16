#include "core/applesoft_variables.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_handling.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_mathtbl.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_reason.hpp"
#include "core/asm_port_stack.hpp"
#include "core/asm_port_unfnc.hpp"
#include "core/io_ports.hpp"
#include "core/jump_table.hpp"

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

void AS_SYNERR();
extern std::uint8_t gJerErrorCode;
void MON_RESET2();
void MON_OLDBRK();
void MON_REGDSP();
void MON_COUT(std::uint8_t a); // Defined in asm_port_outdo.cpp

constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
  return static_cast<std::uint8_t>(lhs + rhs);
}
void AS_GETARY();
void AS_GETARY2();
void AS_FIND_ARRAY_ELEMENT();
std::uint16_t AS_MULTIPLY_SUBS_1(std::uint8_t multiplierHigh);
std::uint16_t AS_MULTIPLY_SUBSCRIPT(std::uint8_t descriptorOffset);
void AS_GIVAYF(std::int16_t value);
void AS_SNGFLT(std::uint8_t value);
void AS_FALSE();
void AS_TRUE();
void AS_ANDOP();
bool AS_ISLETC();
void AS_NAMOK();
void AS_NXDIM();
void AS_ARRAY();
void AS_MI1();
void AS_MI2();
void AS_CMPDONE();
void AS_NUMCMP();
void AS_PLOTFNS();
void AS_SYNCHR(std::uint8_t expected);
void AS_CHKNUM();
void AS_FRMNUM();
void AS_CHKCLS();
void AS_MAKINT();
void AS_CHKOPN();
std::uint16_t AS_PTRGET();
void AS_DATA();
void AS_FRMEVL();
void AS_STRCMP();
void AS_PARCHK();
void AS_STORE_FACDB_YX_ROUNDED();
void AS_ERRDIR();
std::int8_t AS_FCOMP(std::uint16_t argAddress);
void AS_FCOMP2();
void AS_FLOAT();
void AS_FLOAT_1(std::uint8_t exponent);
bool AS_CHKVAL(std::uint8_t savedValTyp);
std::uint8_t MON_SCRN(std::uint8_t row, std::uint8_t column);
std::uint8_t AS_FREFAC();
std::uint8_t AS_FRETMP(std::uint16_t descriptorAddress);
void AS_GARBAG();
std::int8_t CompareArgAndFacStrings();
void AS_GOTO();
void AS_NEWSTT();
void AS_PRINT_ERROR_LINNUM();
void MON_INPORT(std::uint8_t slot);
void MON_OUTPORT(std::uint8_t slot);
void MON_SETTXT();
void MON_HOME();
void AS_CAT();
void AS_CHKSTR();
void AS_CHKCOM();
std::uint8_t AS_GETBYT();
extern std::int8_t gNumericCompareResult;
extern bool gNumericCompareCarry;
extern std::uint8_t gFloatInput;
extern std::uint8_t gPendingErrorCode;

void SetTextPointer(std::uint16_t address) {
  variables().writeWord(ApplesoftVariables::ZP_AS_TXTPTR, address);
}

void ClearErrFlag() {
  variables().writeByte(ApplesoftVariables::ZP_AS_ERRFLG, 0);
}

void MarkDirectMode() {
  variables().writeByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_CURLIN + 1u), 0xffu);
}

std::uint8_t ReadZeroPageByte(std::uint8_t address) {
  return variables_const().readByte(address);
}

void WriteZeroPageByte(std::uint8_t address, std::uint8_t value) {
  variables().writeByte(address, value);
}

void WriteZeroPageWord(std::uint8_t address, std::uint16_t value) {
  variables().writeWord(address, value);
}

std::uint16_t ReadZeroPageWord(std::uint8_t address) {
  return variables_const().readWord(address);
}

std::uint8_t ReadProgramByte(std::uint16_t address) {
  // Program text lives in the same flat address space as zero-page variables;
  // ApplesoftVariables::readByte handles all address regions.
  return variables_const().readByte(address);
}

void WriteProgramByte(std::uint16_t address, std::uint8_t value) {
  variables().writeByte(address, value);
}

bool IsOnErr() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ERROR (inclusive) .. AS_L_ERROR_1 (exclusive)
  // Name normalization: helper name chosen for the inline AS_ERROR predicate.
  // `bit AS_ERRFLG` + `bpl` means ON ERR is active when AS_ERRFLG bit 7 is set.
  return (ReadZeroPageByte(ApplesoftVariables::ZP_AS_ERRFLG) & 0x80u) != 0u;
}

bool IsDirectMode() {
  return ReadZeroPageByte(static_cast<std::uint8_t>(
             ApplesoftVariables::ZP_AS_CURLIN + 1u)) == 0xffu;
}

void AS_NORMAL();
void AS_CRDO();
void AS_SCRTCH();
void AS_RESTART();

constexpr std::array<std::uint8_t, 29> kGenericAS_CHRGETImage = {
    0xe6, 0xb8, 0xd0, 0x02, 0xe6, 0xb9, 0xad, 0x60, 0xea, 0xc9,
    0x3a, 0xb0, 0x0a, 0xc9, 0x20, 0xf0, 0xef, 0x38, 0xe9, 0x30,
    0x38, 0xe9, 0xd0, 0x60, 0x80, 0x4f, 0xc7, 0x52, 0x58,
};

void AS_COLD_START();

void AS_GENERIC_END() {
  // AS_Label-only range in ROM: this address immediately falls into
  // AS_COLD_START.
  AS_COLD_START();
}

// Forward declaration: MON_COUT is defined in asm_port_outdo.cpp.
void MON_COUT(std::uint8_t value);

namespace {

void MON_SETNORM() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: SETNORM (inclusive) .. SETKBD (exclusive)
  // Name normalization: SETNORM -> MON_SETNORM (monitor label gets MON_
  // prefix).

  constexpr std::uint8_t kMON_INVFLG = ApplesoftVariables::ZP_MON_INVFLG;
  WriteZeroPageByte(kMON_INVFLG, 0xffu);
}

void MON_INIT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/display1.o65.lst
  // AS_Labels: INIT (inclusive) .. SETTXT (exclusive)
  // Name normalization: INIT -> MON_INIT (monitor label gets MON_ prefix).
  //
  // Falls through into SETTXT in ROM; modeled by explicit call.

  WriteZeroPageByte(ApplesoftVariables::ZP_MON_STATUS, 0u);
  (void)variables_const().readByte(IOPorts::ADDR_AS_SW_LORES);
  (void)variables_const().readByte(IOPorts::ADDR_AS_SW_LOWSCR);

  MON_SETTXT();
}

void MON_SETVID() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: SETVID (inclusive) .. OUTPORT (exclusive)
  // Name normalization: SETVID -> MON_SETVID (monitor label gets MON_ prefix).

  MON_OUTPORT(0u);
}

void MON_SETKBD() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: SETKBD (inclusive) .. INPORT (exclusive)
  // Name normalization: SETKBD -> MON_SETKBD (monitor label gets MON_ prefix).

  MON_INPORT(0u);
}

void MON_BELL_impl() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: BELL (inclusive) .. AS_RESTORE (exclusive)
  // Name normalization: BELL -> MON_BELL (monitor label gets MON_ prefix).

  constexpr std::uint8_t kBellChar = 0x87u;
  MON_COUT(kBellChar);
}

void MON_LFB60() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/math.o65.lst
  // AS_Labels: AS_LFB60 (inclusive) .. AS_LFB78 (exclusive)
  // Name normalization: AS_LFB60 -> MON_LFB60 (monitor label gets MON_ prefix).
  //
  // Clears the monitor text window, then writes the 9-byte "<APPLE ]["
  // banner to screen memory at $040E..$0416.

  static constexpr std::array<std::uint8_t, 9> kAS_LFB08 = {
      0x3cu,
      static_cast<std::uint8_t>('A' | 0x80u),
      static_cast<std::uint8_t>('P' | 0x80u),
      static_cast<std::uint8_t>('P' | 0x80u),
      static_cast<std::uint8_t>('AS_L' | 0x80u),
      static_cast<std::uint8_t>('E' | 0x80u),
      static_cast<std::uint8_t>(' ' | 0x80u),
      static_cast<std::uint8_t>(']' | 0x80u),
      static_cast<std::uint8_t>('[' | 0x80u),
  };
  constexpr std::uint16_t kBannerAddress = 0x040eu;

  MON_HOME();

  for (std::uint8_t y = 8u;; --y) {
    // AS_LFB65 copies the banner bytes in descending index order.
    variables().writeByte(static_cast<std::uint16_t>(kBannerAddress + y),
                          kAS_LFB08[y]);
    if (y == 0u) {
      break;
    }
  }
}

void MON_CROUT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: CROUT (inclusive) .. PRA1 (exclusive)
  // Name normalization: CROUT -> MON_CROUT (monitor label gets MON_ prefix).
  //
  // `lda #$8d` + `bne COUT` is an unconditional transfer to COUT with CR.
  constexpr std::uint8_t kCarriageReturn = 0x8du;
  MON_COUT(kCarriageReturn);
}

void MON_PRBYTE(std::uint8_t value) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // AS_Labels: PRBYTE (inclusive) .. COUT (exclusive)
  // Name normalization: PRBYTE -> MON_PRBYTE (monitor label gets MON_ prefix).

  const auto emitHexNibble = [](std::uint8_t nibble) -> std::uint8_t {
    // PRHEX/PRHEXZ: AS_ORA #"0"|$80; if >= ':' add 6 to reach 'A'..'F'.
    std::uint8_t ch = static_cast<std::uint8_t>((nibble & 0x0fu) | 0xb0u);
    if (ch >= 0xbau) {
      ch = static_cast<std::uint8_t>(ch + 0x06u);
    }
    return ch;
  };

  MON_COUT(emitHexNibble(static_cast<std::uint8_t>(value >> 4u)));
  MON_COUT(emitHexNibble(value));
}

} // namespace

void MON_BELL() { MON_BELL_impl(); }

std::int8_t MON_INSDS1() {
  // TODO(asm-port): Port monitor label INSDS1.
  return 0;
}

void MON_MON() {
  // TODO(asm-port): Port monitor label MON.
}

void MON_RESET2() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/debug.o65.lst
  // AS_Labels: RESET2 (inclusive) .. REGDSP (exclusive)
  // Name normalization: RESET2 -> MON_RESET2 (monitor label gets MON_ prefix).
  //
  // One-sentence behavior summary: initialize monitor I/O modes, validate
  // bootstrap sentinel bytes, then either jump through warm vectors or scan
  // ROM pages for a signature and jump to the matched page entrypoint.

  constexpr std::uint16_t kWarmVector =
      ApplesoftVariables::ADDR_MON_DEBUG_WARM_VECTOR;
  constexpr std::uint16_t kBootstrap =
      ApplesoftVariables::ADDR_MON_DEBUG_BOOTSTRAP;
  constexpr std::uint16_t kScanPageMirror =
      ApplesoftVariables::ADDR_MON_DEBUG_SCAN_PAGE;

  constexpr std::uint16_t kJumpToBasic =
      ApplesoftNG::ExternalJumpDispatcher::ADDR_AS_BASIC;
  constexpr std::uint16_t kJumpToBasic2 =
      ApplesoftNG::ExternalJumpDispatcher::ADDR_AS_BASIC2;
  constexpr std::uint8_t kSentinelCheckMask = 0xa5u;
  constexpr std::uint8_t kSentinelExpected = 0xe0u;

  // Signature bytes used by AS_LFAA6/AS_LFABA/AS_LFAC7 scan path.
  constexpr std::array<std::uint8_t, 7> kAS_LFB01 = {0x45u, 0x20u, 0xffu, 0x00u,
                                                     0xffu, 0x03u, 0xffu};
  constexpr std::array<std::uint8_t, 6> kAS_LFAFC = {0x00u, 0x00u, 0xfau,
                                                     0x59u, 0xe0u, 0x00u};

  // RESET2 prologue.
  MON_SETNORM();
  MON_INIT();
  MON_SETVID();
  MON_SETKBD();

  // Hardware-side effect reads routed through the I/O companion class.
  (void)variables_const().readByte(IOPorts::ADDR_SW_AN0);
  (void)variables_const().readByte(IOPorts::ADDR_SW_AN1);
  (void)variables_const().readByte(IOPorts::ADDR_SW_AN2);
  (void)variables_const().readByte(IOPorts::ADDR_SW_AN3);
  (void)variables_const().readByte(IOPorts::ADDR_ROM_AS_SIGNATURE);
  (void)variables_const().readByte(IOPorts::ADDR_AS_KEYBOARD_STROBE);

  MON_BELL_impl();

  const std::uint16_t warmVectorValue = variables_const().readWord(kWarmVector);
  const std::uint8_t warmVectorSentinel =
      variables_const().readByte(static_cast<std::uint16_t>(kWarmVector + 2u));

  if (static_cast<std::uint8_t>(ApplesoftVariables::highByte(warmVectorValue) ^
                                kSentinelCheckMask) == warmVectorSentinel) {
    if (warmVectorValue == kJumpToBasic) {
      variables().writeWord(kWarmVector, kJumpToBasic2);
      ApplesoftNG::ExternalJumpDispatcher::Jump(kJumpToBasic);
      return;
    }

    // AS_LFAA3 path: jump through warm vector ($03F2/$03F3).
    ApplesoftNG::ExternalJumpDispatcher::Jump(warmVectorValue);
    return;
  } else {
    // AS_LFAA6 path: install bootstrap bytes, then scan descending pages.
    MON_LFB60();

    // AS_LFAAB loop copies indices 5..1 from AS_LFAFC to $03F4..$03F0.
    for (std::uint8_t x = 5u; x != 0u; --x) {
      variables().writeByte(static_cast<std::uint16_t>(kBootstrap + x),
                            kAS_LFAFC[x]);
    }

    const auto slotAddress = [](int n) constexpr -> std::uint16_t {
      return static_cast<std::uint16_t>(IOPorts::ADDR_BASE |
                                        (static_cast<std::uint16_t>(n) << 8u));
    };

    // Unified pointer for $00/$01 pair used by AS_LFABA/AS_LFAC7 and jmp($00).
    for (std::uint16_t scanPtr = slotAddress(7); scanPtr > slotAddress(0);
         scanPtr = static_cast<std::uint16_t>(scanPtr - 0x0100u)) {

      const std::uint8_t page = ApplesoftVariables::highByte(scanPtr);
      variables().writeByte(kScanPageMirror, page);

      bool match = true;
      for (std::int8_t y = 7; y >= 0; y -= 2) {
        const std::uint8_t lhs =
            variables_const().readByte(static_cast<std::uint16_t>(
                scanPtr + static_cast<std::uint16_t>(y)));
        const std::uint8_t rhs = kAS_LFB01[static_cast<std::size_t>(y)];
        if (lhs != rhs) {
          match = false;
          break;
        }
      }

      if (!match) {
        continue;
      }

      ApplesoftNG::ExternalJumpDispatcher::Jump(scanPtr);
      return;
    }
    variables().writeWord(kWarmVector, kJumpToBasic2);
    ApplesoftNG::ExternalJumpDispatcher::Jump(kJumpToBasic);
    return;
  }
}

void MON_OLDBRK() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/debug.o65.lst
  // AS_Labels: OLDBRK (inclusive) .. RESET2 (exclusive)
  // Name normalization: OLDBRK -> MON_OLDBRK (monitor label gets MON_ prefix).
  //
  // OLDBRK disassembles at current PC, prints registers, then transfers to MON.

  (void)MON_INSDS1();
  MON_REGDSP();
  MON_MON();
}

void MON_REGDSP() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/debug.o65.lst
  // AS_Labels: REGDSP (inclusive) .. AS_LFB01 (exclusive)
  // Name normalization: REGDSP -> MON_REGDSP (monitor label gets MON_ prefix).
  //
  // Prints monitor register labels and the saved register byte values.

  constexpr std::uint8_t kMON_A4 = ApplesoftVariables::ZP_MON_A4;
  constexpr std::uint8_t kRegBase = ApplesoftVariables::ZP_MON_DEBUG_REG_A;
  constexpr std::uint8_t kSpace = static_cast<std::uint8_t>(' ' | 0x80u);
  constexpr std::uint8_t kEquals = static_cast<std::uint8_t>('=' | 0x80u);
  constexpr std::array<std::uint8_t, 5> kRTBL = {
      static_cast<std::uint8_t>('A' | 0x80u),
      static_cast<std::uint8_t>('X' | 0x80u),
      static_cast<std::uint8_t>('Y' | 0x80u),
      static_cast<std::uint8_t>('P' | 0x80u),
      static_cast<std::uint8_t>('S' | 0x80u),
  };

  MON_CROUT();

  // Unified pointer candidate from RGDSP1: $40/$41 points at the saved-register
  // block.
  const std::uint16_t regPointer = kRegBase;
  WriteZeroPageWord(kMON_A4, regPointer);

  for (std::size_t i = 0; i < kRTBL.size(); ++i) {
    MON_COUT(kSpace);
    MON_COUT(kRTBL[i]);
    MON_COUT(kEquals);
    const std::uint8_t regValue =
        variables_const().readByte(static_cast<std::uint16_t>(regPointer + i));
    MON_PRBYTE(regValue);
  }
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_COLD_START (inclusive) .. AS_CALL (exclusive)
// Name normalization: none (assembler label AS_COLD_START kept verbatim).
void AS_COLD_START() {
  constexpr std::uint8_t kAS_CURLIN_HI =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_CURLIN + 1u);
  constexpr std::uint8_t kJmpOpcode = 0x4cu;
  constexpr std::uint16_t kColdStartROM = 0x2128u;
  constexpr std::uint16_t kStroUTROM = 0x0b3au;
  constexpr std::uint16_t kRestartROM = 0x043cu;
  constexpr std::uint16_t kIqErrROM = 0x1199u;
  constexpr std::uint16_t kChrgetRuntime = 0x00b1u;
  constexpr std::uint16_t kProgramStart = 0x0800u;

  WriteZeroPageByte(kAS_CURLIN_HI, 0xffu);
  theStack().setStackPointer(0xfbu);

  WriteZeroPageWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_GOWARM + 1u),
      kColdStartROM);
  WriteZeroPageWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_GOSTROUT + 1u),
      kColdStartROM);

  AS_NORMAL();

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_GOWARM, kJmpOpcode);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_GOSTROUT, kJmpOpcode);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_JMPADRS, kJmpOpcode);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_USR, kJmpOpcode);
  WriteZeroPageWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_USR + 1u), kIqErrROM);

  // Preserve the ROM copy bug: the final random-seed byte is not copied.
  for (std::uint8_t x =
           static_cast<std::uint8_t>(kGenericAS_CHRGETImage.size() - 1u);
       x != 0u; --x) {
    variables()
        .pointer(kChrgetRuntime)
        .write(kGenericAS_CHRGETImage[x - 1u],
               static_cast<std::uint16_t>(x - 1u));
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_SPEEDZ, x);
  }

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_TRCFLG, 0u);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_SHIFT_SIGN_EXT, 0u);
  WriteZeroPageByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_LASTPT + 1u), 0u);
  theStack().pushByte(0u);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_DSCLEN, 3u);

  AS_CRDO();

  variables().writeByte(ApplesoftVariables::ADDR_AS_INPUT_BUFFER_MINUS_3, 1u);
  variables().writeByte(ApplesoftVariables::ADDR_AS_INPUT_BUFFER_MINUS_4, 1u);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_TEMPPT,
                    ApplesoftVariables::ZP_AS_TEMPST);

  // Unified RAM probe pointer lifted from AS_LINNUM low/high carry-chain in
  // ROM.
  std::uint16_t ramProbe = kProgramStart;
  while ((ramProbe & 0xff00u) < IOPorts::ADDR_BASE) {
    ramProbe = static_cast<std::uint16_t>(ramProbe + 0x0100u);
  }

  const std::uint16_t memoryTop =
      static_cast<std::uint16_t>(ramProbe & 0xf000u);
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_MEMSIZ, memoryTop);
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_FRETOP, memoryTop);

  WriteZeroPageWord(ApplesoftVariables::ZP_AS_TXTTAB, kProgramStart);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_LOCK, 0u);
  variables().pointer(kProgramStart).write(0u);
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_TXTTAB,
                    static_cast<std::uint16_t>(kProgramStart + 1u));

  AS_REASONState reasonState{};
  const std::uint16_t txttab =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_TXTTAB);
  const std::uint16_t fretop =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_FRETOP);
  reasonState.a = ApplesoftVariables::lowByte(txttab);
  reasonState.y = ApplesoftVariables::highByte(txttab);
  reasonState.fretopAS_Lo = ApplesoftVariables::lowByte(fretop);
  reasonState.fretopHi = ApplesoftVariables::highByte(fretop);
  const AS_REASONResult reasonResult = AS_REASON(reasonState);
  if (!reasonResult.ok) {
    return;
  }

  AS_SCRTCH();

  WriteZeroPageWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_GOSTROUT + 1u),
      kStroUTROM);
  WriteZeroPageWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_GOWARM + 1u),
      kRestartROM);

  // ROM uses JMP (AS_GOWARM+1), which resolves to AS_RESTART after vectors are
  // installed.
  AS_RESTART();
}

void AS_ERRDIR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ERRDIR (inclusive) .. AS_DEF (exclusive)
  // Name normalization: none (assembler label AS_ERRDIR kept verbatim).

  if (!IsDirectMode()) {
    return;
  }

  AS_ERROR(AS_ERR_ILLDIR);
}

void AS_UNDFNC() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_UNDFNC (inclusive) .. AS_DEF (exclusive)
  // Name normalization: none (assembler label AS_UNDFNC kept verbatim).

  AS_ERROR(AS_ERR_UNDEFFUNC);
}

void AS_BADNAM() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_BADNAM (inclusive) .. AS_NAMOK (exclusive)
  // Name normalization: none (assembler label AS_BADNAM kept verbatim).

  AS_SYNERR();
}

void AS_BASIC() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_BASIC (inclusive) .. AS_BASIC2 (exclusive)
  // Name normalization: none (assembler label AS_BASIC kept verbatim).

  AS_COLD_START();
}

void AS_BASIC2() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_BASIC2 (inclusive) .. AS_PTRGET4 (exclusive)
  // Name normalization: none (assembler label AS_BASIC2 kept verbatim).

  AS_RESTART();
}

void AS_GME() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GME (inclusive) .. AS_MULTIPLY_SUBSCRIPT (exclusive)
  // Name normalization: none (assembler label AS_GME kept verbatim).

  AS_MEMERR();
}

void AS_SUBERR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SUBERR (inclusive) .. AS_IQERR (exclusive)
  // Name normalization: none (assembler label AS_SUBERR kept verbatim).

  gJerErrorCode = AS_ERR_BADSUBS;
  AS_JER();
}

void AS_GSE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GSE (inclusive) .. AS_GME (exclusive)
  // Name normalization: none (assembler label AS_GSE kept verbatim).

  AS_SUBERR();
}

void AS_FAE_1() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FAE_1 (inclusive) .. AS_MULTIPLY_SUBSCRIPT (exclusive)
  // Name normalization: none (assembler label AS_FAE_1 kept verbatim).

  constexpr std::uint8_t kAS_NUMDIM = ApplesoftVariables::ZP_AS_NUMDIM;
  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_STRNG2 = ApplesoftVariables::ZP_AS_STRNG2;
  constexpr std::uint8_t kAS_RESULT = ApplesoftVariables::ZP_AS_RESULT;
  constexpr std::uint8_t kAS_VARNAM = ApplesoftVariables::ZP_AS_VARNAM;
  constexpr std::uint8_t kAS_ARYPNT = ApplesoftVariables::ZP_AS_ARYPNT;
  constexpr std::uint8_t kAS_VARPNT = ApplesoftVariables::ZP_AS_VARPNT;

  ProgramPointer descriptor{ReadZeroPageWord(kAS_LOWTR)};
  std::uint8_t descriptorY = 4u; // FIND_ARRAY_ELEMENT leaves descriptor[4]
                                 // (#dims) as the current slot.
  std::uint8_t remainingDims = ReadZeroPageByte(kAS_NUMDIM);

  while (remainingDims != 0u) {
    ++descriptorY; // Advance to current dimension high byte.

    // Subscripts are stacked as integer low/high pairs.
    const std::uint8_t subscriptLow = theStack().popByte();
    const std::uint8_t subscriptHigh = theStack().popByte();

    WriteZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 3u), subscriptLow);
    WriteZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 4u), subscriptHigh);

    // Bounds check: subscript must be strictly less than descriptor extent.
    const std::uint8_t dimHigh = descriptor.read(descriptorY);
    if (subscriptHigh > dimHigh) {
      AS_GSE();
      return;
    }

    if (subscriptHigh == dimHigh) {
      ++descriptorY; // Compare low bytes only when high bytes are equal.
      const std::uint8_t dimLow = descriptor.read(descriptorY);
      if (subscriptLow >= dimLow) {
        AS_GSE();
        return;
      }
    } else {
      ++descriptorY; // Match FAE_2 path: step to the dimension low-byte slot.
    }

    std::uint16_t runningOffset = ReadZeroPageWord(kAS_STRNG2);
    if (runningOffset != 0u) {
      runningOffset = AS_MULTIPLY_SUBSCRIPT(descriptorY);
    }

    runningOffset = static_cast<std::uint16_t>(
        runningOffset +
        ApplesoftVariables::makeWord(subscriptLow, subscriptHigh));
    WriteZeroPageWord(kAS_STRNG2, runningOffset);

    --remainingDims;
    WriteZeroPageByte(kAS_NUMDIM, remainingDims);
  }

  std::uint8_t elementSize = 5u;
  if ((ReadZeroPageByte(kAS_VARNAM) & 0x80u) != 0u) {
    --elementSize;
  }
  if ((ReadZeroPageByte(static_cast<std::uint8_t>(kAS_VARNAM + 1u)) & 0x80u) !=
      0u) {
    elementSize = static_cast<std::uint8_t>(elementSize - 2u);
  }

  WriteZeroPageByte(static_cast<std::uint8_t>(kAS_RESULT + 2u), elementSize);
  const std::uint16_t elementOffset = AS_MULTIPLY_SUBS_1(0u);
  const std::uint16_t varpnt =
      static_cast<std::uint16_t>(ReadZeroPageWord(kAS_ARYPNT) + elementOffset);
  WriteZeroPageWord(kAS_VARPNT, varpnt);
}

void AS_GETARY() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GETARY (inclusive) .. AS_GETARY2 (exclusive)
  // Name normalization: none (assembler label AS_GETARY kept verbatim).

  AS_GETARY2();
}

void AS_GETARY2() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GETARY2 (inclusive) .. AS_NEG32768 (exclusive)
  // Name normalization: none (assembler label AS_GETARY2 kept verbatim).

  const std::uint8_t numDim =
      ReadZeroPageByte(ApplesoftVariables::ZP_AS_NUMDIM);
  const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_AS_LOWTR)};
  const std::uint16_t arypntOffset =
      static_cast<std::uint16_t>(numDim * 2u) + 5u;
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_ARYPNT,
                    lowtr.advanced(arypntOffset).address);
}

void AS_C_ZERO() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_C_ZERO (inclusive) .. AS_MAKE_NEW_VARIABLE (exclusive)
  // Name normalization: none (assembler label AS_C_ZERO kept verbatim).

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_RESULT, kCZeroData[0]);
  WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_AS_RESULT, 1u),
                    kCZeroData[1]);
}

void AS_USE_OLD_ARRAY() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_USE_OLD_ARRAY (inclusive) .. AS_MAKE_NEW_ARRAY (exclusive)
  // Name normalization: none (assembler label AS_USE_OLD_ARRAY kept verbatim).

  if (ReadZeroPageByte(ApplesoftVariables::ZP_AS_DIMFLG) != 0u) {
    gJerErrorCode = AS_ERR_REDIMD;
    AS_JER();
    return;
  }

  if (ReadZeroPageByte(ApplesoftVariables::ZP_AS_SUBFLG) == 0u) {
    AS_GETARY();
    AS_FIND_ARRAY_ELEMENT();
  }
}

void AS_MAKE_NEW_ARRAY() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_MAKE_NEW_ARRAY (T:11b8, inclusive) .. AS_FIND_ARRAY_ELEMENT
  // (T:124b, exclusive) Name normalization: none (assembler label
  // AS_MAKE_NEW_ARRAY kept verbatim).
  //
  // Creates a new array entry at AS_LOWTR: writes header (name, size, numdim,
  // dim extents), allocates element space, zeroes it, and stores the array
  // byte-size in the descriptor. Falls through to AS_FIND_ARRAY_ELEMENT unless
  // called from AS_DIM (AS_DIMFLG != 0).

  // T:11b8 – lda AS_SUBFLG; bne AS_ERR_NODATA
  if (ReadZeroPageByte(ApplesoftVariables::ZP_AS_SUBFLG) != 0u) {
    AS_ERROR(AS_ERR_NODATA);
    return;
  }

  // T:11c1 – jsr AS_GETARY: sets AS_ARYPNT = AS_LOWTR + 5 + 2*AS_NUMDIM (first
  // element address)
  AS_GETARY();

  // T:11c4 – jsr AS_REASON: ensure array header fits below AS_FRETOP
  {
    const std::uint16_t arypnt =
        ReadZeroPageWord(ApplesoftVariables::ZP_AS_ARYPNT);
    const std::uint16_t fretop =
        ReadZeroPageWord(ApplesoftVariables::ZP_AS_FRETOP);
    AS_REASONState rs{};
    rs.a = static_cast<std::uint8_t>(arypnt & 0xffu);
    rs.y = static_cast<std::uint8_t>(arypnt >> 8u);
    rs.fretopAS_Lo = static_cast<std::uint8_t>(fretop & 0xffu);
    rs.fretopHi = static_cast<std::uint8_t>(fretop >> 8u);
    if (!AS_REASON(rs).ok)
      return;
  }

  // T:11c7 – lda #0; tay; sta AS_STRNG2+1; ldx #5  (seed element-size
  // accumulator)
  WriteZeroPageByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_STRNG2 + 1u), 0u);
  std::uint8_t elemSize = 5u; // X in asm: float default

  // T:11cd – Y=0: write AS_VARNAM byte to descriptor[0]; bit 7 → integer, dex
  const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_AS_LOWTR)};
  const std::uint8_t varnam0 =
      ReadZeroPageByte(ApplesoftVariables::ZP_AS_VARNAM);
  lowtr.write(varnam0, 0u);
  if ((varnam0 & 0x80u) != 0u) {
    --elemSize;
  } // integer: 5→4

  // T:11d5 – Y=1: write AS_VARNAM+1 to descriptor[1]; bit 7 → integer/string,
  // dex dex
  const std::uint8_t varnam1 = ReadZeroPageByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_VARNAM + 1u));
  lowtr.write(varnam1, 1u);
  if ((varnam1 & 0x80u) != 0u) {
    elemSize -= 2u;
  } // integer→2, string→3, float→5

  // T:11de – stx AS_STRNG2: seed running product with element size
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_STRNG2, elemSize);

  // T:11e0 – Y=4: write AS_NUMDIM to descriptor[4]  (three iny's advance Y past
  // size slots)
  const std::uint8_t numDim =
      ReadZeroPageByte(ApplesoftVariables::ZP_AS_NUMDIM);
  lowtr.write(numDim, 4u);

  // T:11e7 – dimension loop: write each dim to descriptor and accumulate total
  // bytes. Y starts at 4 (AS_NUMDIM slot); each iteration writes 2 bytes and
  // advances Y by 2.
  std::uint8_t descriptorY = 4u;
  std::uint8_t remainingDims = numDim;
  while (remainingDims != 0u) {
    // T:11e7 – ldx #11; lda #0; bit AS_DIMFLG; bvc use_default
    std::uint8_t dimAS_Lo = 11u; // default: 11 elements (indices 0..10)
    std::uint8_t dimHi = 0u;

    // bit AS_DIMFLG: V flag (bit 6) set → explicit dimension was pushed on 6502
    // stack.
    if ((ReadZeroPageByte(ApplesoftVariables::ZP_AS_DIMFLG) & 0x40u) != 0u) {
      // T:11ef – pla (raw dim_lo); clc; adc #1; tax
      const std::uint8_t rawAS_Lo = theStack().popByte();
      const std::uint16_t lo16 = static_cast<std::uint16_t>(rawAS_Lo) + 1u;
      dimAS_Lo = static_cast<std::uint8_t>(lo16 & 0xffu);
      const std::uint8_t carry = static_cast<std::uint8_t>(lo16 >> 8u);
      // T:11f4 – pla (dim_hi); adc #0 (carry from low add)
      dimHi = static_cast<std::uint8_t>(theStack().popByte() + carry);
    }

    // T:11f7 – iny; sta (AS_LOWTR),Y [dim_hi]; iny; txa; sta (AS_LOWTR),Y
    // [dim_lo]
    ++descriptorY;
    lowtr.write(dimHi, descriptorY);
    ++descriptorY;
    lowtr.write(dimAS_Lo, descriptorY);

    // T:11fe – jsr AS_MULTIPLY_SUBSCRIPT: running_product = AS_STRNG2 *
    // dim_count AS_STRNG2 must be set before the call (done above / updated
    // each iteration).
    const std::uint16_t product = AS_MULTIPLY_SUBSCRIPT(descriptorY);
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_STRNG2,
                      static_cast<std::uint8_t>(product & 0xffu));
    WriteZeroPageByte(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_STRNG2 + 1u),
        static_cast<std::uint8_t>(product >> 8u));

    // T:1205 – ldy AS_INDEX: restore Y (AS_MULTIPLY_SUBSCRIPT saved descriptorY
    // there)
    descriptorY = ReadZeroPageByte(ApplesoftVariables::ZP_AS_INDEX);

    // T:1207 – dec AS_NUMDIM; bne loop
    --remainingDims;
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_NUMDIM, remainingDims);
  }

  // T:120b – compute endAddr = AS_ARYPNT + totalBytes; both overflow checks
  // collapse to one.
  const std::uint16_t totalBytes =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_STRNG2);
  const std::uint16_t arypnt =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_ARYPNT);
  const std::uint32_t endAddr32 = static_cast<std::uint32_t>(arypnt) +
                                  static_cast<std::uint32_t>(totalBytes);
  if (endAddr32 > 0xffffu) {
    // Covers asm: bcs AS_GME (high-byte overflow) and iny;beq AS_GME
    // (wrap-to-zero).
    AS_GME();
    return;
  }
  const std::uint16_t endAddr = static_cast<std::uint16_t>(endAddr32);

  // T:121a – jsr AS_REASON: ensure end address fits below AS_FRETOP; result →
  // new AS_STREND.
  std::uint16_t strend;
  {
    const std::uint16_t fretop =
        ReadZeroPageWord(ApplesoftVariables::ZP_AS_FRETOP);
    AS_REASONState rs{};
    rs.a = static_cast<std::uint8_t>(endAddr & 0xffu);
    rs.y = static_cast<std::uint8_t>(endAddr >> 8u);
    rs.fretopAS_Lo = static_cast<std::uint8_t>(fretop & 0xffu);
    rs.fretopHi = static_cast<std::uint8_t>(fretop >> 8u);
    const AS_REASONResult rr = AS_REASON(rs);
    if (!rr.ok)
      return;
    strend = static_cast<std::uint16_t>(static_cast<std::uint16_t>(rr.y) << 8u |
                                        static_cast<std::uint16_t>(rr.a));
  }

  // T:121d – sta AS_STREND; sty AS_STREND+1
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_STREND, strend);

  // T:1221–1232 – zero element region [AS_ARYPNT .. AS_STREND)
  // Asm uses a page-by-page backward sweep; C++ equivalent byte loop has same
  // effect.
  for (std::uint16_t addr = arypnt; addr != strend;
       addr = static_cast<std::uint16_t>(addr + 1u)) {
    WriteProgramByte(addr, 0u);
  }

  // T:1234 – inc AS_ARYPNT+1: after zeroing the asm modified AS_ARYPNT+1 for
  // the loop; restoring AS_ARYPNT to elemStart is equivalent here.
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_ARYPNT, arypnt);

  // T:1236–1244 – sec; AS_STREND - AS_LOWTR → descriptor[2..3] (offset to next
  // array entry)
  const std::uint16_t lowtrAddr =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_LOWTR);
  const std::uint16_t arraySize =
      static_cast<std::uint16_t>(strend - lowtrAddr);
  lowtr.write(static_cast<std::uint8_t>(arraySize & 0xffu), 2u);
  lowtr.write(static_cast<std::uint8_t>(arraySize >> 8u), 3u);

  // T:1246 – lda AS_DIMFLG; bne AS_RTS_9: AS_DIM statement is done; otherwise
  // find element.
  if (ReadZeroPageByte(ApplesoftVariables::ZP_AS_DIMFLG) != 0u) {
    return;
  }
  // T:124a – iny (Y=4 for AS_FIND_ARRAY_ELEMENT descriptor[4] = AS_NUMDIM);
  // fall through.
  AS_FIND_ARRAY_ELEMENT();
}

void AS_FIND_ARRAY_ELEMENT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FIND_ARRAY_ELEMENT (inclusive) .. AS_FAE_1 (exclusive)
  // Name normalization: none (assembler label AS_FIND_ARRAY_ELEMENT kept
  // verbatim). AS_LOWTR is the base pointer to the current array descriptor;
  // descriptor[4] stores #dims.

  const ProgramPointer descriptor{
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_LOWTR)};
  const std::uint8_t numDims = descriptor.read(4u);

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_NUMDIM, numDims);
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_STRNG2, 0u);

  // The source slice falls through directly into AS_FAE_1.
  AS_FAE_1();
}

std::uint16_t AS_MULTIPLY_SUBSCRIPT(std::uint8_t descriptorOffset) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_MULTIPLY_SUBSCRIPT (inclusive) .. AS_MULTIPLY_SUBS_1
  // (exclusive) Name normalization: none (assembler label AS_MULTIPLY_SUBSCRIPT
  // kept verbatim). AS_Load the 16-bit array-dimension multiplier from the
  // AS_LOWTR descriptor pointer.

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_INDEX, descriptorOffset);

  const ProgramPointer descriptor{
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_LOWTR)};
  WriteZeroPageByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_RESULT + 2u),
      descriptor.read(descriptorOffset));

  return AS_MULTIPLY_SUBS_1(
      descriptor.read(static_cast<std::uint16_t>(descriptorOffset - 1u)));
}

std::uint16_t AS_MULTIPLY_SUBS_1(std::uint8_t multiplierHigh) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_MULTIPLY_SUBS_1 (inclusive) .. AS_FRE (exclusive)
  // Name normalization: none (assembler label AS_MULTIPLY_SUBS_1 kept
  // verbatim). AS_STRNG2 is dual-use elsewhere, but in this slice it is the
  // 16-bit multiplicand.

  WriteZeroPageByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_RESULT + 3u),
      multiplierHigh);
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_INDX, 16u);

  const std::uint16_t multiplier =
      ApplesoftVariables::makeWord(ReadZeroPageByte(static_cast<std::uint8_t>(
                                       ApplesoftVariables::ZP_AS_RESULT + 2u)),
                                   multiplierHigh);

  std::uint16_t multiplicand =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_STRNG2);
  std::uint16_t product = 0u;

  for (std::uint8_t bitsRemaining = 16u; bitsRemaining > 0u; --bitsRemaining) {
    if ((product & 0x8000u) != 0u) {
      AS_GME();
      return product;
    }

    product = static_cast<std::uint16_t>(product << 1u);

    const bool nextBitSet = (multiplicand & 0x8000u) != 0u;
    multiplicand = static_cast<std::uint16_t>(multiplicand << 1u);
    WriteZeroPageWord(ApplesoftVariables::ZP_AS_STRNG2, multiplicand);

    if (!nextBitSet) {
      continue;
    }

    if (product > static_cast<std::uint16_t>(0xffffu - multiplier)) {
      AS_GME();
      return product;
    }

    product = static_cast<std::uint16_t>(product + multiplier);
  }

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_INDX, 0u);
  return product;
}

void AS_SNGFLT(std::uint8_t value) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SNGFLT (inclusive) .. AS_ERRDIR (exclusive)
  // Name normalization: none (assembler label AS_SNGFLT kept verbatim).

  AS_GIVAYF(static_cast<std::int16_t>(value));
}

void AS_OR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_OR (inclusive) .. AS_ANDOP (exclusive)
  // Name normalization: none (assembler label AS_OR kept verbatim).

  constexpr std::uint8_t kAS_ARG = ApplesoftVariables::ZP_AS_ARG;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;

  if ((ReadZeroPageByte(kAS_ARG) | ReadZeroPageByte(kAS_FAC)) != 0u) {
    AS_TRUE();
    return;
  }

  // Fall-through in ROM from AS_OR to AS_ANDOP.
  AS_ANDOP();
}

void AS_ANDOP() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ANDOP (inclusive) .. AS_FALSE (exclusive)
  // Name normalization: none (assembler label AS_ANDOP kept verbatim).

  constexpr std::uint8_t kAS_ARG = ApplesoftVariables::ZP_AS_ARG;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;

  if (ReadZeroPageByte(kAS_ARG) == 0u || ReadZeroPageByte(kAS_FAC) == 0u) {
    AS_FALSE();
    return;
  }

  // Fall-through in ROM from AS_ANDOP to AS_TRUE.
  AS_TRUE();
}

void AS_FALSE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FALSE (inclusive) .. AS_TRUE (exclusive)
  // Name normalization: none (assembler label AS_FALSE kept verbatim).

  AS_SNGFLT(0u);
}

void AS_TRUE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_TRUE (inclusive) .. AS_RELOPS (exclusive)
  // Name normalization: none (assembler label AS_TRUE kept verbatim).

  AS_SNGFLT(1u);
}

void AS_SET_VARPNT_AND_YA() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SET_VARPNT_AND_YA (inclusive) .. AS_GETARY (exclusive)
  // Name normalization: none (assembler label AS_SET_VARPNT_AND_YA kept
  // verbatim).

  const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_AS_LOWTR)};
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_VARPNT,
                    lowtr.advanced(2u).address); // AS_VARPNT
}

void AS_MAKE_NEW_VARIABLE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: MAKE_NEW_VARIABLE (inclusive) .. GETARY (exclusive)
  // Name normalization: none (assembler label MAKE_NEW_VARIABLE prefixed with
  // AS_ in C++).
  //
  // Create a new simple variable entry by shifting the array region up seven
  // bytes, then writing the two-byte name and a zero-initialized five-byte
  // value.

  constexpr std::uint8_t kAS_ARYTAB = ApplesoftVariables::ZP_AS_ARYTAB;
  constexpr std::uint8_t kAS_STREND = ApplesoftVariables::ZP_AS_STREND;
  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;
  constexpr std::uint8_t kAS_HIGHTR = ApplesoftVariables::ZP_AS_HIGHTR;
  constexpr std::uint8_t kAS_ARYPNT = ApplesoftVariables::ZP_AS_ARYPNT;
  constexpr std::uint8_t kAS_VARNAM = ApplesoftVariables::ZP_AS_VARNAM;
  constexpr std::uint8_t kAS_VARPNT = ApplesoftVariables::ZP_AS_VARPNT;

  const std::uint16_t arytab = ReadZeroPageWord(kAS_ARYTAB);
  const std::uint16_t strend = ReadZeroPageWord(kAS_STREND);

  WriteZeroPageWord(kAS_LOWTR, arytab);
  WriteZeroPageWord(kAS_HIGHTR, strend);

  const std::uint16_t newStrend = static_cast<std::uint16_t>(strend + 7u);
  WriteZeroPageWord(kAS_ARYPNT, newStrend);

  {
    const std::uint16_t fretop =
        ReadZeroPageWord(ApplesoftVariables::ZP_AS_FRETOP);
    AS_REASONState rs{};
    rs.a = static_cast<std::uint8_t>(newStrend & 0xffu);
    rs.y = static_cast<std::uint8_t>(newStrend >> 8u);
    rs.fretopAS_Lo = static_cast<std::uint8_t>(fretop & 0xffu);
    rs.fretopHi = static_cast<std::uint8_t>(fretop >> 8u);
    const AS_REASONResult rr = AS_REASON(rs);
    if (!rr.ok) {
      return;
    }
    WriteZeroPageWord(kAS_STREND, static_cast<std::uint16_t>(
                                      static_cast<std::uint16_t>(rr.y) << 8u |
                                      static_cast<std::uint16_t>(rr.a)));
  }

  const std::uint16_t newArytab = static_cast<std::uint16_t>(arytab + 7u);
  if (strend > arytab) {
    for (std::uint16_t source = strend; source != arytab;
         source = static_cast<std::uint16_t>(source - 1u)) {
      const std::uint16_t from = static_cast<std::uint16_t>(source - 1u);
      WriteProgramByte(static_cast<std::uint16_t>(from + 7u),
                       ReadProgramByte(from));
    }
  }

  WriteZeroPageWord(kAS_ARYTAB, newArytab);

  auto variableRecord = variables().pointer(arytab);
  variableRecord.write(ReadZeroPageByte(kAS_VARNAM), 0u);
  variableRecord.write(
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_VARNAM + 1u)), 1u);
  for (std::uint16_t offset = 2u; offset < 7u; ++offset) {
    variableRecord.write(0u, offset);
  }

  WriteZeroPageWord(kAS_VARPNT, static_cast<std::uint16_t>(arytab + 2u));
}

void AS_NAME_NOT_FOUND() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_NAME_NOT_FOUND (inclusive) .. AS_C_ZERO (exclusive)
  // Name normalization: none (assembler label AS_NAME_NOT_FOUND kept verbatim).
  //
  // Variable not found: check context via return address to decide between
  // returning a zero constant or creating a new variable entry.
  //
  // Original ROM logic:
  // 3443 T:1087  68        pla  ; pull return address low byte
  // 3444 T:1088  48        pha  ; push it back
  // 3445 T:1089  c9 d7     cmp #<FRM_VARIABLE_CALL
  // 3446 T:108b  d0 0f     bne MAKE_NEW_VARIABLE
  // 3447 T:108d  ba        tsx
  // 3448 T:108e  bd 02 01  lda STACK+2,X ; peek return address high byte
  // 3449 T:1091  c9 0e     cmp #>FRM_VARIABLE_CALL
  // 3450 T:1093  d0 07     bne MAKE_NEW_VARIABLE
  // 3451 T:1095  a9 9a     lda #<C_ZERO
  // 3452 T:1097  a0 10     ldy #>C_ZERO
  // 3453 T:1099  60        rts

  constexpr std::uint16_t kFRM_VARIABLE_CALL = 0x0ed7u; // Derived from .sym

  if (theStack().probeIsCalledFrom(kFRM_VARIABLE_CALL)) {
    AS_C_ZERO();
    return;
  }

  AS_MAKE_NEW_VARIABLE();
}

void AS_PTRGET3() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PTRGET3 (inclusive) .. AS_BADNAM (exclusive)
  // Name normalization: none (assembler label AS_PTRGET3 kept verbatim).

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_VARNAM,
                    AS_CHRGOT()); // AS_VARNAM low byte
  AS_CHRGOT();
  if (!AS_ISLETC()) {
    AS_BADNAM();
    return;
  }

  AS_NAMOK();
}

void AS_PTRGET2() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PTRGET2 (inclusive) .. AS_PTRGET3 (exclusive)
  // Name normalization: none (assembler label AS_PTRGET2 kept verbatim).

  AS_PTRGET3();
}

std::uint16_t AS_PTRGET() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PTRGET (inclusive) .. AS_PTRGET2 (exclusive)
  // Name normalization: none (assembler label AS_PTRGET kept verbatim).

  AS_CHRGOT();
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_DIMFLG, 0u); // AS_DIMFLG
  AS_PTRGET3();
  return ReadZeroPageWord(ApplesoftVariables::ZP_AS_VARPNT); // AS_VARPNT
}

void AS_ARRAY() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ARRAY (inclusive) .. AS_MAKE_NEW_ARRAY (exclusive)
  // Name normalization: none (assembler label AS_ARRAY kept verbatim).
  // Parse subscripts (if present), then search AS_ARYTAB for matching array
  // name.

  constexpr std::uint8_t kAS_SUBFLG = ApplesoftVariables::ZP_AS_SUBFLG;
  constexpr std::uint8_t kAS_DIMFLG = ApplesoftVariables::ZP_AS_DIMFLG;
  constexpr std::uint8_t kAS_VALTYP = ApplesoftVariables::ZP_AS_VALTYP;
  constexpr std::uint8_t kAS_VALTYP_PLUS_1 =
      ApplesoftVariables::ZP_AS_VALTYP_PLUS_1;
  constexpr std::uint8_t kAS_NUMDIM = ApplesoftVariables::ZP_AS_NUMDIM;
  constexpr std::uint8_t kAS_VARNAM = ApplesoftVariables::ZP_AS_VARNAM;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_ARYTAB = ApplesoftVariables::ZP_AS_ARYTAB;
  constexpr std::uint8_t kAS_STREND = ApplesoftVariables::ZP_AS_STREND;
  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;

  if (ReadZeroPageByte(kAS_SUBFLG) == 0u) {
    const std::uint8_t dimflgOrInteger = static_cast<std::uint8_t>(
        ReadZeroPageByte(kAS_DIMFLG) | ReadZeroPageByte(kAS_VALTYP_PLUS_1));
    theStack().pushByte(dimflgOrInteger);
    theStack().pushByte(ReadZeroPageByte(kAS_VALTYP));

    std::uint8_t dimensions = 0u;
    for (;;) {
      // Save loop state and variable name while AS_MAKINT evaluates next
      // subscript.
      theStack().pushByte(dimensions);
      theStack().pushByte(
          ReadZeroPageByte(static_cast<std::uint8_t>(kAS_VARNAM + 1u)));
      theStack().pushByte(ReadZeroPageByte(kAS_VARNAM));

      AS_MAKINT();

      WriteZeroPageByte(kAS_VARNAM, theStack().popByte());
      WriteZeroPageByte(static_cast<std::uint8_t>(kAS_VARNAM + 1u),
                        theStack().popByte());
      dimensions = theStack().popByte();

      // Replace saved (AS_VALTYP, AS_DIMFLG|AS_VALTYP+1) with subscript value
      // (AS_FAC+3/+4).
      const std::uint8_t x = theStack().readStackPointer();
      const std::uint16_t stackPlus2 =
          static_cast<std::uint16_t>(0x0100u + add_u8(x, 2u));
      const std::uint16_t stackPlus1 =
          static_cast<std::uint16_t>(0x0100u + add_u8(x, 1u));
      theStack().pushByte(ReadProgramByte(stackPlus2));
      theStack().pushByte(ReadProgramByte(stackPlus1));
      WriteProgramByte(stackPlus2, ReadZeroPageByte(static_cast<std::uint8_t>(
                                       kAS_FAC + 3u)));
      WriteProgramByte(stackPlus1, ReadZeroPageByte(static_cast<std::uint8_t>(
                                       kAS_FAC + 4u)));

      ++dimensions;
      if (AS_CHRGOT() == static_cast<std::uint8_t>(',')) {
        continue;
      }

      WriteZeroPageByte(kAS_NUMDIM, dimensions);
      AS_CHKCLS();

      const std::uint8_t restoredValTyp = theStack().popByte();
      const std::uint8_t restoredValTypPlus1Dim = theStack().popByte();
      WriteZeroPageByte(kAS_VALTYP, restoredValTyp);
      WriteZeroPageByte(kAS_VALTYP_PLUS_1, restoredValTypPlus1Dim);
      WriteZeroPageByte(kAS_DIMFLG, static_cast<std::uint8_t>(
                                        restoredValTypPlus1Dim & 0x7fu));
      break;
    }
  }

  std::uint16_t lowtr = ReadZeroPageWord(kAS_ARYTAB);
  const std::uint16_t strend = ReadZeroPageWord(kAS_STREND);
  const std::uint8_t varnamLo = ReadZeroPageByte(kAS_VARNAM);
  const std::uint8_t varnamHi =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_VARNAM + 1u));

  for (;;) {
    // AS_LOWTR is a 16-bit running pointer through array descriptors.
    WriteZeroPageWord(kAS_LOWTR, lowtr);

    if (lowtr == strend) {
      AS_MAKE_NEW_ARRAY();
      return;
    }

    const ProgramPointer descriptor{lowtr};
    if (descriptor.read(0u) == varnamLo && descriptor.read(1u) == varnamHi) {
      AS_USE_OLD_ARRAY();
      return;
    }

    const std::uint16_t offsetToNext =
        ApplesoftVariables::makeWord(descriptor.read(2u), descriptor.read(3u));
    const std::uint32_t next = static_cast<std::uint32_t>(lowtr) +
                               static_cast<std::uint32_t>(offsetToNext);
    if (next > 0xffffu) {
      AS_SUBERR();
      return;
    }

    lowtr = static_cast<std::uint16_t>(next);
  }
}

void AS_DIM() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_DIM (inclusive) .. AS_PTRGET (exclusive)
  // Name normalization: none (assembler label AS_DIM kept verbatim).

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_DIMFLG,
                    1u); // AS_DIMFLG non-zero when called from AS_DIM.
  AS_PTRGET2();

  if (AS_CHRGOT() != 0u) {
    AS_NXDIM();
  }
}

void AS_NXDIM() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_NXDIM (inclusive) .. AS_DIM (exclusive)
  // Name normalization: none (assembler label AS_NXDIM kept verbatim).

  AS_CHKCOM();
  AS_DIM();
}

void AS_PTRGET4() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PTRGET4 (inclusive) .. AS_ISLETC (exclusive)
  // Name normalization: none (assembler label AS_PTRGET4 kept verbatim).

  std::uint8_t current = AS_CHRGET();
  std::uint8_t secondChar = 0u;

  if ((current >= '0') && (current <= '9')) {
    secondChar = current;
    do {
      current = AS_CHRGET();
    } while ((current >= '0') && (current <= '9'));
  } else {
    if (AS_ISLETC()) {
      secondChar = current;
      do {
        current = AS_CHRGET();
      } while (((current >= '0') && (current <= '9')) || AS_ISLETC());
    }
  }

  if (current == static_cast<std::uint8_t>('$')) {
    WriteZeroPageByte(ApplesoftVariables::ZP_AS_VALTYP,
                      0xffu); // AS_VALTYP string
    current = AS_CHRGET();
  } else if (current == static_cast<std::uint8_t>('%')) {
    if ((ReadZeroPageByte(ApplesoftVariables::ZP_AS_SUBFLG) & 0x80u) != 0u) {
      AS_BADNAM();
      return;
    }

    WriteZeroPageByte(ApplesoftVariables::ZP_AS_VALTYP_PLUS_1,
                      0x80u); // integer mode
    WriteZeroPageByte(
        ApplesoftVariables::ZP_AS_VARNAM,
        static_cast<std::uint8_t>(
            ReadZeroPageByte(ApplesoftVariables::ZP_AS_VARNAM) | 0x80u));
    secondChar = static_cast<std::uint8_t>(secondChar | 0x80u);
    current = AS_CHRGET();
  }

  WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_AS_VARNAM, 1u),
                    secondChar); // AS_VARNAM+1

  const std::uint8_t subflg =
      ReadZeroPageByte(ApplesoftVariables::ZP_AS_SUBFLG);
  if (subflg == 0u && current == static_cast<std::uint8_t>('(')) {
    AS_ARRAY();
    return;
  }

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_SUBFLG, 0u); // clear AS_SUBFLG
  AS_NAME_NOT_FOUND();
}

void SetPendingErrorCode(std::uint8_t errorCode) {
  gPendingErrorCode = errorCode;
}

void AS_NUMCMP() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_NUMCMP (inclusive) .. AS_CMPDONE (exclusive)
  // Name normalization: none (assembler label AS_NUMCMP kept verbatim).

  // ROM reaches AS_CMPDONE with C set only when compare result was negative.
  gNumericCompareCarry = (gNumericCompareResult < 0);
  AS_CMPDONE();
}

void AS_CMPDONE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_CMPDONE (inclusive) .. AS_PDL (exclusive)
  // Name normalization: none (assembler label AS_CMPDONE kept verbatim).

  constexpr std::uint8_t kAS_CPRMASK = ApplesoftVariables::ZP_AS_CPRMASK;

  std::int16_t x = static_cast<std::int16_t>(gNumericCompareResult) + 1;
  if (x < 0) {
    x = 0;
  }

  std::uint8_t a = static_cast<std::uint8_t>(x & 0xff);
  a = static_cast<std::uint8_t>((a << 1) | (gNumericCompareCarry ? 1u : 0u));
  a = static_cast<std::uint8_t>(a & ReadZeroPageByte(kAS_CPRMASK));

  gFloatInput = (a == 0u) ? 0u : 1u;
  AS_SNGFLT(gFloatInput);
  AS_FLOAT();
}

void AS_AYINT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_AYINT (inclusive) .. AS_MI1 (exclusive)
  // Name normalization: none (assembler label AS_AYINT kept verbatim).

  if (ReadZeroPageByte(ApplesoftVariables::ZP_AS_FAC) < 0x90u) {
    AS_MI2();
    return;
  }

  AS_NEG32768();
  if (AS_FCOMP(0x0062u) != 0) {
    AS_MI1();
    return;
  }

  AS_MI2();
}

void AS_HANDLERR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_HANDLERR (inclusive) .. AS_RESUME (exclusive)
  // Name normalization: none (assembler label AS_HANDLERR kept verbatim).
  constexpr std::uint8_t kAS_ERRNUM = ApplesoftVariables::ZP_AS_ERRNUM;
  constexpr std::uint8_t kAS_ERRLIN = ApplesoftVariables::ZP_AS_ERRLIN;
  constexpr std::uint8_t kAS_ERRPOS = ApplesoftVariables::ZP_AS_ERRPOS;
  constexpr std::uint8_t kAS_ERRSTK = ApplesoftVariables::ZP_AS_ERRSTK;
  constexpr std::uint8_t kAS_TXTPSV = ApplesoftVariables::ZP_AS_TXTPSV;
  constexpr std::uint8_t kAS_CURLSV = ApplesoftVariables::ZP_AS_CURLSV;
  constexpr std::uint8_t kAS_REMSTK = ApplesoftVariables::ZP_AS_REMSTK;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_OLDTEXT = ApplesoftVariables::ZP_AS_OLDTEXT;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;

  WriteZeroPageByte(kAS_ERRNUM, gPendingErrorCode);
  WriteZeroPageByte(kAS_ERRSTK, ReadZeroPageByte(kAS_REMSTK));

  WriteZeroPageWord(kAS_ERRLIN, ReadZeroPageWord(kAS_CURLIN));
  WriteZeroPageWord(kAS_ERRPOS, ReadZeroPageWord(kAS_OLDTEXT));

  WriteZeroPageWord(kAS_TXTPTR, ReadZeroPageWord(kAS_TXTPSV));
  WriteZeroPageWord(kAS_CURLIN, ReadZeroPageWord(kAS_CURLSV));

  AS_CHRGOT();
  AS_GOTO();
  AS_NEWSTT();
}

void AS_SCREEN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SCREEN (inclusive) .. AS_UNARY (exclusive)
  // Name normalization: none (assembler label AS_SCREEN kept verbatim).

  constexpr std::uint8_t kAS_FIRST = ApplesoftVariables::ZP_AS_FIRST;

  AS_CHRGET();
  AS_PLOTFNS();

  // AS_PLOTFNS returns row in X and column in AS_FIRST in ROM.
  const std::uint8_t row = ReadZeroPageByte(kAS_FIRST);
  const std::uint8_t column = ReadZeroPageByte(kAS_FIRST);
  const std::uint8_t color = MON_SCRN(row, column);

  AS_SNGFLT(color);
  AS_SYNCHR(static_cast<std::uint8_t>(')'));
}

void AS_UNARY() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_UNARY (inclusive) .. AS_OR (exclusive)
  // Name normalization: none (assembler label AS_UNARY kept verbatim).
  //
  // Dispatch a built-in unary function or string function identified by
  // the current token.  String functions (AS_LEFT$, RIGHT$, MID$) parse two
  // arguments; all other functions parse one argument via AS_PARCHK.

  constexpr std::uint8_t kAS_TOKEN_SCRN = 0xd7u;
  constexpr std::uint8_t kAS_TOKEN_SGN = 0xd2u;
  constexpr std::uint8_t kAS_TOKEN_LEFTSTR = 0xe8u;

  const std::uint8_t token = AS_CHRGOT();

  if (token == kAS_TOKEN_SCRN) {
    // ROM branches back to AS_SCREEN for SCRN(.
    AS_SCREEN();
    return;
  }

  // ASL: double the token (8-bit) as the AS_UNFNC table index key.
  const std::uint8_t doubled = static_cast<std::uint8_t>(token << 1);

  AS_CHRGET();

  // cpx #<(AS_TOKEN_LEFTSTR*2-1) = $cf
  // bcc AS_L_UNARY_1: doubled < $cf means numeric or CHR$ function.
  if (doubled >= static_cast<std::uint8_t>((kAS_TOKEN_LEFTSTR * 2u) - 1u)) {
    // String function path (AS_LEFT$, RIGHT$, MID$).
    AS_CHKOPN(); // require '('
    AS_FRMEVL(); // evaluate string argument into AS_FAC
    AS_CHKCOM(); // require ','
    AS_CHKSTR(); // ensure AS_FAC holds a string value

    // Save the string descriptor address (AS_VPNT = AS_FAC+3/AS_FAC+4) into
    // AS_DSCPTR before AS_GETBYT overwrites AS_FAC+4 with the sub-argument.
    // AS_VPNT = $a0/$a1 = ZP_AS_FAC+3/+4; AS_DSCPTR = ZP_AS_DSCPTR ($8c).
    const std::uint16_t vpnt = variables_const().readWord(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 3u));
    variables().writeWord(ApplesoftVariables::ZP_AS_DSCPTR, vpnt);

    // AS_GETBYT evaluates the numeric sub-argument; result lands in AS_FAC+4.
    (void)AS_GETBYT();
  } else {
    // AS_L_UNARY_1: numeric or CHR$ function — require "(expression)".
    AS_PARCHK();
  }

  // AS_L_UNARY_2: dispatch through AS_UNFNC table.
  // Index = token - AS_TOKEN_SGN (0-based, matches AS_UNFNC table layout).
  const std::size_t index = static_cast<std::size_t>(token - kAS_TOKEN_SGN);
  AS_UNFNC(index)();
  // CHR$, AS_LEFT$, RIGHT$, MID$ handlers do not return to this point.
  AS_CHKNUM();
}

void AS_RELOPS() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_RELOPS (inclusive) .. AS_STRCMP (exclusive)
  // Name normalization: none (assembler label AS_RELOPS kept verbatim).

  constexpr std::uint8_t kAS_CPRTYP = ApplesoftVariables::ZP_AS_CPRTYP;
  constexpr std::uint16_t kAS_ARG = ApplesoftVariables::ZP_AS_ARG;

  const std::uint8_t compareTypeFlags = ReadZeroPageByte(kAS_CPRTYP);
  if (AS_CHKVAL(compareTypeFlags)) {
    // Carry set in ROM indicates string compare path.
    AS_STRCMP();
    return;
  }

  gNumericCompareResult = AS_FCOMP(kAS_ARG);
  gNumericCompareCarry = gNumericCompareResult >= 0;
  AS_NUMCMP();
}

void AS_STRCMP() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_STRCMP (inclusive) .. AS_NUMCMP (exclusive)
  // Name normalization: none (assembler label AS_STRCMP kept verbatim).
  // Pointer candidates lifted: FAC+1/+2 and ARG+3/+4 are unified string-data
  // pointers.

  constexpr std::uint8_t kAS_VALTYP = ApplesoftVariables::ZP_AS_VALTYP;
  constexpr std::uint8_t kAS_CPRTYP = ApplesoftVariables::ZP_AS_CPRTYP;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_ARG = ApplesoftVariables::ZP_AS_ARG;
  constexpr std::uint8_t kAS_INDEX = ApplesoftVariables::ZP_AS_INDEX;

  WriteZeroPageByte(kAS_VALTYP, 0u);
  WriteZeroPageByte(
      kAS_CPRTYP, static_cast<std::uint8_t>(ReadZeroPageByte(kAS_CPRTYP) - 1u));

  const std::uint8_t facLength = AS_FREFAC();
  WriteZeroPageByte(kAS_FAC, facLength);
  WriteZeroPageWord(static_cast<std::uint8_t>(kAS_FAC + 1u),
                    ReadZeroPageWord(kAS_INDEX));

  const std::uint16_t argDescriptorAddress =
      ReadZeroPageWord(static_cast<std::uint8_t>(kAS_ARG + 3u));
  const std::uint8_t argLength = AS_FRETMP(argDescriptorAddress);
  WriteZeroPageByte(kAS_ARG, argLength);
  WriteZeroPageWord(static_cast<std::uint8_t>(kAS_ARG + 3u),
                    ReadZeroPageWord(kAS_INDEX));

  gNumericCompareResult = CompareArgAndFacStrings();
  AS_NUMCMP();
}

void AS_FRE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FRE (inclusive) .. AS_GIVAYF (exclusive)
  // Name normalization: none (assembler label AS_FRE kept verbatim).
  // AS_FRETOP and AS_STREND are one logical address pair each; model the
  // subtraction as one 16-bit free-space computation before floating the signed
  // result.

  if (ReadZeroPageByte(ApplesoftVariables::ZP_AS_VALTYP) != 0u) {
    (void)AS_FREFAC();
  }

  AS_GARBAG();

  const std::uint16_t fretop =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_FRETOP);
  const std::uint16_t strend =
      ReadZeroPageWord(ApplesoftVariables::ZP_AS_STREND);
  const std::uint16_t freeSpace = static_cast<std::uint16_t>(fretop - strend);
  AS_GIVAYF(static_cast<std::int16_t>(freeSpace));
}

void AS_GIVAYF(std::int16_t value) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GIVAYF (inclusive) .. AS_POS (exclusive)
  // Name normalization: none (assembler label AS_GIVAYF kept verbatim).
  // The A/Y pair is one signed 16-bit integer on entry; represent it as one
  // C++ value instead of split low/high byte locals.

  const std::uint16_t rawValue = static_cast<std::uint16_t>(value);

  WriteZeroPageByte(ApplesoftVariables::ZP_AS_VALTYP, 0u);
  WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_AS_FAC, 1u),
                    ApplesoftVariables::lowByte(rawValue));
  WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_AS_FAC, 2u),
                    ApplesoftVariables::highByte(rawValue));
  WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_AS_FAC, 3u), 0u);
  WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_AS_FAC, 4u), 0u);

  AS_FLOAT_1(0x90u);
}

void AS_FNC_() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FNC_ (inclusive) .. AS_FUNCT (exclusive)
  // Name normalization: none (assembler label AS_FNC_ kept verbatim).
  //
  // Common routine for "AS_DEF" and "FN" - parse FN token and function name.
  // Requires "FN" token, sets AS_SUBFLG high bit, parses name to AS_FNCNAM.

  // Require "FN" token
  AS_SYNCHR(static_cast<std::uint8_t>(0xc2u)); // AS_TOKEN_FN = 0xc2

  // Set high bit in AS_SUBFLG to signal this is from AS_DEF/FN context
  constexpr std::uint8_t kAS_SUBFLG = ApplesoftVariables::ZP_AS_SUBFLG;
  const std::uint8_t subflg = ReadZeroPageByte(kAS_SUBFLG);
  WriteZeroPageByte(kAS_SUBFLG, static_cast<std::uint8_t>(subflg | 0x80u));

  // Parse function name via AS_PTRGET3
  AS_PTRGET3();

  // AS_PTRGET3 leaves A=name_lo, Y=name_hi
  // Store to AS_FNCNAM
  constexpr std::uint8_t kAS_FNCNAM = ApplesoftVariables::ZP_AS_FNCNAM;
  const std::uint8_t nameA = ReadZeroPageByte(
      ApplesoftVariables::ZP_AS_STRNG1); // Temp storage from AS_PTRGET3
  const std::uint8_t nameY = ReadZeroPageByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_STRNG1 + 1u));
  WriteZeroPageByte(kAS_FNCNAM, nameA);
  WriteZeroPageByte(static_cast<std::uint8_t>(kAS_FNCNAM + 1u), nameY);

  // Jump to AS_CHKNUM to validate numeric type
  AS_CHKNUM();
}

void AS_DEF() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_DEF (inclusive) .. AS_FNC_ (exclusive)
  // Name normalization: none (assembler label AS_DEF kept verbatim).
  //
  // "AS_DEF" STATEMENT
  // Parse: AS_DEF FN name (arg) = expression
  // Stacks AS_VARPNT, AS_TXTPTR, and 5 bytes of AS_FAC, then jumps to
  // AS_FNCDATA to store.

  // Parse "FN name"
  AS_FNC_();

  // Error if in direct mode
  AS_ERRDIR();

  // Require "("
  AS_CHKOPN();

  // Set AS_SUBFLG to flag AS_DEF context for AS_PTRGET
  constexpr std::uint8_t kAS_SUBFLG = ApplesoftVariables::ZP_AS_SUBFLG;
  WriteZeroPageByte(kAS_SUBFLG, 0x80u);

  // Get pointer to argument variable
  AS_PTRGET();

  // Argument must be numeric
  AS_CHKNUM();

  // Require ")"
  AS_CHKCLS();

  // Require "=" and advance past it
  AS_SYNCHR(static_cast<std::uint8_t>(0xd0u)); // TOKEN_EQUAL = 0xd0

  // Stack the argument variable pointer (AS_VARPNT)
  constexpr std::uint8_t kAS_VARPNT = ApplesoftVariables::ZP_AS_VARPNT;
  WriteZeroPageByte(
      static_cast<std::uint8_t>(kAS_VARPNT + 1u),
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_VARPNT + 1u)));
  WriteZeroPageByte(kAS_VARPNT, ReadZeroPageByte(kAS_VARPNT));

  // Stack the text pointer (AS_TXTPTR)
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  const std::uint16_t txtPtr = ReadZeroPageWord(kAS_TXTPTR);
  (void)txtPtr;

  // Scan to next statement
  AS_DATA();

  // Fall through to AS_FNCDATA to store 5-byte AS_FAC
}

// Stack emulation for AS_FUNCT/AS_FNCDATA.
// 6502 stack is AS_LIFO at $0100-$01FF; we model it as a simple deque during
// execution. FN call stack is managed via theStack().pushFnByte() /
// peekFnByte() / popFnByte().

void AS_FUNCT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FUNCT (inclusive) .. AS_FNCDATA (exclusive)
  // Name normalization: none (assembler label AS_FUNCT kept verbatim).
  //
  // "FN" AS_FUNCTION AS_CALL - invoke user-defined function
  // Parse FN name, save old argument value, evaluate expression with new value,
  // restore old value via AS_FNCDATA.

  constexpr std::uint8_t kAS_FNCNAM = ApplesoftVariables::ZP_AS_FNCNAM;
  constexpr std::uint8_t kAS_VARPNT = ApplesoftVariables::ZP_AS_VARPNT;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;

  // Clear stack for this function call
  theStack().clearFnStack();

  // Parse "FN name"
  AS_FNC_();

  // Stack function address for nested FN calls (push high byte, then low byte)
  const std::uint16_t fncAddr = ReadZeroPageWord(kAS_FNCNAM);
  theStack().pushFnByte(ApplesoftVariables::highByte(fncAddr));
  theStack().pushFnByte(ApplesoftVariables::lowByte(fncAddr));

  // Parse "(expression)" and evaluate
  AS_PARCHK();

  // Result in AS_FAC - must be numeric
  AS_CHKNUM();

  // Pop function address back (in reverse order: low byte, then high byte)
  std::uint16_t tempAddr = ReadZeroPageWord(kAS_FNCNAM);
  ApplesoftVariables::setAS_LowByte(tempAddr, theStack().peekFnByte());
  theStack().popFnByte();
  ApplesoftVariables::setHighByte(tempAddr, theStack().peekFnByte());
  theStack().popFnByte();
  WriteZeroPageWord(kAS_FNCNAM, tempAddr);

  // Get argument variable pointer from AS_FNCNAM+2,+3 (offsets within function
  // definition) Read 16-bit pointer from function definition at offset +2,+3
  const std::uint16_t funcDefAddr = ReadZeroPageWord(kAS_FNCNAM);
  const std::uint8_t argVarAddrAS_Lo =
      variables_const().readByte(funcDefAddr + 2u);
  const std::uint8_t argVarAddrHi =
      variables_const().readByte(funcDefAddr + 3u);

  // Check for undefined function (high byte of address must be non-zero)
  if (argVarAddrHi == 0u) {
    // Undefined function error
    AS_UNDFNC();
    return;
  }

  const std::uint16_t argVarAddr =
      variables_const().makeWord(argVarAddrAS_Lo, argVarAddrHi);
  WriteZeroPageWord(kAS_VARPNT, argVarAddr);

  // Save old value of argument variable (5 bytes) to stack
  // AS_Loop from Y=4 down to Y=0 (inclusive)
  for (std::int8_t y = 4; y >= 0; --y) {
    const std::uint8_t byte = variables_const().readByte(argVarAddr + y);
    theStack().pushFnByte(byte);
  }

  // Store AS_FAC to argument variable (rounded)
  // The assembly loads Y with AS_VARPNT+1 and then calls
  // AS_STORE_FACDB_YX_ROUNDED. Since AS_STORE_FACDB_YX_ROUNDED takes no
  // parameters, it must read the target address from AS_VARPNT (which we just
  // set).
  AS_STORE_FACDB_YX_ROUNDED();

  // Save current AS_TXTPTR to stack (push high byte, then low byte)
  const std::uint16_t savedTxtPtr = ReadZeroPageWord(kAS_TXTPTR);
  theStack().pushFnByte(ApplesoftVariables::highByte(savedTxtPtr));
  theStack().pushFnByte(ApplesoftVariables::lowByte(savedTxtPtr));

  // AS_Load function definition address to AS_TXTPTR (point to function body)
  WriteZeroPageWord(kAS_TXTPTR, funcDefAddr);

  // Save argument variable address to stack (push high byte, then low byte)
  theStack().pushFnByte(ApplesoftVariables::highByte(argVarAddr));
  theStack().pushFnByte(ApplesoftVariables::lowByte(argVarAddr));

  // Evaluate the function expression
  AS_FRMNUM();

  // Pop argument variable address back and store to AS_FNCNAM
  std::uint16_t argAddr = ReadZeroPageWord(kAS_VARPNT);
  ApplesoftVariables::setAS_LowByte(argAddr, theStack().peekFnByte());
  theStack().popFnByte();
  ApplesoftVariables::setHighByte(argAddr, theStack().peekFnByte());
  theStack().popFnByte();
  WriteZeroPageWord(kAS_FNCNAM, argAddr);

  // Check for ":" or EOL
  if (AS_CHRGOT() != 0u && AS_CHRGOT() != static_cast<std::uint8_t>(':')) {
    AS_SYNERR();
  }

  // Pop and restore AS_TXTPTR
  std::uint16_t txtAddr = ReadZeroPageWord(kAS_TXTPTR);
  ApplesoftVariables::setAS_LowByte(txtAddr, theStack().peekFnByte());
  theStack().popFnByte();
  ApplesoftVariables::setHighByte(txtAddr, theStack().peekFnByte());
  theStack().popFnByte();
  WriteZeroPageWord(kAS_TXTPTR, txtAddr);

  // Stack now contains 5 saved bytes - fall through to AS_FNCDATA to restore
}

void AS_FNCDATA() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FNCDATA (inclusive) .. AS_STR (exclusive)
  // Name normalization: none (assembler label AS_FNCDATA kept verbatim).
  //
  // AS_STORE FIVE BYTES FROM AS_STACK AT (AS_FNCNAM)
  // Pop 5 stack bytes and store to (AS_FNCNAM),Y with Y incrementing from 0
  // to 4.

  constexpr std::uint8_t kAS_FNCNAM = ApplesoftVariables::ZP_AS_FNCNAM;
  const std::uint16_t fncnampnt = ReadZeroPageWord(kAS_FNCNAM);

  // AS_Loop 5 times: pop stack byte and store to (AS_FNCNAM)+Y
  for (std::uint8_t y = 0u; y < 5u; ++y) {
    if (!theStack().fnStackEmpty()) {
      const std::uint8_t byte = theStack().peekFnByte();
      theStack().popFnByte();
      variables().writeByte(fncnampnt + y, byte);
    }
  }
}

void AS_FRMEVL() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FRMEVL (inclusive) .. AS_FRM_STACK_1 (exclusive)
  // Name normalization: AS_FRMEVL_1/2 and related sublabels are modeled inline.
  //
  // Incremental port note:
  // This now includes the AS_FRM_RECURSE..AS_FRM_STACK_1 tranche by modeling
  // the recursive precedence walk and the stacked-AS_LHS frame handoff to
  // AS_ARG/AS_CPRMASK.

  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_VALTYP = ApplesoftVariables::ZP_AS_VALTYP;
  constexpr std::uint8_t kAS_CPRTYP = ApplesoftVariables::ZP_AS_CPRTYP;
  constexpr std::uint8_t kAS_CPRMASK = ApplesoftVariables::ZP_AS_CPRMASK;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_FAC_SIGN = ApplesoftVariables::ZP_AS_FAC_SIGN;
  constexpr std::uint8_t kAS_ARG = ApplesoftVariables::ZP_AS_ARG;
  constexpr std::uint8_t kAS_SGNCPR =
      ApplesoftVariables::ZP_AS_STRNG1; // AS_SGNCPR shares $AB with AS_STRNG1.
  constexpr std::uint8_t kAS_STACK_ROOM_BYTES = 1u;
  constexpr std::uint8_t kAS_TOKEN_PLUS = 0xc8u;
  constexpr std::uint8_t kAS_TOKEN_GREATER = 0xcfu;
  constexpr std::uint8_t kTOKEN_EQUAL = 0xd0u;
  constexpr std::uint8_t kTOKEN_AS_LESS = 0xd1u;

  const auto frmevl_eval = [&](auto &&self, std::uint8_t callerPrecedence,
                               bool runEntryBackstep) -> void {
    // AS_FRMEVL entry point only: back AS_TXTPTR up one byte so AS_FRM_ELEMENT
    // starts from the current token. Recursive AS_FRMEVL_1 calls skip this
    // backstep.
    if (runEntryBackstep) {
      const std::uint16_t txtptr = ReadZeroPageWord(kAS_TXTPTR);
      WriteZeroPageWord(kAS_TXTPTR, static_cast<std::uint16_t>(txtptr - 1u));
    }

    // AS_FRMEVL_1 prologue: AS_CHKMEM(1), AS_FRM_ELEMENT.
    AS_CHKMEMState chkmemState{};
    chkmemState.a = kAS_STACK_ROOM_BYTES;
    if (!AS_CHKMEM(chkmemState).ok) {
      return;
    }
    AS_UNARY();
    WriteZeroPageByte(kAS_CPRTYP, 0u);

    while (true) {
      std::uint8_t token = AS_CHRGOT();

      // AS_FRMEVL_2 relational scan: absorb chains of <, =, >.
      while (token == kAS_TOKEN_GREATER || token == kTOKEN_EQUAL ||
             token == kTOKEN_AS_LESS) {
        std::uint8_t mask = 0u;
        if (token == kAS_TOKEN_GREATER) {
          mask = 0x01u;
        } else if (token == kTOKEN_EQUAL) {
          mask = 0x02u;
        } else {
          mask = 0x04u;
        }

        const std::uint8_t existing = ReadZeroPageByte(kAS_CPRTYP);
        if ((existing & mask) != 0u) {
          AS_SYNERR();
          return;
        }

        WriteZeroPageByte(kAS_CPRTYP,
                          static_cast<std::uint8_t>(existing | mask));
        token = AS_CHRGET();
      }

      MathTblEntry pendingEntry{};
      std::uint8_t cprtypForFrame = 0u;
      bool relationalPath = false;

      if (ReadZeroPageByte(kAS_CPRTYP) != 0u) {
        // AS_FRM_RELATIONAL: fold string-vs-numeric state into AS_CPRTYP and
        // treat as AS_MATHTBL AS_M_REL for precedence dispatch.
        relationalPath = true;
        const std::uint8_t relFlags = ReadZeroPageByte(kAS_CPRTYP);
        const bool facIsString = (ReadZeroPageByte(kAS_VALTYP) & 0x80u) != 0u;
        cprtypForFrame = static_cast<std::uint8_t>((relFlags << 1u) |
                                                   (facIsString ? 1u : 0u));

        const std::uint16_t txtptr = ReadZeroPageWord(kAS_TXTPTR);
        WriteZeroPageWord(kAS_TXTPTR, static_cast<std::uint16_t>(txtptr - 1u));
        pendingEntry = AS_MATHTBL(AS_M_REL_IDX);
      } else {
        // AS_NOTMATH/AS_GOEX: stop when the next token is not an infix
        // operator.
        if (token < kAS_TOKEN_PLUS || token > kTOKEN_AS_LESS) {
          return;
        }

        // AS_FRMEVL_2_3 special-case (+ with string AS_FAC) is AS_CAT in ROM.
        if (token == kAS_TOKEN_PLUS &&
            (ReadZeroPageByte(kAS_VALTYP) & 0x80u) != 0u) {
          AS_CAT();
          return;
        }

        AS_CHKNUM();

        const std::size_t mathIndex =
            static_cast<std::size_t>(token - kAS_TOKEN_PLUS);
        if (mathIndex > AS_M_REL_IDX) {
          return;
        }
        pendingEntry = AS_MATHTBL(mathIndex);
      }

      // AS_FRM_PRECEDENCE_TEST/AS_PREFNC: defer lower-or-equal precedence work
      // to the caller stack frame.
      if (callerPrecedence >= pendingEntry.precedence) {
        return;
      }

      // AS_FRM_RECURSE (inclusive) .. AS_FRM_STACK_1 (exclusive): recurse into
      // AS_FRMEVL_1 while carrying pending operator/precedence state.
      const std::array<std::uint8_t, 5> lhsFac = {
          ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 0u)),
          ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 1u)),
          ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 2u)),
          ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 3u)),
          ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 4u)),
      };
      const std::uint8_t lhsSign = ReadZeroPageByte(kAS_FAC_SIGN);

      if (!relationalPath) {
        (void)AS_CHRGET();
      }
      self(self, pendingEntry.precedence, false);

      // AS_FRM_PERFORM_2 frame handoff: move stacked left operand to AS_ARG and
      // synthesize AS_CPRMASK/AS_SGNCPR as if popped from the ROM expression
      // stack.
      WriteZeroPageByte(kAS_CPRMASK,
                        static_cast<std::uint8_t>(cprtypForFrame >> 1u));
      for (std::uint8_t i = 0; i < lhsFac.size(); ++i) {
        WriteZeroPageByte(static_cast<std::uint8_t>(kAS_ARG + i), lhsFac[i]);
      }
      WriteZeroPageByte(static_cast<std::uint8_t>(kAS_ARG + 5u), lhsSign);
      WriteZeroPageByte(
          kAS_SGNCPR,
          static_cast<std::uint8_t>(lhsSign ^ ReadZeroPageByte(kAS_FAC_SIGN)));

      if (cprtypForFrame != 0u) {
        WriteZeroPageByte(kAS_CPRTYP, cprtypForFrame);
      }

      if (pendingEntry.handler != nullptr) {
        pendingEntry.handler();
      }
    }
  };

  frmevl_eval(frmevl_eval, 0u, true);
}

std::uint8_t MON_SCRN(std::uint8_t row, std::uint8_t column) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/lores.o65.lst
  // AS_Labels: SCRN (inclusive) .. end of SCRN routine (exclusive)
  // Name normalization: SCRN -> MON_SCRN (monitor label gets MON_ prefix).
  //
  // Monitor path:
  //   lsr A / php / jsr GBASCALC / lda (GBASL),Y / plp
  //   if odd row, shift high nibble down; then mask to 4-bit color.

  const std::uint8_t halfRow = static_cast<std::uint8_t>(row >> 1u);
  const bool gbasCarry = (row & 0x01u) != 0u;

  // Inline GBASCALC for base address synthesis in page $0400-$07ff.
  const std::uint8_t gbash =
      static_cast<std::uint8_t>(((halfRow >> 1u) & 0x03u) | 0x04u);
  std::uint8_t gbasl = static_cast<std::uint8_t>(halfRow & 0x18u);
  if (gbasCarry) {
    gbasl = static_cast<std::uint8_t>(gbasl + 0x80u);
  }
  const std::uint8_t gbaslBase = gbasl;
  gbasl = static_cast<std::uint8_t>((gbasl << 2u) | gbaslBase);

  const std::uint16_t baseAddress = ApplesoftVariables::makeWord(gbasl, gbash);
  const std::uint16_t screenAddress =
      static_cast<std::uint16_t>(baseAddress + column);

  std::uint8_t value = variables_const().readByte(screenAddress);
  if ((row & 0x01u) != 0u) {
    value = static_cast<std::uint8_t>(value >> 4u);
  }
  return static_cast<std::uint8_t>(value & 0x0fu);
}

void MON_IRQ();

class MON_M6502VEC {
public:
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/vectors.o65.lst
  // AS_Labels: M6502VEC (inclusive) .. end of listing (exclusive)
  // Name normalization: M6502VEC -> MON_M6502VEC (monitor label gets MON_
  // prefix).
  //
  // EOM vector helpers: each method directly transfers control to the target
  // used by the original vector entry.
  static void NMI_VECTOR() {
    // FFFA points at $03FB.
    ApplesoftNG::ExternalJumpDispatcher::Jump(0x03fbu);
  }

  static void RESET_VECTOR() {
    // FFFC points to RESET2.
    MON_RESET2();
  }

  static void IRQ_VECTOR() {
    // FFFE points to IRQ.
    MON_IRQ();
  }
};

static void MON_BREAK() { MON_OLDBRK(); }
void MON_IRQ() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/debug.o65.lst
  // AS_Labels: IRQ (inclusive) .. BREAK (exclusive)
  // Name normalization: IRQ -> MON_IRQ (monitor label gets MON_ prefix).
  //
  // 6502 IRQ/BRK handler entry point.
  // - Saves A to $45.
  // - Checks processor status on stack to distinguish IRQ from BRK.
  // - If BRK (bit 4 of status on stack is set), branches to BREAK.
  // - If IRQ, jumps through vector at $03FE.

  WriteZeroPageByte(0x45, 0); // sta $45 (A is currently not passed, using 0)

  // Simulate bit 4 (Break) check. In a real environment, we'd examine the
  // stack frame pushed by the CPU.
  const bool isBreak = false; // Placeholder

  if (isBreak) {
    // Jumps to BREAK (0x000c in debug.o65.lst)
    // BREAK logic:
    // - Restore CPU state, save to monitor registers, jump through $03F0.
    MON_BREAK();
  } else {
    constexpr std::uint16_t kIRQVector =
        ApplesoftVariables::ADDR_MON_IRQ_VECTOR;
    ApplesoftNG::ExternalJumpDispatcher::Jump(
        variables_const().readWord(kIRQVector));
  }
}

std::int8_t AS_FCOMP(std::uint16_t argAddress) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FCOMP (inclusive) .. AS_FCOMP2 (exclusive)
  // Name normalization: none (assembler label AS_FCOMP kept verbatim).
  // Pointer candidate: DEST ($60/$61) is one unified pointer to the packed
  // comparand.

  WriteZeroPageWord(ApplesoftVariables::ZP_AS_DEST, argAddress);

  // ROM falls through directly into AS_FCOMP2 after storing DEST.
  AS_FCOMP2();
  return gNumericCompareResult;
}

std::int8_t CompareArgAndFacStrings() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: STRCMP_1 (inclusive) .. PDL (exclusive)
  // Name normalization: CompareArgAndFacStrings is the C++ helper name for the
  // STRCMP_1/STRCMP_2 loop. Pointer candidates lifted: ARG+3/+4 and FAC+1/+2
  // are the two unified string-data pointers.

  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_FAC_SIGN = ApplesoftVariables::ZP_AS_FAC_SIGN;
  constexpr std::uint8_t kAS_ARG = ApplesoftVariables::ZP_AS_ARG;

  const std::uint8_t argLength = ReadZeroPageByte(kAS_ARG);
  const std::uint8_t facLength = ReadZeroPageByte(kAS_FAC);

  std::uint8_t shorterFlag = 0u;
  std::uint8_t compareCount = argLength;
  if (argLength < facLength) {
    shorterFlag = 1u;
  } else if (argLength > facLength) {
    shorterFlag = 0xffu;
    compareCount = facLength;
  }
  WriteZeroPageByte(kAS_FAC_SIGN, shorterFlag);

  const auto argString = variables_const().pointer(
      ReadZeroPageWord(static_cast<std::uint8_t>(kAS_ARG + 3u)));
  const auto facString = variables_const().pointer(
      ReadZeroPageWord(static_cast<std::uint8_t>(kAS_FAC + 1u)));
  for (std::uint16_t offset = 0u; offset < compareCount; ++offset) {
    const std::uint8_t argByte = argString.read(offset);
    const std::uint8_t facByte = facString.read(offset);
    if (argByte > facByte) {
      return -1;
    }
    if (argByte < facByte) {
      return 1;
    }
  }

  if (shorterFlag == 0xffu) {
    return -1;
  }
  if (shorterFlag == 1u) {
    return 1;
  }
  return 0;
}

std::uint8_t MON_PREAD() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/paddles.o65.lst
  // AS_Labels: PREAD (inclusive) .. end of listing routine (exclusive)
  // Name normalization: PREAD -> MON_PREAD (monitor label gets MON_ prefix).
  //
  // Monitor flow:
  //   lda PTRIG        ; trigger paddle timing
  //   ldy #$00         ; counter
  // PREAD2:
  //   lda PADDL0,X     ; selected paddle input
  //   bpl RTS2D        ; return when bit 7 clears
  //   iny              ; count while timing bit remains set
  //   bne PREAD2       ; saturate at 255
  //   dey
  // RTS2D: rts

  constexpr std::uint8_t kAS_FAC_LAST =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 4u);

  // In the original monitor, paddle index is supplied in X.
  // The current C++ calling path provides the converted operand in AS_FAC+4.
  const std::uint8_t paddleIndex = ReadZeroPageByte(kAS_FAC_LAST);
  const std::uint16_t paddleAddress =
      static_cast<std::uint16_t>(IOPorts::ADDR_PADDLE_0 + paddleIndex);

  (void)variables_const().readByte(IOPorts::ADDR_PADDLE_TRIGGER);

  std::uint8_t count = 0u;
  for (;;) {
    const std::uint8_t paddleValue = variables_const().readByte(paddleAddress);
    if ((paddleValue & 0x80u) == 0u) {
      return count;
    }

    count = static_cast<std::uint8_t>(count + 1u);
    if (count == 0u) {
      return 0xffu;
    }
  }
}

} // namespace applesoft::asm_port
