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

void AS_END2_impl(bool shouldPrintBreak);
void AS_END4_impl(bool shouldPrintBreak);
void AS_ENDX_impl(bool shouldPrintBreak);
void AS_STOP_impl(bool shouldPrintBreak);
void AS_RESTART();
void AS_CRDO();
struct Inlin2Result;
Inlin2Result AS_INLIN2(std::uint8_t prompt);
void ClearErrFlag();
void MarkDirectMode();
void HandleNumberedAS_Line();
void AS_PARSE_INPUT_LINE();
void SetTextPointer(std::uint16_t address);

bool IsStatementEndOfParsedInput();
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
void AS_JSYN();
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

constexpr std::uint16_t kStepLabelAddress = 0x07afu;
constexpr std::uint16_t kConOneScratchAddress = 0xe913u;
constexpr std::uint8_t kPackedFloatByteCount = 5u;
constexpr std::uint8_t kStepValueOffsetInForFrame = 4u;
// Applesoft packed float for 1.0:
// exponent=0x81 (biased exponent for 2^0), then sign-packed high mantissa, mid
// mantissa, low mantissa, extension byte.
constexpr std::uint8_t kConOnePacked[kPackedFloatByteCount] = {
    0x81u, 0x00u, 0x00u, 0x00u, 0x00u};

void ApplyFacSign() {
  const std::uint8_t facSign = variables_const().AS_FAC_SIGN;
  const std::uint8_t facMantissaHigh = variables_const().AS_FAC[1];
  // ROM sequence at $079C: AS_LDA AS_FAC_SIGN / AS_ORA #$7F / AND AS_FAC+1 /
  // STA AS_FAC+1. This clears bit 7 for positive values and preserves AS_FAC+1
  // when AS_FAC_SIGN is negative.
  const std::uint8_t signedMantissaHigh =
      static_cast<std::uint8_t>(facMantissaHigh & (facSign | 0x7fu));
  variables().AS_FAC[1] = signedMantissaHigh;
}

void SetBranchTargetToAS_STEP() { variables().AS_INDEX = kStepLabelAddress; }

void AS_LOAD_FAC_FROM_YA() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_LOAD_FAC_FROM_YA (inclusive) .. AS_STORE_FAC_IN_TEMP2_ROUNDED
  // (exclusive) This function ports only AS_LOAD_FAC_FROM_YA;
  // AS_STORE_FAC_IN_TEMP2_ROUNDED starts at the exclusive end label. Name
  // normalization: none (assembler label AS_LOAD_FAC_FROM_YA kept verbatim).
  // Caller precondition: AS_INDEX points to the packed 5-byte source value.
  const ProgramPointer source{variables_const().AS_INDEX};
  variables().AS_FAC[4] = source.read(4u);
  variables().AS_FAC[3] = source.read(3u);
  variables().AS_FAC[2] = source.read(2u);

  const std::uint8_t signPackedMantissa = source.read(1u);
  // AS_FAC_SIGN stores packed byte #1 (bit 7 is sign, remaining bits are high
  // mantissa bits). AS_FAC+1 then reuses mantissa bits with bit 7 forced set by
  // AS_OR #$80 to restore normalized form.
  variables().AS_FAC_SIGN = signPackedMantissa;
  variables().AS_FAC[1] = static_cast<std::uint8_t>(signPackedMantissa | 0x80u);
  variables().AS_FAC[0] = source.read(0u);
  variables().AS_FAC_EXTENSION = 0u;
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
  return AS_SIGN2(variables_const().AS_FAC_SIGN);
}

std::int8_t AS_SIGN() {
  // AS_Labels: AS_SIGN (inclusive) .. AS_SIGN2 (exclusive)
  if (variables_const().AS_FAC[0] == 0u) {
    return 0; // Numbers are effectively zero
  }
  return AS_SIGN1();
}

std::int8_t AS_L_FCOMP2_2(std::uint8_t signByte) {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_L_FCOMP2_2 (inclusive) .. AS_QINT (exclusive)
  // Name normalization: none (assembler label AS_L_FCOMP2_2 kept verbatim).

  return AS_SIGN2(signByte);
}

std::int8_t AS_L_FCOMP2_1(bool compareCarrySet) {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_L_FCOMP2_1 (inclusive) .. AS_L_FCOMP2_2 (exclusive)
  // Name normalization: none (assembler label AS_L_FCOMP2_1 kept verbatim).

  std::uint8_t signByte = variables_const().AS_FAC_SIGN;
  if (compareCarrySet) {
    signByte ^= 0xffu;
  }
  return AS_L_FCOMP2_2(signByte);
}

} // namespace

void AS_FCOMP2() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_FCOMP2 (inclusive) .. AS_L_FCOMP2_1 (exclusive)
  // Name normalization: none (assembler label AS_FCOMP2 kept verbatim).
  // Pointer candidate: DEST ($60/$61) is one unified pointer to the packed
  // comparand.

  const auto comparand = variables_const().pointer(variables_const().AS_DEST);
  const std::uint8_t comparandExponent = comparand.read(0u);
  if (comparandExponent == 0u) {
    gNumericCompareResult = AS_SIGN();
    return;
  }

  const std::uint8_t comparandMantissaHighWithSign = comparand.read(1u);
  if (((comparandMantissaHighWithSign ^ variables_const().AS_FAC_SIGN) &
       0x80u) != 0u) {
    gNumericCompareResult = AS_SIGN1();
    return;
  }

  const std::uint8_t facExponent = variables_const().AS_FAC[0];
  bool compareCarrySet = comparandExponent >= facExponent;
  if (comparandExponent != facExponent) {
    gNumericCompareResult = AS_L_FCOMP2_1(compareCarrySet);
    return;
  }

  const std::uint8_t normalizedComparandMantissaHigh =
      static_cast<std::uint8_t>(comparandMantissaHighWithSign | 0x80u);
  const std::uint8_t facMantissaHigh = variables_const().AS_FAC[1];
  compareCarrySet = normalizedComparandMantissaHigh >= facMantissaHigh;
  if (normalizedComparandMantissaHigh != facMantissaHigh) {
    gNumericCompareResult = AS_L_FCOMP2_1(compareCarrySet);
    return;
  }

  for (std::uint16_t offset = 2u; offset <= 3u; ++offset) {
    const std::uint8_t comparandMantissaByte = comparand.read(offset);
    const std::uint8_t facMantissaByte = variables_const().AS_FAC[offset];
    compareCarrySet = comparandMantissaByte >= facMantissaByte;
    if (comparandMantissaByte != facMantissaByte) {
      gNumericCompareResult = AS_L_FCOMP2_1(compareCarrySet);
      return;
    }
  }

  const std::uint8_t facExtension = variables_const().AS_FAC_EXTENSION;
  const std::uint8_t comparandExtension = comparand.read(4u);
  const std::uint8_t facMantissaLow = variables_const().AS_FAC[4];
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

void PushFacSignReturnAddress(std::uint8_t signByte) {
  const std::uint8_t returnAddressAS_Low = theStack().popByte();
  // Net effect of ROM sequence PLA / STA AS_INDEX / INC AS_INDEX:
  // store the low return-address byte plus one as an 8-bit value so AS_INDEX
  // points at the byte immediately after the JSR call-site return location.
  // The uint8_t cast intentionally truncates carry; ROM assumes no
  // page-boundary carry into AS_INDEX+1.
  ApplesoftVariables::setLowByte(
      variables().AS_INDEX,
      static_cast<std::uint8_t>(returnAddressAS_Low + 1u));
  ApplesoftVariables::setHighByte(variables().AS_INDEX, theStack().popByte());

  theStack().pushByte(signByte);
}

void PushRoundedFacAndDispatch() {
  AS_ROUND_FAC();

  theStack().pushByte(variables_const().AS_FAC[4]);
  theStack().pushByte(variables_const().AS_FAC[3]);
  theStack().pushByte(variables_const().AS_FAC[2]);
  theStack().pushByte(variables_const().AS_FAC[1]);
  theStack().pushByte(variables_const().AS_FAC[0]);

  const std::uint16_t branchTarget = variables_const().AS_INDEX;
  if (branchTarget == kStepLabelAddress) {
    AS_STEP();
  }
  // Other indirect targets used by this FOR/STEP helper are not ported yet;
  // return to caller.
}

constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
  return static_cast<std::uint8_t>(lhs + rhs);
}

std::uint16_t readStackWordAt(std::uint8_t x, std::uint8_t lowOffset,
                              std::uint8_t highOffset) {
  return ApplesoftVariables::makeWord(theStack().readByteAt(x, lowOffset),
                                      theStack().readByteAt(x, highOffset));
}

// Assembly: tsx; sec; sbc STACK+9,X; beq L_NEXT_3_2
// Terminates (returns true) when the 8-bit difference is zero, i.e. when
// the unsigned comparison result from AS_FCOMP2 equals the step sign byte.
bool AS_NEXT_shouldTerminateLoop(std::uint8_t forFrameX) {
  const std::uint8_t stepSign = theStack().readByteAt(forFrameX, 9u);
  return static_cast<std::uint8_t>(gNumericCompareResult) == stepSign;
}

std::uint8_t ScanAheadOffset(std::uint8_t terminator) {
  variables().AS_CHARAC = terminator;
  std::uint8_t offset = 0;
  variables().AS_ENDCHR = 0;

  while (true) {
    const std::uint8_t previousEnd = variables_const().AS_ENDCHR;
    const std::uint8_t previousCharac = variables_const().AS_CHARAC;
    variables().AS_CHARAC = previousEnd;
    variables().AS_ENDCHR = previousCharac;

    while (true) {
      const ProgramPointer textPtr{variables_const().AS_TXTPTR};
      const std::uint8_t ch = textPtr.read(offset);
      if (ch == 0 || ch == variables_const().AS_ENDCHR) {
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
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_SETFOR (inclusive) .. AS_COPY_ARG_TO_FAC (exclusive)
  // Name normalization: none (assembler label AS_SETFOR kept verbatim).
  AS_ROUND_FAC();

  const ProgramPointer forVariablePtr{variables_const().AS_FORPNT};
  forVariablePtr.write(variables_const().AS_FAC[0], 0u);

  const std::uint8_t facMantissaHigh = variables_const().AS_FAC[1];
  const std::uint8_t facSign = variables_const().AS_FAC_SIGN;
  // ROM sequence: (AS_FAC+1) is masked by (AS_FAC_SIGN | $7F), preserving
  // mantissa low 7 bits while applying sign-bit packing semantics.
  const std::uint8_t packedMantissaHigh = static_cast<std::uint8_t>(
      facMantissaHigh & static_cast<std::uint8_t>(facSign | 0x7fu));
  forVariablePtr.write(packedMantissaHigh, 1u);
  forVariablePtr.write(variables_const().AS_FAC[2], 2u);
  forVariablePtr.write(variables_const().AS_FAC[3], 3u);
  forVariablePtr.write(variables_const().AS_FAC[4], 4u);

  variables().AS_FAC_EXTENSION = 0u;
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

// AS_Line 1925, 1781 call this: jsr AS_ISCNTC
// AS_Labels: AS_ISCNTC (inclusive) .. AS_CONTROL_C_TYPED (exclusive)
bool AS_ISCNTC() {
  constexpr std::uint8_t kCTRL_C_CODE = 0x83;

  if (ioPorts_const().readByte(IOPorts::ADDR_AS_KEYBOARD) != kCTRL_C_CODE) {
    return false;
  }

  AS_INCHR();
  AS_CONTROL_C_TYPED();
  return true;
}

// AS_Labels: AS_STOP (inclusive) .. AS_END2 (exclusive)
void AS_STOP() { AS_STOP_impl(false); }

void AS_STOP_impl(bool shouldPrintBreak) {
  if (!IsStatementEndOfParsedInput()) {
    return;
  }

  AS_ENDX_impl(shouldPrintBreak);
}

// AS_Labels: AS_ENDX (inclusive) .. AS_END2 (exclusive)
void AS_ENDX() { AS_ENDX_impl(false); }

void AS_ENDX_impl(bool shouldPrintBreak) {
  if (!IsStatementEndOfParsedInput()) {
    return;
  }
  AS_END2_impl(shouldPrintBreak);
}

// AS_Labels: AS_END2 (inclusive) .. AS_END4 (exclusive)
void AS_END2_impl(bool shouldPrintBreak) {
  const std::uint16_t textPointer = variables_const().AS_TXTPTR;
  const std::uint16_t currentAS_Line = variables_const().AS_CURLIN;
  const std::uint8_t currentPageHi =
      ApplesoftVariables::highByte(currentAS_Line);

  if (static_cast<std::uint8_t>(currentPageHi + 1u) != 0u) {
    variables().AS_OLDTEXT = textPointer;
    variables().AS_OLDLIN = currentAS_Line;
  }

  theStack().setStackPointer(0xffu); // AS_STKINI logic
  AS_END4_impl(shouldPrintBreak);
}

// AS_Labels: AS_END4 (inclusive) .. AS_CONT (exclusive)
void AS_END4_impl(bool shouldPrintBreak) {
  if (shouldPrintBreak) {
    AS_PRINT_ERROR_LINNUM(AS_QT_ERROR(AS_QT_BREAK_INDEX));
    return;
  }

  AS_RESTART();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_RESTART (inclusive)
// Name normalization: none (assembler label AS_RESTART kept verbatim).
void AS_RESTART() {
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

// AS_Labels: AS_CONTROL_C_TYPED (inclusive) .. AS_STOP (exclusive)
void AS_CONTROL_C_TYPED() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // Name normalization: none (assembler label AS_CONTROL_C_TYPED kept
  // verbatim).
  const std::uint8_t errFlags = variables_const().AS_ERRFLG;

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

// AS_Labels: AS_CONT (inclusive) .. AS_SAVE (exclusive)
void AS_CONT() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // Name normalization: none (assembler label AS_CONT kept verbatim).
  // Internal label mapping: "bne AS_RTS_4" is modeled as an early return.

  if (!IsStatementEndOfParsedInput()) {
    return;
  }

  if (ApplesoftVariables::highByte(variables_const().AS_OLDTEXT) == 0) {
    AS_ERROR(AS_ERR_CANTCONT);
    return;
  }

  variables().AS_TXTPTR = variables_const().AS_OLDTEXT;
  variables().AS_CURLIN = variables_const().AS_OLDLIN;
}

// AS_Labels: AS_GOSUB (inclusive) .. AS_GO_TO_LINE (exclusive)
void AS_GOSUB() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  //
  // Name normalization: none (assembler label AS_GOSUB kept verbatim).
  // Executes the "AS_GOSUB" command:
  // - Checks stack space for the return frame (7 bytes)
  // - Pushes return frame containing: AS_TXTPTR (2), AS_CURLIN (2),
  // AS_TOKEN_GOSUB (1)
  // - Falls through to shared AS_GO_TO_LINE logic to find and execute the
  // target line
  // - On AS_RETURN, restores execution state from the stack frame

  constexpr std::uint8_t kAS_TOKEN_GOSUB = 0xb0;

  AS_CHKMEMState chkmemState{};
  chkmemState.a = 3;
  chkmemState.stackPointer = theStack().readStackPointer();
  const auto chkmemResult = AS_CHKMEM(chkmemState);
  if (!chkmemResult.ok) {
    return;
  }

  const std::uint16_t textPointer = variables_const().AS_TXTPTR;
  const std::uint16_t currentAS_Line = variables_const().AS_CURLIN;

  theStack().pushWord(textPointer);
  theStack().pushWord(currentAS_Line);
  theStack().pushByte(kAS_TOKEN_GOSUB);

  AS_GO_TO_LINE();
}

// AS_Labels: AS_GO_TO_LINE (inclusive) .. AS_GOTO (exclusive)
void AS_GO_TO_LINE() {
  (void)AS_CHRGOT();
  AS_GOTO();
  AS_NEWSTT();
}

// AS_Labels: AS_GOTO (inclusive) .. AS_RTS_5 (exclusive)
void AS_GOTO() {
  AS_LINGET();
  const std::uint8_t remnOffset = AS_REMN();

  const std::uint8_t currentPage =
      ApplesoftVariables::highByte(variables_const().AS_CURLIN);
  const std::uint8_t targetPage =
      ApplesoftVariables::highByte(variables_const().AS_LINNUM);

  ProgramPointer start{};
  if (currentPage >= targetPage) {
    start = ProgramPointer{variables_const().AS_TXTTAB};
  } else {
    const ProgramPointer textPtr{variables_const().AS_TXTPTR};
    start = textPtr.advanced(static_cast<std::uint16_t>(remnOffset) + 1u);
  }

  if (!AS_FL1(start.address)) {
    AS_ERROR(AS_ERR_UNDEFSTAT);
    return;
  }

  const std::uint16_t destination =
      static_cast<std::uint16_t>(variables_const().AS_LOWTR - 1u);
  variables().AS_TXTPTR = destination;
}

// AS_Labels: AS_RESUME (inclusive) .. AS_JSYN (exclusive)
void AS_RESUME() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // Name normalization: none (assembler label AS_RESUME kept verbatim).
  variables().AS_CURLIN = variables_const().AS_ERRLIN;
  variables().AS_TXTPTR = variables_const().AS_ERRPOS;
  theStack().setStackPointer(variables_const().AS_ERRSTK);
  AS_NEWSTT();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_JSYN (inclusive) .. AS_DEL (exclusive)
// Name normalization: none (assembler label AS_JSYN kept verbatim).
void AS_JSYN() {
  // ROM uses an unconditional jump to AS_SYNERR.
  AS_SYNERR();
}

// AS_Labels: AS_ONERR (inclusive) .. AS_HANDLERR (exclusive)
void AS_ONERR() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // Name normalization: none (assembler label AS_ONERR kept verbatim).
  constexpr std::uint8_t kAS_TOKEN_GOTO = 0xabu;

  AS_SYNCHR(kAS_TOKEN_GOTO);
  variables().AS_TXPSV = variables_const().AS_TXTPTR;

  const std::uint8_t errflg = variables_const().AS_ERRFLG;
  variables().AS_ERRFLG = static_cast<std::uint8_t>((errflg >> 1u) | 0x80u);

  variables().AS_CURLSV = variables_const().AS_CURLIN;
  AS_ADDON(AS_REMN());
}

void AS_RTS_5() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_RTS_5 (inclusive) .. AS_RETURN (exclusive)
  // Name normalization: RTS_5 -> AS_RTS_5 virtual Applesoft prefix only.
  // ROM label RTS_5 is a shared return target for AS_GOTO and AS_POP.
  return;
}

void AS_PULL3() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_PULL3 (inclusive) .. AS_IF (exclusive)
  // Name normalization: none (assembler label AS_PULL3 kept verbatim).
  [[maybe_unused]] const std::uint8_t pulled1 = theStack().popByte();
  [[maybe_unused]] const std::uint8_t pulled2 = theStack().popByte();
  [[maybe_unused]] const std::uint8_t pulled3 = theStack().popByte();
}

std::uint8_t AS_REMN() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_REMN (inclusive) .. AS_PULL3 (exclusive)
  // Name normalization: none (assembler label AS_REMN kept verbatim).

  return ScanAheadOffset(0);
}

void PushForPntFrame() {
  theStack().pushByte(
      ApplesoftVariables::highByte(variables_const().AS_FORPNT));
  theStack().pushByte(ApplesoftVariables::lowByte(variables_const().AS_FORPNT));
  theStack().pushToken(AS_TOKEN_FOR);
}

// AS_Labels: AS_FOR (inclusive) .. AS_STEP (exclusive)
void AS_FOR() {
  constexpr std::uint8_t kAS_TOKEN_TO = 0xc1u;

  variables().AS_SUBFLG = 0x80;
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
  PushRoundedFacAndDispatch();
}

void AS_NEXT() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_NEXT (inclusive) .. AS_FRMNUM (exclusive)
  // Name normalization: none (assembler label AS_NEXT kept verbatim).

  // d0 04 / AS_NEXT_1 jsr AS_PTRGET / AS_NEXT_2 sta AS_FORPNT, sty AS_FORPNT+1
  // No-variable AS_NEXT case is represented by AS_FORPNT+1 = 0.
  if (AS_CHRGOT() == 0u) {
    ApplesoftVariables::setHighByte(variables().AS_FORPNT, 0u);
  } else {
    const std::uint16_t varPtr = AS_PTRGET();
    variables().AS_FORPNT = varPtr;
  }

  // jsr AS_GTFORPNT
  AS_GTFORPNTState gtforpntState{};
  gtforpntState.forpntAS_Lo =
      ApplesoftVariables::lowByte(variables_const().AS_FORPNT);
  gtforpntState.forpntHi =
      ApplesoftVariables::highByte(variables_const().AS_FORPNT);
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
  variables().AS_INDEX = static_cast<std::uint16_t>(
      0x0100u + add_u8(gtforpntResult.x, kStepValueOffsetInForFrame));
  AS_LOAD_FAC_FROM_YA();
  variables().AS_FAC_SIGN =
      theStack().readByteAt(gtforpntResult.x, 9u); // AS_FAC_SIGN
  AS_FADD();
  AS_SETFOR();
  // Assembly: stx DEST (X = frame_x + 10) then ldy #>STACK; jsr FCOMP2.
  // Set AS_DEST to the full 16-bit address of the end value in the FOR frame.
  variables().AS_DEST =
      static_cast<std::uint16_t>(0x0100u + add_u8(gtforpntResult.x, 10u));
  AS_FCOMP2();

  if (!AS_NEXT_shouldTerminateLoop(gtforpntResult.x)) {
    // Restore line/AS_TXTPTR from AS_FOR frame and jump AS_NEWSTT.
    const std::uint16_t restoredAS_Line =
        readStackWordAt(gtforpntResult.x, 15u, 16u);
    const std::uint16_t restoredTextPointer =
        readStackWordAt(gtforpntResult.x, 18u, 17u);
    variables().AS_CURLIN = restoredAS_Line;
    variables().AS_TXTPTR = restoredTextPointer;
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

// AS_Labels: AS_POP (inclusive) .. AS_RETURN (exclusive)
void AS_POP() {
  constexpr std::uint8_t kAS_TOKEN_GOSUB = 0xb0;

  if (!IsStatementEndOfParsedInput()) {
    AS_RTS_5();
    return;
  }

  ApplesoftVariables::setLowByte(variables().AS_FORPNT, 0xffu);

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

// AS_Labels: AS_RETURN (inclusive) .. AS_DATA (exclusive)
void AS_RETURN() {
  (void)theStack().popByte();
  const std::uint8_t currentAS_LineLo = theStack().popByte();

  if (ReturnWasFromAS_POPContext()) {
    AS_PULL3();
    return;
  }

  const std::uint8_t currentAS_LineHi = theStack().popByte();
  const std::uint8_t textPointerAS_Lo = theStack().popByte();
  const std::uint8_t textPointerHi = theStack().popByte();

  variables().AS_CURLIN =
      ApplesoftVariables::makeWord(currentAS_LineLo, currentAS_LineHi);
  variables().AS_TXTPTR =
      ApplesoftVariables::makeWord(textPointerAS_Lo, textPointerHi);
}

// AS_Labels: AS_STEP (inclusive) .. AS_NEWSTT (exclusive)
void AS_STEP() {
  constexpr std::uint8_t kAS_TOKEN_STEP = 0xc7u;

  for (std::uint8_t i = 0; i < kPackedFloatByteCount; ++i) {
    WriteProgramByte(static_cast<std::uint16_t>(kConOneScratchAddress + i),
                     kConOnePacked[i]);
  }
  variables().AS_INDEX = kConOneScratchAddress;
  AS_LOAD_FAC_FROM_YA();
  if (AS_CHRGOT() == kAS_TOKEN_STEP) {
    AS_CHRGET();
    AS_FRMNUM();
  }

  const std::int8_t stepSign = AS_SIGN();
  PushFacSignReturnAddress(static_cast<std::uint8_t>(stepSign));
  PushForPntFrame();
  AS_NEWSTT();
}

// AS_Labels: AS_NEWSTT (inclusive) .. AS_TRACE_ (exclusive)
void AS_NEWSTT() {
  variables().AS_REMSTK = theStack().readStackPointer();

  if (AS_ISCNTC()) {
    return;
  }

  if (ApplesoftVariables::highByte(variables_const().AS_CURLIN) != 0xffu) {
    variables().AS_OLDTEXT = variables_const().AS_TXTPTR;
  } else {
    variables().AS_OLDTEXT = 0;
  }

  if (IsEndOfAS_LineAtTextPointer()) {
    if (IsEndOfProgramAtTextPointer()) {
      AS_GOEND();
      return;
    }
  }

  variables().AS_CURLIN = ReadAS_LineNumberFromTextPointer();
  AdvanceTextPointerToNextAS_Line();
  AS_TRACE_();
}

// AS_Labels: AS_TRACE_ (inclusive) .. AS_GOEND (exclusive)
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
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_GOEND (inclusive) .. AS_EXECUTE_STATEMENT (exclusive)
  // Name normalization: none (assembler label AS_GOEND kept verbatim).
  // End-of-program path in AS_NEWSTT jumps into AS_END4 with carry clear, which
  // restarts without printing BREAK. Model that directly here.
  AS_RESTART();
}

bool IsEndOfAS_LineAtTextPointer() {
  // Source: AS_NEWSTT inline — ldy #0 / lda (AS_TXTPTR),Y: end-of-statement
  // when byte is 0.
  return ReadProgramByte(variables_const().AS_TXTPTR) == 0u;
}

bool IsEndOfProgramAtTextPointer() {
  // Source: AS_NEWSTT inline — ldy #2 / lda (AS_TXTPTR),Y: next-line link high
  // byte; if zero the program has ended (null forward pointer).
  return ReadProgramByte(static_cast<std::uint16_t>(
             variables_const().AS_TXTPTR + 2u)) == 0u;
}

std::uint16_t ReadAS_LineNumberFromTextPointer() {
  // Source: AS_NEWSTT inline — reads AS_CURLIN from (AS_TXTPTR)+3 and
  // (AS_TXTPTR)+4. Memory layout at AS_TXTPTR when it sits on an EOL 0x00:
  //   [0] = 0x00 (EOL), [1] = link.lo, [2] = link.hi, [3] = lineno.lo, [4] =
  //   lineno.hi.
  const std::uint16_t txtptr = variables_const().AS_TXTPTR;
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
  variables().AS_TXTPTR =
      static_cast<std::uint16_t>(variables_const().AS_TXTPTR + 4u);
}

bool IsRunningMode() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_TRACE_ (inclusive) .. AS_EXECUTE_STATEMENT (exclusive)
  // Name normalization: helper name chosen for the inline AS_TRACE_ predicate.
  // AS_TRACE_ checks AS_CURLIN+1 and only traces when non-zero (running mode).
  return ApplesoftVariables::highByte(variables_const().AS_CURLIN) != 0u;
}

bool IsTraceEnabled() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_TRACE_ (inclusive) .. AS_EXECUTE_STATEMENT (exclusive)
  // Name normalization: helper name chosen for the inline AS_TRACE_ predicate.
  // `bit AS_TRCFLG` + `bpl` means tracing is enabled when AS_TRCFLG bit 7 is
  // set.
  return (variables_const().AS_TRCFLG & 0x80u) != 0u;
}

void AS_EXECUTE_STATEMENT() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
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
  // SourceMaterial/Combo/asrom.lst
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
  // SourceMaterial/Combo/asrom.lst
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
  // SourceMaterial/Combo/asrom.lst
  // AS_Labels: AS_IF (inclusive) .. AS_REM (exclusive)
  // Name normalization: none (assembler label AS_IF kept verbatim).

  constexpr std::uint8_t kAS_TOKEN_GOTO = 0xabu;
  constexpr std::uint8_t kAS_TOKEN_THEN = 0xc4u;

  AS_FRMEVL();
  if (AS_CHRGOT() != kAS_TOKEN_GOTO) {
    AS_SYNCHR(kAS_TOKEN_THEN);
  }

  if (variables_const().AS_FAC[0] != 0u) {
    AS_IF_TRUE();
    return;
  }

  // False AS_IF falls through to AS_REM in ROM.
  AS_REM();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_REM (inclusive) .. AS_IF_TRUE (exclusive)
// Name normalization: none (assembler label AS_REM kept verbatim).
void AS_REM() {
  const std::uint8_t offset = AS_REMN();
  AS_ADDON(offset);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_IF_TRUE (inclusive) .. AS_ONGOTO (exclusive)
// Name normalization: none (assembler label AS_IF_TRUE kept verbatim).
void AS_IF_TRUE() {
  if (AS_CHRGOT() >= kTokenBase) {
    AS_EXECUTE_STATEMENT();
    return;
  }

  AS_GOTO();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ONGOTO (inclusive) .. AS_LINGET (exclusive)
// Name normalization: none (assembler label AS_ONGOTO kept verbatim).
void AS_ONGOTO() {
  constexpr std::uint8_t kAS_TOKEN_GOSUB = 0xb0u;
  constexpr std::uint8_t kAS_TOKEN_GOTO = 0xabu;

  const std::uint8_t token = AS_GETBYT();
  if (token != kAS_TOKEN_GOSUB && token != kAS_TOKEN_GOTO) {
    AS_SYNERR();
    return;
  }

  while (true) {
    const std::uint8_t selector = variables_const().AS_FAC[4];
    variables().AS_FAC[4] = static_cast<std::uint8_t>(selector - 1u);

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
