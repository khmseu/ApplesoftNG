#include "core/applesoft_variables.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/asm_port_stack.hpp"
#include "core/asm_port_token_address_table.hpp"
#include "core/io_ports.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

bool IsStatementEndOfParsedInput();
std::uint8_t ReadZeroPageByte(std::uint8_t address);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);
void AS_PRINT_ERROR_LINNUM(std::string_view prefix);
std::uint8_t AS_CHRGOT();
std::uint8_t AS_CHRGET();
void AS_LINGET();
void AS_SYNERR();
void AS_SYNCHR(std::uint8_t expected);
void AS_LET();
void AS_IF();
void AS_REM();
void AS_IF_TRUE();
void AS_ONGOTO();
void AS_CONTROL_C_TYPED();
void AS_STEP();
void AS_TRACE_();
void AS_FRMNUM();
void AS_FRMEVL();
void AS_CHKNUM();
void AS_FADD();
void PushForPntFrame();
std::uint8_t AS_GETBYT();
void AS_ADDON(std::uint8_t offset);
bool AS_ISCNTC();
void AS_OUTSP();
void AS_LINPRT();
void AS_OUTDO(std::uint8_t value);
std::uint8_t CurrentStatementChar();
bool IsRunningMode();
bool IsTraceEnabled();
bool IsEndOfAS_LineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadAS_LineNumberFromTextPointer();
void AdvanceTextPointerToNextAS_Line();
void AS_GOEND();
void AS_EXECUTE_STATEMENT();
void AS_EXECUTE_STATEMENT_1();
void AS_NEWSTT();
void AS_RESTART();
void AS_RTS_5();
void AS_GOSUB();
void AS_GO_TO_LINE();
void AS_GOTO();
void AS_POP();
void AS_RETURN();
std::uint16_t AS_PTRGET();
void AS_PULL3();
std::uint8_t AS_REMN();
std::uint8_t AS_DATAN();
bool AS_FL1(std::uint16_t startAddress);
void AS_HANDLERR();
void SetPendingErrorCode(std::uint8_t errorCode);
std::uint8_t AS_INCHR();
void SetTextPointer(std::uint16_t address);
void ClearErrFlag();
void MarkDirectMode();
void AS_PARSE_INPUT_LINE();
void HandleNumberedAS_Line();
void AS_CRDO();
void AS_SETFOR();
void AS_ROUND_FAC();
extern std::int8_t gNumericCompareResult;

constexpr std::uint8_t kTokenBase = 0x80u;
constexpr std::uint8_t AS_RESTART_PROMPT = ']' | 0x80u;

void AS_STOP_impl(bool shouldPrintBreak);
void AS_ENDX_impl(bool shouldPrintBreak);

namespace {

constexpr std::uint16_t kStepAS_LabelAddress = 0x07afu;
constexpr std::uint16_t kConOneScratchAddress = 0x03fbu;
constexpr std::uint8_t kPackedFloatByteCount = 5u;
constexpr std::uint8_t kStepValueOffsetInForFrame = 4u;
// Applesoft packed float for 1.0:
// exponent=0x81 (biased exponent for 2^0), then sign-packed high mantissa, mid
// mantissa, low mantissa, extension byte.
constexpr std::uint8_t kConOnePacked[kPackedFloatByteCount] = {
    0x81u, 0x00u, 0x00u, 0x00u, 0x00u};

void ApplyFacSign() {
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_FAC_SIGN = ApplesoftVariables::ZP_AS_FAC_SIGN;

  const std::uint8_t facSign = ReadZeroPageByte(kAS_FAC_SIGN);
  const std::uint8_t facMantissaHigh = ReadZeroPageByte(kAS_FAC + 1u);
  // ROM sequence at $079C: AS_LDA AS_FAC_SIGN / AS_ORA #$7F / AND AS_FAC+1 /
  // STA AS_FAC+1. This clears bit 7 for positive values and preserves AS_FAC+1
  // when AS_FAC_SIGN is negative.
  const std::uint8_t signedMantissaHigh =
      static_cast<std::uint8_t>(facMantissaHigh & (facSign | 0x7fu));
  WriteZeroPageByte(kAS_FAC + 1u, signedMantissaHigh);
}

void SetBranchTargetToAS_STEP() {
  constexpr std::uint8_t kAS_INDEX = ApplesoftVariables::ZP_AS_INDEX;

  WriteZeroPageWord(kAS_INDEX, kStepAS_LabelAddress);
}

void AS_LOAD_FAC_FROM_YA() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_LOAD_FAC_FROM_YA (inclusive) .. AS_STORE_FAC_IN_TEMP2_ROUNDED
  // (exclusive) This function ports only AS_LOAD_FAC_FROM_YA;
  // AS_STORE_FAC_IN_TEMP2_ROUNDED starts at the exclusive end label. Name
  // normalization: none (assembler label AS_LOAD_FAC_FROM_YA kept verbatim).
  constexpr std::uint8_t kAS_INDEX = ApplesoftVariables::ZP_AS_INDEX;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_FAC_SIGN = ApplesoftVariables::ZP_AS_FAC_SIGN;
  constexpr std::uint8_t kAS_FAC_EXTENSION =
      ApplesoftVariables::ZP_AS_FAC_EXTENSION;

  // Caller precondition: AS_INDEX points to the packed 5-byte source value.
  const ProgramPointer source{ReadZeroPageWord(kAS_INDEX)};
  WriteZeroPageByte(kAS_FAC + 4u, source.read(4u));
  WriteZeroPageByte(kAS_FAC + 3u, source.read(3u));
  WriteZeroPageByte(kAS_FAC + 2u, source.read(2u));

  const std::uint8_t signPackedMantissa = source.read(1u);
  // AS_FAC_SIGN stores packed byte #1 (bit 7 is sign, remaining bits are high
  // mantissa bits). AS_FAC+1 then reuses mantissa bits with bit 7 forced set by
  // AS_OR #$80 to restore normalized form.
  WriteZeroPageByte(kAS_FAC_SIGN, signPackedMantissa);
  WriteZeroPageByte(kAS_FAC + 1u,
                    static_cast<std::uint8_t>(signPackedMantissa | 0x80u));
  WriteZeroPageByte(kAS_FAC, source.read(0u));
  WriteZeroPageByte(kAS_FAC_EXTENSION, 0u);
}

std::int8_t AS_SIGN2(std::uint8_t sign) {
  // AS_Labels: AS_SIGN2 (inclusive) .. AS_SGN (exclusive)
  // MSBIT to carry, then return -1 if carry set, +1 if carry clear.
  if ((sign & 0x80u) != 0u) {
    return -1;
  }
  return 1;
}

std::int8_t AS_SIGN1() {
  // AS_Labels: AS_SIGN1 (inclusive) .. AS_SIGN2 (exclusive)
  return AS_SIGN2(ReadZeroPageByte(ApplesoftVariables::ZP_AS_FAC_SIGN));
}

std::int8_t AS_SIGN() {
  // AS_Labels: AS_SIGN (inclusive) .. AS_SIGN2 (exclusive)
  if (ReadZeroPageByte(ApplesoftVariables::ZP_AS_FAC) == 0u) {
    return 0; // Numbers are effectively zero
  }
  return AS_SIGN1();
}

std::int8_t AS_L_FCOMP2_2(std::uint8_t signByte) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_L_FCOMP2_2 (inclusive) .. AS_QINT (exclusive)
  // Name normalization: none (assembler label AS_L_FCOMP2_2 kept verbatim).

  return AS_SIGN2(signByte);
}

std::int8_t AS_L_FCOMP2_1(bool compareCarrySet) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_L_FCOMP2_1 (inclusive) .. AS_L_FCOMP2_2 (exclusive)
  // Name normalization: none (assembler label AS_L_FCOMP2_1 kept verbatim).

  std::uint8_t signByte = ReadZeroPageByte(ApplesoftVariables::ZP_AS_FAC_SIGN);
  if (compareCarrySet) {
    signByte ^= 0xffu;
  }
  return AS_L_FCOMP2_2(signByte);
}

} // namespace

void AS_FCOMP2() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FCOMP2 (inclusive) .. AS_L_FCOMP2_1 (exclusive)
  // Name normalization: none (assembler label AS_FCOMP2 kept verbatim).
  // Pointer candidate: DEST ($60/$61) is one unified pointer to the packed
  // comparand.

  constexpr std::uint8_t kAS_DEST = ApplesoftVariables::ZP_AS_DEST;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_FAC_SIGN = ApplesoftVariables::ZP_AS_FAC_SIGN;
  constexpr std::uint8_t kAS_FAC_EXTENSION =
      ApplesoftVariables::ZP_AS_FAC_EXTENSION;

  const auto comparand = variables_const().pointer(ReadZeroPageWord(kAS_DEST));
  const std::uint8_t comparandExponent = comparand.read(0u);
  if (comparandExponent == 0u) {
    gNumericCompareResult = AS_SIGN();
    return;
  }

  const std::uint8_t comparandMantissaHighWithSign = comparand.read(1u);
  if (((comparandMantissaHighWithSign ^ ReadZeroPageByte(kAS_FAC_SIGN)) &
       0x80u) != 0u) {
    gNumericCompareResult = AS_SIGN1();
    return;
  }

  const std::uint8_t facExponent = ReadZeroPageByte(kAS_FAC);
  bool compareCarrySet = comparandExponent >= facExponent;
  if (comparandExponent != facExponent) {
    gNumericCompareResult = AS_L_FCOMP2_1(compareCarrySet);
    return;
  }

  const std::uint8_t normalizedComparandMantissaHigh =
      static_cast<std::uint8_t>(comparandMantissaHighWithSign | 0x80u);
  const std::uint8_t facMantissaHigh =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 1u));
  compareCarrySet = normalizedComparandMantissaHigh >= facMantissaHigh;
  if (normalizedComparandMantissaHigh != facMantissaHigh) {
    gNumericCompareResult = AS_L_FCOMP2_1(compareCarrySet);
    return;
  }

  for (std::uint16_t offset = 2u; offset <= 3u; ++offset) {
    const std::uint8_t comparandMantissaByte = comparand.read(offset);
    const std::uint8_t facMantissaByte =
        ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + offset));
    compareCarrySet = comparandMantissaByte >= facMantissaByte;
    if (comparandMantissaByte != facMantissaByte) {
      gNumericCompareResult = AS_L_FCOMP2_1(compareCarrySet);
      return;
    }
  }

  const std::uint8_t facExtension = ReadZeroPageByte(kAS_FAC_EXTENSION);
  const std::uint8_t comparandExtension = comparand.read(4u);
  const std::uint8_t facMantissaLow =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FAC + 4u));
  compareCarrySet =
      (0x7fu > facExtension) ||
      ((0x7fu == facExtension) && (comparandExtension >= facMantissaLow));
  if ((0x7fu == facExtension) && (comparandExtension == facMantissaLow)) {
    gNumericCompareResult = 0;
    return;
  }

  gNumericCompareResult = AS_L_FCOMP2_1(compareCarrySet);
}

namespace {

void AS_FRM_STACK_2(std::uint8_t signByte) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FRM_STACK_2 (inclusive) .. AS_FRM_STACK_3 (exclusive)
  // Name normalization: none (assembler label AS_FRM_STACK_2 kept verbatim).
  constexpr std::uint8_t kAS_INDEXZeroPageAddress =
      ApplesoftVariables::ZP_AS_INDEX;
  constexpr std::uint8_t kAS_INDEXHighByteAddress =
      static_cast<std::uint8_t>(kAS_INDEXZeroPageAddress + 1u);

  const std::uint8_t returnAddressAS_Low = theStack().popByte();
  // Net effect of ROM sequence PLA / STA AS_INDEX / INC AS_INDEX:
  // store the low return-address byte plus one as an 8-bit value so AS_INDEX
  // points at the byte immediately after the JSR call-site return location.
  // The uint8_t cast intentionally truncates carry; ROM assumes no
  // page-boundary carry into AS_INDEX+1.
  WriteZeroPageByte(kAS_INDEXZeroPageAddress,
                    static_cast<std::uint8_t>(returnAddressAS_Low + 1u));
  WriteZeroPageByte(kAS_INDEXHighByteAddress, theStack().popByte());

  theStack().pushByte(signByte);
}

void AS_FRM_STACK_3() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FRM_STACK_3 (inclusive) .. AS_NOTMATH (exclusive)
  // Name normalization: none (assembler label AS_FRM_STACK_3 kept verbatim).
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_INDEX = ApplesoftVariables::ZP_AS_INDEX;

  AS_ROUND_FAC();

  theStack().pushByte(ReadZeroPageByte(kAS_FAC + 4u));
  theStack().pushByte(ReadZeroPageByte(kAS_FAC + 3u));
  theStack().pushByte(ReadZeroPageByte(kAS_FAC + 2u));
  theStack().pushByte(ReadZeroPageByte(kAS_FAC + 1u));
  theStack().pushByte(ReadZeroPageByte(kAS_FAC));

  const std::uint16_t branchTarget = ReadZeroPageWord(kAS_INDEX);
  if (branchTarget == kStepAS_LabelAddress) {
    AS_STEP();
  }
  // Other indirect targets used by AS_FRM_STACK_3 are not ported yet; return to
  // caller.
}

constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
  return static_cast<std::uint8_t>(lhs + rhs);
}

std::uint16_t readStackWordAt(std::uint8_t x, std::uint8_t lowOffset,
                              std::uint8_t highOffset) {
  return ApplesoftVariables::makeWord(theStack().readByteAt(x, lowOffset),
                                      theStack().readByteAt(x, highOffset));
}

// TODO(asm-port): decide branch condition after comparing AS_FOR value with end
// value.
bool AS_NEXT_shouldTerminateLoop() { return false; }

std::uint8_t ScanAheadOffset(std::uint8_t terminator) {
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_CHARAC = ApplesoftVariables::ZP_AS_CHARAC;
  constexpr std::uint8_t kAS_ENDCHR = ApplesoftVariables::ZP_AS_ENDCHR;

  WriteZeroPageByte(kAS_CHARAC, terminator);
  std::uint8_t offset = 0;
  WriteZeroPageByte(kAS_ENDCHR, 0);

  while (true) {
    const std::uint8_t previousEnd = ReadZeroPageByte(kAS_ENDCHR);
    const std::uint8_t previousCharac = ReadZeroPageByte(kAS_CHARAC);
    WriteZeroPageByte(kAS_CHARAC, previousEnd);
    WriteZeroPageByte(kAS_ENDCHR, previousCharac);

    while (true) {
      const ProgramPointer textPtr{ReadZeroPageWord(kAS_TXTPTR)};
      const std::uint8_t ch = textPtr.read(offset);
      if (ch == 0 || ch == ReadZeroPageByte(kAS_ENDCHR)) {
        return offset;
      }

      ++offset;
      if (ch == static_cast<std::uint8_t>('"')) {
        break;
      }
    }
  }
}

bool isDigit(std::uint8_t ch) { return ch >= '0' && ch <= '9'; }

} // namespace

void AS_SETFOR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SETFOR (inclusive) .. AS_COPY_ARG_TO_FAC (exclusive)
  // Name normalization: none (assembler label AS_SETFOR kept verbatim).
  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;
  constexpr std::uint8_t kAS_FAC_SIGN = ApplesoftVariables::ZP_AS_FAC_SIGN;
  constexpr std::uint8_t kAS_FAC_EXTENSION =
      ApplesoftVariables::ZP_AS_FAC_EXTENSION;

  AS_ROUND_FAC();

  const ProgramPointer forVariablePtr{ReadZeroPageWord(kAS_FORPNT)};
  forVariablePtr.write(ReadZeroPageByte(kAS_FAC), 0u);

  const std::uint8_t facMantissaHigh = ReadZeroPageByte(add_u8(kAS_FAC, 1u));
  const std::uint8_t facSign = ReadZeroPageByte(kAS_FAC_SIGN);
  // ROM sequence: (AS_FAC+1) is masked by (AS_FAC_SIGN | $7F), preserving
  // mantissa low 7 bits while applying sign-bit packing semantics.
  const std::uint8_t packedMantissaHigh = static_cast<std::uint8_t>(
      facMantissaHigh & static_cast<std::uint8_t>(facSign | 0x7fu));
  forVariablePtr.write(packedMantissaHigh, 1u);
  forVariablePtr.write(ReadZeroPageByte(add_u8(kAS_FAC, 2u)), 2u);
  forVariablePtr.write(ReadZeroPageByte(add_u8(kAS_FAC, 3u)), 3u);
  forVariablePtr.write(ReadZeroPageByte(add_u8(kAS_FAC, 4u)), 4u);

  WriteZeroPageByte(kAS_FAC_EXTENSION, 0u);
}

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);

bool gReturnFromPopContext = false;

bool ReturnWasFromAS_POPContext() {
  if (!gReturnFromPopContext) {
    return false;
  }

  gReturnFromPopContext = false;
  return true;
}

std::uint8_t PeekTopControlTokenAfterAS_GTFORPNT() {
  return theStack().readByteAt(theStack().readStackPointer(), 1u);
}

bool AS_ISCNTC() {
  constexpr std::uint8_t kCTRL_C_CODE = 0x83;

  if (ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD) != kCTRL_C_CODE) {
    return false;
  }

  AS_INCHR();
  AS_CONTROL_C_TYPED();
  return true;
}

void AS_STOP() { AS_STOP_impl(false); }

void AS_STOP_impl(bool shouldPrintBreak) {
  if (!IsStatementEndOfParsedInput()) {
    return;
  }

  AS_ENDX_impl(shouldPrintBreak);
}

void AS_ENDX() { AS_ENDX_impl(false); }

void AS_ENDX_impl(bool shouldPrintBreak) {
  if (!IsStatementEndOfParsedInput()) {
    return;
  }

  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_OLDTEXT = ApplesoftVariables::ZP_AS_OLDTEXT;
  constexpr std::uint8_t kAS_OLDLIN = ApplesoftVariables::ZP_AS_OLDLIN;

  const std::uint16_t textPointer = ReadZeroPageWord(kAS_TXTPTR);
  const std::uint16_t currentAS_Line = ReadZeroPageWord(kAS_CURLIN);
  const std::uint8_t currentPageHi =
      ApplesoftVariables::highByte(currentAS_Line);

  if (static_cast<std::uint8_t>(currentPageHi + 1u) != 0u) {
    WriteZeroPageWord(kAS_OLDTEXT, textPointer);
    WriteZeroPageWord(kAS_OLDLIN, currentAS_Line);
  }

  theStack().popReturnAddress();
  theStack().popReturnAddress();

  if (shouldPrintBreak) {
    AS_PRINT_ERROR_LINNUM(AS_QT_ERROR(AS_QT_BREAK_INDEX));
    return;
  }

  AS_RESTART();
}

void AS_RESTART() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_RESTART (inclusive)
  // Name normalization: none (assembler label AS_RESTART kept verbatim).

  AS_CRDO();
  const Inlin2Result inlin2 = AS_INLIN2(AS_RESTART_PROMPT);
  SetTextPointer(inlin2.address());
  ClearErrFlag();

  const std::uint8_t firstChar = AS_CHRGET();
  if (firstChar == 0) {
    AS_RESTART();
    return;
  }

  MarkDirectMode();

  if (isDigit(firstChar)) {
    HandleNumberedAS_Line();
    return;
  }

  AS_PARSE_INPUT_LINE();
  AS_TRACE_();
}

void AS_CONTROL_C_TYPED() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_CONTROL_C_TYPED (inclusive) .. AS_STOP (exclusive)
  // Name normalization: none (assembler label AS_CONTROL_C_TYPED kept
  // verbatim).
  constexpr std::uint8_t kAS_ERRFLG = ApplesoftVariables::ZP_AS_ERRFLG;
  const std::uint8_t errFlags = ReadZeroPageByte(kAS_ERRFLG);

  // `bit AS_ERRFLG` / `bpl` in ROM: when sign bit is set, ON ERR is active and
  // AS_CONTROL-C dispatches to AS_HANDLERR with code $FF semantics.
  if ((errFlags & 0x80u) != 0u) {
    SetPendingErrorCode(0xffu);
    AS_HANDLERR();
    return;
  }

  // Control-C attempts to fall through to the AS_STOP/END handler with an
  // implicit "break" condition.
  AS_STOP_impl(true);
}

void AS_CONT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_CONT (inclusive) .. AS_SAVE (exclusive)
  // Name normalization: none (assembler label AS_CONT kept verbatim).
  // Internal label mapping: "bne AS_RTS_4" is modeled as an early return.

  if (!IsStatementEndOfParsedInput()) {
    return;
  }

  constexpr std::uint8_t kAS_OLDTEXT = ApplesoftVariables::ZP_AS_OLDTEXT;
  constexpr std::uint8_t kAS_OLDTEXT_plus_1 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_OLDTEXT + 1u);
  constexpr std::uint8_t kAS_OLDLIN = ApplesoftVariables::ZP_AS_OLDLIN;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;

  if (ReadZeroPageByte(kAS_OLDTEXT_plus_1) == 0) {
    AS_ERROR(AS_ERR_CANTCONT);
    return;
  }

  WriteZeroPageWord(kAS_TXTPTR, ReadZeroPageWord(kAS_OLDTEXT));
  WriteZeroPageWord(kAS_CURLIN, ReadZeroPageWord(kAS_OLDLIN));
}

void AS_GOSUB() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GOSUB (inclusive) .. AS_GO_TO_LINE (exclusive)
  // Name normalization: none (assembler label AS_GOSUB kept verbatim).
  //
  // Executes the "AS_GOSUB" command:
  // - Checks stack space for the return frame (7 bytes)
  // - Pushes return frame containing: AS_TXTPTR (2), AS_CURLIN (2),
  // AS_TOKEN_GOSUB (1)
  // - Falls through to shared AS_GO_TO_LINE logic to find and execute the
  // target line
  // - On AS_RETURN, restores execution state from the stack frame

  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_TOKEN_GOSUB = 0xb0;

  AS_CHKMEMState chkmemState{};
  chkmemState.a = 3;
  chkmemState.stackPointer = theStack().readStackPointer();
  const auto chkmemResult = AS_CHKMEM(chkmemState);
  if (!chkmemResult.ok) {
    return;
  }

  const std::uint16_t textPointer = ReadZeroPageWord(kAS_TXTPTR);
  const std::uint16_t currentAS_Line = ReadZeroPageWord(kAS_CURLIN);

  theStack().pushWord(textPointer);
  theStack().pushWord(currentAS_Line);
  theStack().pushByte(kAS_TOKEN_GOSUB);

  AS_GO_TO_LINE();
}

void AS_GO_TO_LINE() {
  (void)AS_CHRGOT();
  AS_GOTO();
  AS_NEWSTT();
}

void AS_GOTO() {
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;

  AS_LINGET();
  const std::uint8_t remnOffset = AS_REMN();

  const std::uint8_t currentPage =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_CURLIN + 1u));
  const std::uint8_t targetPage =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_LINNUM + 1u));

  ProgramPointer start{};
  if (currentPage >= targetPage) {
    start = ProgramPointer{ReadZeroPageWord(kAS_TXTTAB)};
  } else {
    const ProgramPointer textPtr{ReadZeroPageWord(kAS_TXTPTR)};
    start = textPtr.advanced(static_cast<std::uint16_t>(remnOffset) + 1u);
  }

  if (!AS_FL1(start.address)) {
    AS_ERROR(AS_ERR_UNDEFSTAT);
    return;
  }

  const std::uint16_t destination =
      static_cast<std::uint16_t>(ReadZeroPageWord(kAS_LOWTR) - 1u);
  WriteZeroPageWord(kAS_TXTPTR, destination);
}

void AS_RESUME() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_RESUME (inclusive) .. AS_JSYN (exclusive)
  // Name normalization: none (assembler label AS_RESUME kept verbatim).
  constexpr std::uint8_t kAS_ERRLIN = ApplesoftVariables::ZP_AS_ERRLIN;
  constexpr std::uint8_t kAS_ERRPOS = ApplesoftVariables::ZP_AS_ERRPOS;
  constexpr std::uint8_t kAS_ERRSTK = ApplesoftVariables::ZP_AS_ERRSTK;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;

  WriteZeroPageWord(kAS_CURLIN, ReadZeroPageWord(kAS_ERRLIN));
  WriteZeroPageWord(kAS_TXTPTR, ReadZeroPageWord(kAS_ERRPOS));
  theStack().setStackPointer(ReadZeroPageByte(kAS_ERRSTK));
  AS_NEWSTT();
}

void AS_ONERR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ONERR (inclusive) .. AS_HANDLERR (exclusive)
  // Name normalization: none (assembler label AS_ONERR kept verbatim).
  constexpr std::uint8_t kAS_TOKEN_GOTO = 0xabu;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_TXTPSV = ApplesoftVariables::ZP_AS_TXTPSV;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_CURLSV = ApplesoftVariables::ZP_AS_CURLSV;
  constexpr std::uint8_t kAS_ERRFLG = ApplesoftVariables::ZP_AS_ERRFLG;

  AS_SYNCHR(kAS_TOKEN_GOTO);
  WriteZeroPageWord(kAS_TXTPSV, ReadZeroPageWord(kAS_TXTPTR));

  const std::uint8_t errflg = ReadZeroPageByte(kAS_ERRFLG);
  WriteZeroPageByte(kAS_ERRFLG,
                    static_cast<std::uint8_t>((errflg >> 1u) | 0x80u));

  WriteZeroPageWord(kAS_CURLSV, ReadZeroPageWord(kAS_CURLIN));
  AS_ADDON(AS_REMN());
}

void AS_RTS_5() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_RTS_5 (inclusive) .. AS_RETURN (exclusive)
  // Name normalization: RTS_5 -> AS_RTS_5 virtual Applesoft prefix only.
  // ROM label RTS_5 is a shared return target for AS_GOTO and AS_POP.
  return;
}

void AS_PULL3() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PULL3 (inclusive) .. AS_IF (exclusive)
  // Name normalization: none (assembler label AS_PULL3 kept verbatim).
  [[maybe_unused]] const std::uint8_t pulled1 = theStack().popByte();
  [[maybe_unused]] const std::uint8_t pulled2 = theStack().popByte();
  [[maybe_unused]] const std::uint8_t pulled3 = theStack().popByte();
}

std::uint8_t AS_REMN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_REMN (inclusive) .. AS_PULL3 (exclusive)
  // Name normalization: none (assembler label AS_REMN kept verbatim).

  return ScanAheadOffset(0);
}

void PushForPntFrame() {
  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;
  theStack().pushByte(ReadZeroPageByte(add_u8(kAS_FORPNT, 1u)));
  theStack().pushByte(ReadZeroPageByte(kAS_FORPNT));
  theStack().pushToken(AS_TOKEN_FOR);
}

void AS_FOR() {
  constexpr std::uint8_t kAS_SUBFLG = ApplesoftVariables::ZP_AS_SUBFLG;
  constexpr std::uint8_t kAS_TOKEN_TO = 0xc1u;

  WriteZeroPageByte(kAS_SUBFLG, 0x80);
  AS_LET();

  AS_GTFORPNTState gtforpntState{};
  for (std::size_t i = 0; i < gtforpntState.stackPage.size(); ++i) {
    gtforpntState.stackPage[i] =
        ReadProgramByte(static_cast<std::uint16_t>(0x0100u + i));
  }
  const auto gtforpntResult =
      AS_GTFORPNT(theStack().readStackPointer(), gtforpntState);
  if (gtforpntResult.found) {
    theStack().setStackPointer(add_u8(gtforpntResult.x, 15u));
  }

  theStack().popReturnAddress();
  theStack().popReturnAddress();

  AS_CHKMEMState chkmemState{};
  chkmemState.a = 9;
  chkmemState.stackPointer = theStack().readStackPointer();
  const auto chkmemResult = AS_CHKMEM(chkmemState);
  if (!chkmemResult.ok) {
    return;
  }

  AS_DATAN();
  theStack().pushTextPointerAddress();
  theStack().pushCurrentAS_LineNumber();
  theStack().pushToken(kAS_TOKEN_TO);
  AS_SYNCHR(kAS_TOKEN_TO);
  AS_CHKNUM();
  AS_FRMNUM();
  ApplyFacSign();
  SetBranchTargetToAS_STEP();
  AS_FRM_STACK_3();
}

void AS_NEXT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_NEXT (inclusive) .. AS_FRMNUM (exclusive)
  // Name normalization: none (assembler label AS_NEXT kept verbatim).

  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;

  // d0 04 / AS_NEXT_1 jsr AS_PTRGET / AS_NEXT_2 sta AS_FORPNT, sty AS_FORPNT+1
  // No-variable AS_NEXT case is represented by AS_FORPNT+1 = 0.
  if (AS_CHRGOT() == 0u) {
    WriteZeroPageByte(add_u8(kAS_FORPNT, 1u), 0u);
  } else {
    const std::uint16_t varPtr = AS_PTRGET();
    WriteZeroPageWord(kAS_FORPNT, varPtr);
  }

  // jsr AS_GTFORPNT
  AS_GTFORPNTState gtforpntState{};
  gtforpntState.forpntAS_Lo = ReadZeroPageByte(kAS_FORPNT);
  gtforpntState.forpntHi = ReadZeroPageByte(add_u8(kAS_FORPNT, 1u));
  for (std::size_t i = 0; i < gtforpntState.stackPage.size(); ++i) {
    gtforpntState.stackPage[i] =
        ReadProgramByte(static_cast<std::uint16_t>(0x0100u + i));
  }

  const auto gtforpntResult =
      AS_GTFORPNT(theStack().readStackPointer(), gtforpntState);
  if (!gtforpntResult.found) {
    // AS_Ldx #AS_ERR_NOFOR / jmp AS_ERROR via AS_GERR/AS_JERROR path.
    AS_ERROR(AS_ERR_NOFOR);
    return;
  }

  // AS_NEXT_3: txs
  theStack().setStackPointer(gtforpntResult.x);

  // AS_STEP arithmetic path (AS_LOAD_FAC_FROM_YA / AS_FADD / AS_SETFOR /
  // AS_FCOMP2). Stack offsets follow ROM comments; helpers are placeholders
  // until stack memory and AS_FAC math ports are fully wired.
  WriteZeroPageWord(
      ApplesoftVariables::ZP_AS_INDEX,
      static_cast<std::uint16_t>(
          0x0100u + add_u8(gtforpntResult.x, kStepValueOffsetInForFrame)));
  AS_LOAD_FAC_FROM_YA();
  WriteZeroPageByte(ApplesoftVariables::ZP_AS_FAC_SIGN,
                    theStack().readByteAt(gtforpntResult.x, 9u)); // AS_FAC_SIGN
  WriteZeroPageWord(kAS_FORPNT, ReadZeroPageWord(kAS_FORPNT));
  AS_FADD();
  AS_SETFOR();
  AS_FCOMP2();

  if (!AS_NEXT_shouldTerminateLoop()) {
    // Restore line/AS_TXTPTR from AS_FOR frame and jump AS_NEWSTT.
    const std::uint16_t restoredAS_Line =
        readStackWordAt(gtforpntResult.x, 15u, 16u);
    const std::uint16_t restoredTextPointer =
        readStackWordAt(gtforpntResult.x, 18u, 17u);
    WriteZeroPageWord(kAS_CURLIN, restoredAS_Line);
    WriteZeroPageWord(kAS_TXTPTR, restoredTextPointer);
    AS_NEWSTT();
    return;
  }

  // AS_L_NEXT_3_2: pop AS_FOR frame, then continue AS_NEWSTT unless another
  // variable follows in AS_NEXT var-list (AS_NEXT I,J,...).
  theStack().setStackPointer(add_u8(gtforpntResult.x, 18u));

  if (AS_CHRGOT() != static_cast<std::uint8_t>(',')) {
    AS_NEWSTT();
    return;
  }

  AS_CHRGET();
  // jsr AS_NEXT_1 (does not return in ROM when comma-separated variables
  // remain).
  AS_NEXT();
}

void AS_POP() {
  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;
  constexpr std::uint8_t kAS_TOKEN_GOSUB = 0xb0;

  if (!IsStatementEndOfParsedInput()) {
    AS_RTS_5();
    return;
  }

  WriteZeroPageByte(kAS_FORPNT, 0xffu);

  AS_GTFORPNTState gtforpntState{};
  const auto gtforpntResult =
      AS_GTFORPNT(theStack().readStackPointer(), gtforpntState);
  theStack().setStackPointer(gtforpntResult.x);

  if (PeekTopControlTokenAfterAS_GTFORPNT() == kAS_TOKEN_GOSUB) {
    gReturnFromPopContext = true;
    AS_RETURN();
    return;
  }

  AS_ERROR(AS_ERR_NOGOSUB);
}

void AS_RETURN() {
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;

  (void)theStack().popByte();
  const std::uint8_t currentAS_LineLo = theStack().popByte();

  if (ReturnWasFromAS_POPContext()) {
    AS_PULL3();
    return;
  }

  const std::uint8_t currentAS_LineHi = theStack().popByte();
  const std::uint8_t textPointerAS_Lo = theStack().popByte();
  const std::uint8_t textPointerHi = theStack().popByte();

  WriteZeroPageWord(kAS_CURLIN, ApplesoftVariables::makeWord(currentAS_LineLo,
                                                             currentAS_LineHi));
  WriteZeroPageWord(kAS_TXTPTR, ApplesoftVariables::makeWord(textPointerAS_Lo,
                                                             textPointerHi));
}

void AS_STEP() {
  constexpr std::uint8_t kAS_TOKEN_STEP = 0xc7u;

  for (std::uint8_t i = 0; i < kPackedFloatByteCount; ++i) {
    WriteProgramByte(static_cast<std::uint16_t>(kConOneScratchAddress + i),
                     kConOnePacked[i]);
  }
  WriteZeroPageWord(ApplesoftVariables::ZP_AS_INDEX, kConOneScratchAddress);
  AS_LOAD_FAC_FROM_YA();
  if (AS_CHRGOT() == kAS_TOKEN_STEP) {
    AS_CHRGET();
    AS_FRMNUM();
  }

  const std::int8_t stepSign = AS_SIGN();
  AS_FRM_STACK_2(static_cast<std::uint8_t>(stepSign));
  PushForPntFrame();
  AS_NEWSTT();
}

void AS_NEWSTT() {
  constexpr std::uint8_t kAS_REMSTK = ApplesoftVariables::ZP_AS_REMSTK;
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  constexpr std::uint8_t kAS_OLDTEXT = ApplesoftVariables::ZP_AS_OLDTEXT;

  WriteZeroPageByte(kAS_REMSTK, theStack().readStackPointer());

  if (AS_ISCNTC()) {
    return;
  }

  if (ReadZeroPageByte(static_cast<std::uint8_t>(kAS_CURLIN + 1u)) != 0xffu) {
    WriteZeroPageWord(kAS_OLDTEXT, ReadZeroPageWord(kAS_TXTPTR));
  } else {
    WriteZeroPageWord(kAS_OLDTEXT, 0);
  }

  if (IsEndOfAS_LineAtTextPointer()) {
    if (IsEndOfProgramAtTextPointer()) {
      AS_GOEND();
      return;
    }
  }

  WriteZeroPageWord(kAS_CURLIN, ReadAS_LineNumberFromTextPointer());
  AdvanceTextPointerToNextAS_Line();
  AS_TRACE_();
}

void AS_TRACE_() {
  if (IsTraceEnabled()) {
    if (IsRunningMode()) {
      AS_OUTDO('#' & 0x7fu);
      AS_LINPRT();
      AS_OUTSP();
    }
  }

  AS_CHRGET();
  AS_EXECUTE_STATEMENT();
  AS_NEWSTT();
}

void AS_GOEND() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_GOEND (inclusive) .. AS_EXECUTE_STATEMENT (exclusive)
  // Name normalization: none (assembler label AS_GOEND kept verbatim).
  // End-of-program path in AS_NEWSTT jumps into AS_END4 with carry clear, which
  // restarts without printing BREAK. Model that directly here.
  AS_RESTART();
}

bool IsEndOfAS_LineAtTextPointer() {
  // Source: AS_NEWSTT inline — ldy #0 / lda (AS_TXTPTR),Y: end-of-statement
  // when byte is 0.
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  return ReadProgramByte(ReadZeroPageWord(kAS_TXTPTR)) == 0u;
}

bool IsEndOfProgramAtTextPointer() {
  // Source: AS_NEWSTT inline — ldy #2 / lda (AS_TXTPTR),Y: next-line link high
  // byte; if zero the program has ended (null forward pointer).
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  return ReadProgramByte(static_cast<std::uint16_t>(
             ReadZeroPageWord(kAS_TXTPTR) + 2u)) == 0u;
}

std::uint16_t ReadAS_LineNumberFromTextPointer() {
  // Source: AS_NEWSTT inline — reads AS_CURLIN from (AS_TXTPTR)+3 and
  // (AS_TXTPTR)+4. Memory layout at AS_TXTPTR when it sits on an EOL 0x00:
  //   [0] = 0x00 (EOL), [1] = link.lo, [2] = link.hi, [3] = lineno.lo, [4] =
  //   lineno.hi.
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  const std::uint16_t txtptr = ReadZeroPageWord(kAS_TXTPTR);
  const std::uint8_t lo =
      ReadProgramByte(static_cast<std::uint16_t>(txtptr + 3u));
  const std::uint8_t hi =
      ReadProgramByte(static_cast<std::uint16_t>(txtptr + 4u));
  return ApplesoftVariables::makeWord(lo, hi);
}

void AdvanceTextPointerToNextAS_Line() {
  // Source: AS_NEWSTT inline — tya (A=4) + adc AS_TXTPTR → AS_TXTPTR += 4.
  // AS_CHRGET called next by AS_TRACE_ adds 1 more, landing on the first
  // content byte.
  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;
  WriteZeroPageWord(kAS_TXTPTR, static_cast<std::uint16_t>(
                                    ReadZeroPageWord(kAS_TXTPTR) + 4u));
}

bool IsRunningMode() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_TRACE_ (inclusive) .. AS_EXECUTE_STATEMENT (exclusive)
  // Name normalization: helper name chosen for the inline AS_TRACE_ predicate.
  // AS_TRACE_ checks AS_CURLIN+1 and only traces when non-zero (running mode).
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;
  return ReadZeroPageByte(static_cast<std::uint8_t>(kAS_CURLIN + 1u)) != 0u;
}

bool IsTraceEnabled() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_TRACE_ (inclusive) .. AS_EXECUTE_STATEMENT (exclusive)
  // Name normalization: helper name chosen for the inline AS_TRACE_ predicate.
  // `bit AS_TRCFLG` + `bpl` means tracing is enabled when AS_TRCFLG bit 7 is
  // set.
  constexpr std::uint8_t kAS_TRCFLG = ApplesoftVariables::ZP_AS_TRCFLG;
  return (ReadZeroPageByte(kAS_TRCFLG) & 0x80u) != 0u;
}

void AS_EXECUTE_STATEMENT() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_EXECUTE_STATEMENT (inclusive) .. AS_EXECUTE_STATEMENT_1
  // (exclusive) Name normalization: none (assembler label AS_EXECUTE_STATEMENT
  // kept verbatim).

  if (CurrentStatementChar() == 0) {
    // EMPTY STATEMENT: fall through to caller behavior.
    return;
  }

  AS_EXECUTE_STATEMENT_1();
}

void AS_EXECUTE_STATEMENT_1() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_EXECUTE_STATEMENT_1 (inclusive) .. AS_COLON_ (exclusive)
  // Name normalization: none (assembler label AS_EXECUTE_STATEMENT_1 kept
  // verbatim).

  const std::uint8_t ch = CurrentStatementChar();
  if ((ch & 0x80u) == 0u) {
    AS_LET();
    return;
  }

  const std::uint8_t tokenIndex = static_cast<std::uint8_t>(ch - kTokenBase);
  if (tokenIndex >= 0x40u) {
    AS_SYNERR();
    return;
  }

  AS_CHRGET();
  const AS_TOKEN_ADDRESS_TABLE_fn handler =
      AS_TOKEN_ADDRESS_TABLE(static_cast<std::size_t>(tokenIndex));
  handler();
}

void AS_COLON_() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_COLON_ (inclusive) .. AS_RESTORE (exclusive)
  // Name normalization: none (assembler label AS_COLON_ kept verbatim).

  if (CurrentStatementChar() == static_cast<std::uint8_t>(':')) {
    AS_TRACE_();
    return;
  }

  AS_SYNERR();
}

void AS_IF() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_IF (inclusive) .. AS_REM (exclusive)
  // Name normalization: none (assembler label AS_IF kept verbatim).

  constexpr std::uint8_t kAS_TOKEN_GOTO = 0xabu;
  constexpr std::uint8_t kAS_TOKEN_THEN = 0xc4u;
  constexpr std::uint8_t kAS_FAC = ApplesoftVariables::ZP_AS_FAC;

  AS_FRMEVL();
  if (AS_CHRGOT() != kAS_TOKEN_GOTO) {
    AS_SYNCHR(kAS_TOKEN_THEN);
  }

  if (ReadZeroPageByte(kAS_FAC) != 0u) {
    AS_IF_TRUE();
    return;
  }

  // False AS_IF falls through to AS_REM in ROM.
  AS_REM();
}

void AS_REM() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_REM (inclusive) .. AS_IF_TRUE (exclusive)
  // Name normalization: none (assembler label AS_REM kept verbatim).

  const std::uint8_t offset = AS_REMN();
  AS_ADDON(offset);
}

void AS_IF_TRUE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_IF_TRUE (inclusive) .. AS_ONGOTO (exclusive)
  // Name normalization: none (assembler label AS_IF_TRUE kept verbatim).

  if (AS_CHRGOT() >= kTokenBase) {
    AS_EXECUTE_STATEMENT();
    return;
  }

  AS_GOTO();
}

void AS_ONGOTO() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ONGOTO (inclusive) .. AS_LINGET (exclusive)
  // Name normalization: none (assembler label AS_ONGOTO kept verbatim).

  constexpr std::uint8_t kAS_TOKEN_GOSUB = 0xb0u;
  constexpr std::uint8_t kAS_TOKEN_GOTO = 0xabu;
  constexpr std::uint8_t kAS_FAC_PLUS_4 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 4u);

  const std::uint8_t token = AS_GETBYT();
  if (token != kAS_TOKEN_GOSUB && token != kAS_TOKEN_GOTO) {
    AS_SYNERR();
    return;
  }

  while (true) {
    const std::uint8_t selector = ReadZeroPageByte(kAS_FAC_PLUS_4);
    WriteZeroPageByte(kAS_FAC_PLUS_4, static_cast<std::uint8_t>(selector - 1u));

    if (selector == 1u) {
      AS_EXECUTE_STATEMENT_1();
      return;
    }

    AS_CHRGET();
    AS_LINGET();
    if (AS_CHRGOT() == static_cast<std::uint8_t>(',')) {
      continue;
    }

    return;
  }
}

} // namespace applesoft::asm_port
