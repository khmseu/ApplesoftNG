#include "core/asm_port_statements.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_clear.hpp"
#include "core/asm_port_control_flow.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_handling.hpp"
#include "core/asm_port_graphics.hpp"
#include "core/asm_port_parser.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_token_name_table.hpp"
#include "core/io_ports.hpp"
#include "platform/asm_port_outdo.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace applesoft::asm_port {

constexpr std::size_t kTokenCount = 107;
constexpr std::uint8_t kTokenBase = 0x80u;

// AS_Labels: AS_NEW (inclusive) .. AS_SCRTCH (exclusive)
bool AS_NEW_impl() {
  if (!IsStatementEndOfParsedInput()) {
    return false;
  }

  AS_SCRTCH_impl();
  return true;
}

// AS_Labels: AS_SCRTCH (inclusive) .. AS_SETPTRS (exclusive)
void AS_SCRTCH_impl() {
  const std::uint16_t txtTabAddr = variables_const().AS_TXTTAB;
  variables().AS_LOCK = 0;
  WriteProgramByte(txtTabAddr, 0);
  WriteProgramByte(static_cast<std::uint16_t>(txtTabAddr + 1u), 0);

  const std::uint16_t nextFree = static_cast<std::uint16_t>(txtTabAddr + 2u);
  variables().AS_VARTAB = nextFree;
  variables().AS_PRGEND = nextFree;
  variables().AS_FRETOP = variables_const().AS_MEMSIZ;
  variables().AS_ARYTAB = variables_const().AS_VARTAB;
  variables().AS_STREND = variables_const().AS_VARTAB;

  AS_SETPTRS_impl();
}

// AS_Labels: AS_SETPTRS (inclusive) .. AS_CLEAR (exclusive)
bool AS_SETPTRS_impl() {
  AS_STXTPT();
  AS_CLEAR();
  return true;
}

bool AS_NEW() { return AS_NEW_impl(); }

bool AS_SETPTRS() { return AS_SETPTRS_impl(); }

void AS_SCRTCH() { AS_SCRTCH_impl(); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_RUN (inclusive) .. AS_GOSUB (exclusive)
// Name normalization: none (assembler label AS_RUN kept verbatim).
void AS_RUN() {
  std::uint8_t curlinHi =
      ApplesoftVariables::highByte(variables_const().AS_CURLIN);
  ApplesoftVariables::setHighByte(variables().AS_CURLIN,
                                  static_cast<std::uint8_t>(curlinHi - 1));

  const std::uint8_t currentChar = AS_CHRGOT();
  if (currentChar == 0) {
    AS_SETPTRS();
    return;
  }

  AS_CLEARC();
  AS_GO_TO_LINE();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PEEK (inclusive) .. AS_POKE (exclusive)
// Name normalization: none (assembler label AS_PEEK kept verbatim).
//
// Preserves AS_LINNUM across the peeked read while converting the loaded byte
// to FAC via AS_SNGFLT.
void AS_PEEK() {
  const std::uint16_t savedAS_Linnum = variables_const().AS_LINNUM;
  AS_GETADR();
  const std::uint8_t value = ReadProgramByte(variables_const().AS_LINNUM);
  variables().AS_LINNUM = savedAS_Linnum;
  AS_SNGFLT(value);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_POKE (inclusive) .. AS_WAIT (exclusive)
// Name normalization: none (assembler label AS_POKE kept verbatim).
void AS_POKE() {
  const std::uint8_t value = AS_GTNUM();
  WriteProgramByte(variables_const().AS_LINNUM, value);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_WAIT (inclusive) .. AS_FADDH (exclusive)
// Name normalization: none (assembler label AS_WAIT kept verbatim).
void AS_WAIT() {
  const std::uint8_t mask = AS_GTNUM();
  ApplesoftVariables::setLowByte(variables().AS_FORPNT, mask);

  std::uint8_t xorMask = 0u;
  if (AS_CHRGOT() != 0u) {
    xorMask = AS_COMBYTE();
  }
  ApplesoftVariables::setHighByte(variables().AS_FORPNT, xorMask);

  while (true) {
    const std::uint8_t value = ReadProgramByte(variables_const().AS_LINNUM);
    const std::uint8_t masked =
        static_cast<std::uint8_t>((value ^ xorMask) & mask);
    if (masked != 0u) {
      AS_RTS_10();
      return;
    }
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_RTS_10 (inclusive) .. AS_FADDH (exclusive)
// Name normalization: RTS_10 -> AS_RTS_10 virtual Applesoft prefix only.
// ROM label RTS_10 is a shared return target for AS_WAIT.
void AS_RTS_10() { return; }

struct TokenMatch {
  std::uint8_t code;
  std::uint8_t length;
  std::string_view name;
};

static std::uint8_t read_AS_INPUT_BUFFER(std::uint8_t index) {
  return variables_const().pointer(0x0200u).read(index);
}

static void write_AS_INPUT_BUFFER_minus_5(std::uint8_t index,
                                          std::uint8_t value) {
  variables().pointer(0x01fbu).write(value, index);
}

static void SetTextPointerToInputBufferMinus1() {
  // Sets TXTPTR to the byte just before the input buffer ($01FF), matching the
  // ROM convention where CHRGET advances past this address to reach the first
  // byte of INPUT_BUFFER at $0200.
  SetTextPointer(ApplesoftVariables::ADDR_AS_INPUT_BUFFER_MINUS_1);
}

static std::optional<TokenMatch> MatchToken(std::uint8_t index) {
  std::optional<TokenMatch> best;

  for (std::size_t i = 0; i < kTokenCount; ++i) {
    const std::string_view token = AS_TOKEN_NAME_TABLE(i);
    if (token.empty()) {
      continue;
    }

    std::uint8_t current = index;
    bool matched = true;
    for (char expected : token) {
      if (read_AS_INPUT_BUFFER(current) !=
          static_cast<std::uint8_t>(expected)) {
        matched = false;
        break;
      }
      ++current;
    }

    if (!matched) {
      continue;
    }

    if (token == "AT") {
      const std::uint8_t next = read_AS_INPUT_BUFFER(current);
      if (next == static_cast<std::uint8_t>('N') ||
          next == static_cast<std::uint8_t>('O')) {
        continue;
      }
    }

    const std::uint8_t tokenCode =
        static_cast<std::uint8_t>(kTokenBase + static_cast<std::uint8_t>(i));
    if (!best || token.size() > best->length) {
      best =
          TokenMatch{tokenCode, static_cast<std::uint8_t>(token.size()), token};
    }
  }

  return best;
}

static std::uint8_t ScanAheadOffsetForData(std::uint8_t terminator) {
  variables().AS_CHARAC = terminator;
  std::uint8_t offset = 0;
  variables().AS_ENDCHR = 0;

  while (true) {
    const std::uint8_t previousEnd = variables_const().AS_ENDCHR;
    const std::uint8_t previousCharac = variables_const().AS_CHARAC;
    variables().AS_CHARAC = previousEnd;
    variables().AS_ENDCHR = previousCharac;

    while (true) {
      const std::uint16_t textPtr = variables_const().AS_TXTPTR;
      const std::uint8_t ch = variables_const().pointer(textPtr).read(offset);
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

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ADDON (inclusive) .. AS_DATAN (exclusive)
// Name normalization: none (assembler label AS_ADDON kept verbatim).
void AS_ADDON(std::uint8_t offset) {

  const std::uint16_t textPtr = variables_const().AS_TXTPTR;
  variables().AS_TXTPTR = static_cast<std::uint16_t>(textPtr + offset);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_DATAN (inclusive) .. AS_REMN (exclusive)
// Name normalization: none (assembler label AS_DATAN kept verbatim).
std::uint8_t AS_DATAN() {

  return ScanAheadOffsetForData(static_cast<std::uint8_t>(':'));
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_DATA (inclusive) .. AS_ADDON (exclusive)
// Name normalization: none (assembler label AS_DATA kept verbatim).
void AS_DATA() {

  const std::uint8_t offset = AS_DATAN();
  AS_ADDON(offset);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_LET (inclusive) .. AS_LET2 (exclusive)
// Name normalization: none (assembler label AS_LET kept verbatim).
void AS_LET() {

  constexpr std::uint8_t kTOKEN_EQUAL = 0xd0;

  const std::uint16_t variablePtr = AS_PTRGET();
  variables().AS_FORPNT = variablePtr;

  AS_SYNCHR(kTOKEN_EQUAL);

  const std::uint8_t savedValTyp = variables_const().AS_VALTYP;
  const std::uint8_t savedValTypPlus1 = variables_const().AS_VALTYP_PLUS_1;

  AS_FRMEVL();

  if (AS_CHKVAL(savedValTyp)) {
    // AS_LET_STRING branch falls through to AS_PUTSTR in ROM.
    AS_PUTSTR();
    return;
  }

  // Explicitly model AS_LET -> AS_LET2 fall-through.
  AS_LET2(savedValTypPlus1);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_LET2 (inclusive) .. AS_PUTSTR (exclusive)
// Name normalization: none (assembler label AS_LET2 kept verbatim).
void AS_LET2(std::uint8_t savedValTypPlus1) {

  // Positive means real variable; ROM jumps directly to AS_SETFOR.
  if ((savedValTypPlus1 & 0x80u) == 0u) {
    AS_SETFOR();
    return;
  }

  AS_ROUND_FAC();
  AS_AYINT();

  const std::uint16_t forPtr = variables_const().AS_FORPNT;
  auto forPtrByte = variables().pointer(forPtr);
  forPtrByte.write(variables_const().AS_FAC[3]);
  forPtrByte.write(variables_const().AS_FAC[4], 1u);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PUTSTR (inclusive) .. AS_PR_STRING (exclusive)
// Name normalization: none (assembler label AS_PUTSTR kept verbatim).
void AS_PUTSTR() {

  constexpr std::uint8_t kAS_FAC_PLUS_3 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 3u);

  const std::uint16_t facDescriptor = variables_const().AS_FAC_WORD_3;
  const auto facDescriptorPtr = variables_const().pointer(facDescriptor);
  auto readDescriptorByte = [&](std::uint8_t offset) {
    return facDescriptorPtr.read(offset);
  };

  std::uint16_t descriptorPointer = variables_const().AS_FAC_WORD_3;

  const std::uint8_t descDataHigh = readDescriptorByte(2);
  const std::uint8_t fretopHigh =
      ApplesoftVariables::highByte(variables_const().AS_FRETOP);

  bool useExistingDescriptor = false;
  bool descriptorIsVariable = false;

  if (descDataHigh < fretopHigh) {
    useExistingDescriptor = true;
  } else if (descDataHigh == fretopHigh) {
    const std::uint8_t descDataAS_Low = readDescriptorByte(1);
    if (descDataAS_Low <
        ApplesoftVariables::lowByte(variables_const().AS_FRETOP)) {
      useExistingDescriptor = true;
    }
  }

  if (!useExistingDescriptor) {
    if (descriptorPointer >= variables_const().AS_VARTAB) {
      descriptorIsVariable = true;
    }
  }

  if (descriptorIsVariable) {
    AS_STRINI(readDescriptorByte(0));
    variables().AS_STRNG1 = variables_const().AS_DSCPTR;
    AS_MOVINS();
    descriptorPointer = kAS_FAC_PLUS_3;
  }

  variables().AS_DSCPTR = descriptorPointer;

  (void)AS_FRETMS(descriptorPointer);

  const std::uint16_t source = variables_const().AS_DSCPTR;
  const std::uint16_t dest = variables_const().AS_FORPNT;
  const auto sourcePtr = variables_const().pointer(source);
  auto destPtr = variables().pointer(dest);
  for (std::uint8_t i = 0; i < 3; ++i) {
    destPtr.write(sourcePtr.read(i), i);
  }
}

static void DeleteExistingAS_Line() {
  // C++ helper: implements the inline delete/shift block of AS_NUMBERED_LINE
  // (d471–d4b4), extracted for clarity.  Not a ROM entry point.

  const std::uint16_t lowtr = variables_const().AS_LOWTR;
  const std::uint16_t nextAS_Line = ApplesoftVariables::makeWord(
      variables_const().readByte(lowtr),
      variables_const().readByte(static_cast<std::uint16_t>(lowtr + 1u)));

  const std::uint16_t lineSize =
      static_cast<std::uint16_t>(nextAS_Line - lowtr);
  const std::uint16_t vartab = variables_const().AS_VARTAB;

  const std::uint16_t moveCount =
      static_cast<std::uint16_t>(vartab - nextAS_Line);
  for (std::uint16_t i = 0; i < moveCount; ++i) {
    variables().writeByte(static_cast<std::uint16_t>(lowtr + i),
                          variables_const().readByte(
                              static_cast<std::uint16_t>(nextAS_Line + i)));
  }

  variables().AS_VARTAB = static_cast<std::uint16_t>(vartab - lineSize);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PUT_NEW_LINE (inclusive) .. AS_FIX_LINKS (exclusive)
// Name normalization: renamed to InsertNewAS_Line in C++; corresponds to
// AS_PUT_NEW_LINE at d4b5.
static void InsertNewAS_Line() {
  constexpr std::uint16_t kTokenBuf =
      static_cast<std::uint16_t>(ApplesoftVariables::ADDR_AS_INPUT_BUFFER - 5u);

  if (variables_const().AS_INPUT_BUFFER_PAGE[0] == 0u) {
    return;
  }

  variables().AS_FRETOP = variables_const().AS_MEMSIZ;

  std::uint16_t tokenAS_Len = 0;
  while (variables_const().readByte(
             static_cast<std::uint16_t>(kTokenBuf + tokenAS_Len)) != 0u) {
    ++tokenAS_Len;
  }

  const std::uint16_t lineSize = static_cast<std::uint16_t>(tokenAS_Len + 5u);
  const std::uint16_t lowtr = variables_const().AS_LOWTR;
  const std::uint16_t vartab = variables_const().AS_VARTAB;

  for (std::uint16_t i = vartab; i > lowtr; --i) {
    const std::uint16_t src = static_cast<std::uint16_t>(i - 1u);
    variables().writeByte(static_cast<std::uint16_t>(src + lineSize),
                          variables_const().readByte(src));
  }

  const std::uint16_t linnum = variables_const().AS_LINNUM;
  variables().writeByte(lowtr, 0u);
  variables().writeByte(static_cast<std::uint16_t>(lowtr + 1u), 0u);
  variables().writeByte(static_cast<std::uint16_t>(lowtr + 2u),
                        ApplesoftVariables::lowByte(linnum));
  variables().writeByte(static_cast<std::uint16_t>(lowtr + 3u),
                        ApplesoftVariables::highByte(linnum));

  for (std::uint16_t i = 0; i <= tokenAS_Len; ++i) {
    variables().writeByte(
        static_cast<std::uint16_t>(lowtr + 4u + i),
        variables_const().readByte(static_cast<std::uint16_t>(kTokenBuf + i)));
  }

  const std::uint16_t newVartab = static_cast<std::uint16_t>(vartab + lineSize);
  variables().AS_VARTAB = newVartab;
  variables().AS_STREND = newVartab;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PARSE_INPUT_LINE (inclusive) .. AS_FNDLIN (exclusive)
// Name normalization: none (assembler label AS_PARSE_INPUT_LINE kept verbatim).
// AS_PARSE (d56c) falls through from AS_PARSE_INPUT_LINE (d559); both address
// ranges are implemented by this single C++ function.
void AS_PARSE_INPUT_LINE() {
  std::uint8_t inputIndex = 0;
  std::uint8_t outputIndex = 0;
  bool inRem = false;

  while (true) {
    const std::uint8_t ch = read_AS_INPUT_BUFFER(inputIndex);
    if (ch == 0u) {
      break;
    }

    if (inRem) {
      write_AS_INPUT_BUFFER_minus_5(outputIndex++, ch);
      ++inputIndex;
      continue;
    }

    if (ch == static_cast<std::uint8_t>(' ')) {
      ++inputIndex;
      continue;
    }

    if (ch == static_cast<std::uint8_t>(0x22u)) {
      write_AS_INPUT_BUFFER_minus_5(outputIndex++, ch);
      ++inputIndex;
      while (true) {
        const std::uint8_t quoteChar = read_AS_INPUT_BUFFER(inputIndex);
        if (quoteChar == 0u) {
          break;
        }
        write_AS_INPUT_BUFFER_minus_5(outputIndex++, quoteChar);
        ++inputIndex;
        if (quoteChar == static_cast<std::uint8_t>(0x22u)) {
          break;
        }
      }
      continue;
    }

    const auto token = MatchToken(inputIndex);
    if (token.has_value()) {
      const TokenMatch match = *token;
      write_AS_INPUT_BUFFER_minus_5(outputIndex++, match.code);

      if (match.name == "AS_REM") {
        inputIndex += match.length;
        inRem = true;
        continue;
      }

      inputIndex += match.length;
      continue;
    }

    write_AS_INPUT_BUFFER_minus_5(outputIndex++, ch);
    ++inputIndex;
  }

  write_AS_INPUT_BUFFER_minus_5(outputIndex, 0u);
  SetTextPointerToInputBufferMinus1();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_NUMBERED_LINE (inclusive) .. AS_PUT_NEW_LINE (exclusive)
// Name normalization: renamed to HandleNumberedAS_Line in C++; implements the
// full AS_NUMBERED_LINE entry point at d45c (squash VARTAB, LINGET, parse,
// FNDLIN, optional delete, then fall through to AS_PUT_NEW_LINE).
void HandleNumberedAS_Line() {
  AS_LINGET();
  AS_PARSE_INPUT_LINE();

  if (AS_FNDLIN()) {
    DeleteExistingAS_Line();
  }

  InsertNewAS_Line();
  AS_FIX_LINKS();
}

std::uint16_t GetTextTablePointer() { return variables_const().AS_TXTTAB; }

bool IsEndOfProgram(std::uint16_t current) { return current == 0u; }

static std::uint16_t AdvanceToNextAS_Line(std::uint16_t current) {
  // The original AS_FIX_LINKS routine scans from the current line until it
  // finds the end-of-line marker, then computes the address of the next line.
  std::uint16_t offset = 4u;
  while (variables_const().readByte(
             static_cast<std::uint16_t>(current + offset)) != 0u) {
    ++offset;
  }

  return static_cast<std::uint16_t>(current + offset + 1u);
}

static void WriteForwardPointer(std::uint16_t current, std::uint16_t next) {
  variables().writeByte(current, ApplesoftVariables::lowByte(next));
  variables().writeByte(static_cast<std::uint16_t>(current + 1u),
                        ApplesoftVariables::highByte(next));
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FIX_LINKS (inclusive) .. AS_INLIN (exclusive)
// Name normalization: none (assembler label AS_FIX_LINKS kept verbatim).
void AS_FIX_LINKS() {
  AS_SETPTRS();

  std::uint16_t current = GetTextTablePointer();
  while (true) {
    if (IsEndOfProgram(current)) {
      AS_RESTART();
      return;
    }

    const std::uint16_t next = AdvanceToNextAS_Line(current);
    WriteForwardPointer(current, next);
    current = next;
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FNDLIN (inclusive) .. AS_FL1 (exclusive)
// Name normalization: none (assembler label AS_FNDLIN kept verbatim).
bool AS_FNDLIN() {
  // Assembler falls through from AS_FNDLIN directly into AS_FL1 with
  // A=AS_TXTTAB, X=AS_TXTTAB+1.
  return AS_FL1(variables_const().AS_TXTTAB);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FL1 (inclusive) .. AS_NEW (exclusive)
// Name normalization: none (assembler label AS_FL1 kept verbatim).
bool AS_FL1(std::uint16_t startAddress) {
  const std::uint8_t targetAS_Lo =
      ApplesoftVariables::lowByte(variables_const().AS_LINNUM);
  const std::uint8_t targetHi =
      ApplesoftVariables::highByte(variables_const().AS_LINNUM);

  std::uint16_t current = startAddress;

  while (true) {
    variables().AS_LOWTR = current;

    const std::uint8_t nextHi =
        variables_const().readByte(static_cast<std::uint16_t>(current + 1u));
    if (nextHi == 0u) {
      return false;
    }

    const std::uint8_t lineHi =
        variables_const().readByte(static_cast<std::uint16_t>(current + 3u));
    if (targetHi < lineHi) {
      return false;
    }

    if (targetHi == lineHi) {
      const std::uint8_t lineAS_Lo =
          variables_const().readByte(static_cast<std::uint16_t>(current + 2u));
      if (targetAS_Lo < lineAS_Lo) {
        return false;
      }
      if (targetAS_Lo == lineAS_Lo) {
        return true;
      }
    }

    const std::uint8_t nextAS_Lo = variables_const().readByte(current);
    current = ApplesoftVariables::makeWord(nextAS_Lo, nextHi);
  }
}

bool AS_FL1(std::uint8_t startAS_Lo, std::uint8_t startHi) {
  return AS_FL1(ApplesoftVariables::makeWord(startAS_Lo, startHi));
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_DEL (inclusive) .. AS_GR (exclusive)
// Name normalization: none (assembler label AS_DEL kept verbatim).
void AS_DEL() {

  const std::uint16_t prgend = variables_const().AS_PRGEND;
  variables().AS_VARTAB = prgend;

  AS_LINGET();
  AS_FNDLIN();
  variables().AS_DEST = variables_const().AS_LOWTR;

  AS_SYNCHR(static_cast<std::uint8_t>(','));
  AS_LINGET();

  std::uint8_t linnumAS_Lo =
      ApplesoftVariables::lowByte(variables_const().AS_LINNUM);
  if (linnumAS_Lo == 0xffu) {
    ApplesoftVariables::setLowByte(variables().AS_LINNUM, 0u);
    const std::uint8_t linnumHi =
        ApplesoftVariables::highByte(variables_const().AS_LINNUM);
    ApplesoftVariables::setHighByte(variables().AS_LINNUM,
                                    static_cast<std::uint8_t>(linnumHi + 1u));
  } else {
    ApplesoftVariables::setLowByte(variables().AS_LINNUM,
                                   static_cast<std::uint8_t>(linnumAS_Lo + 1u));
  }

  AS_FNDLIN();

  const std::uint16_t lowtr = variables_const().AS_LOWTR;
  const std::uint16_t dest = variables_const().AS_DEST;
  if (lowtr < dest) {
    return;
  }

  const std::uint16_t vartab = variables_const().AS_VARTAB;
  std::uint16_t source = lowtr;
  std::uint16_t destination = dest;
  while (source < vartab) {
    const std::uint8_t byteVal = variables_const().readByte(source);
    variables().writeByte(destination, byteVal);
    ++source;
    ++destination;
  }

  const std::uint16_t deletedSize = static_cast<std::uint16_t>(lowtr - dest);
  const std::uint16_t newVartab =
      static_cast<std::uint16_t>(vartab - deletedSize);
  variables().AS_VARTAB = newVartab;

  AS_FIX_LINKS();
}

// AS_Labels: AS_LIST (inclusive) .. AS_FOR (exclusive)
// Included ROM labels: AS_GETCHR, AS_LIST_4
void AS_LIST() {
  // Source:
  // SourceMaterial/Combo/asrom.lst
  // Name normalization: none (assembler label AS_LIST kept verbatim).

  if (!IsStatementEndOfParsedInput()) {
    return;
  }

  AS_LINGET();
  AS_FNDLIN();

  const std::uint8_t rangeChar = AS_CHRGOT();
  if (rangeChar == static_cast<std::uint8_t>('-') ||
      rangeChar == static_cast<std::uint8_t>(',')) {
    AS_CHRGET();
    AS_LINGET();
  }

  std::uint16_t endRange = variables_const().AS_LINNUM;
  if (endRange == 0u) {
    endRange = 0xffffu;
  }

  std::uint16_t current = variables_const().AS_LOWTR;
  while (current != 0u) {
    if (AS_ISCNTC()) {
      break;
    }

    const std::uint16_t currentAS_Line = ApplesoftVariables::makeWord(
        variables_const().readByte(static_cast<std::uint16_t>(current + 2u)),
        variables_const().readByte(static_cast<std::uint16_t>(current + 3u)));
    if (currentAS_Line > endRange) {
      break;
    }

    AS_CRDO();
    variables().AS_CURLIN = currentAS_Line;
    AS_LINPRT();
    AS_OUTDO(' ');

    std::uint16_t offset = 4u;
    while (true) {
      const std::uint8_t ch = variables_const().readByte(
          static_cast<std::uint16_t>(current + offset));
      if (ch == 0u) {
        break;
      }

      if (ch < 0x80u) {
        AS_OUTDO(static_cast<std::uint8_t>(ch & 0x7fu));
      } else {
        // Expand token
        AS_OUTDO(' ');
        std::string_view tokenName = AS_TOKEN_NAME_TABLE(ch - 0x80u);
        for (char tc : tokenName) {
          AS_OUTDO(static_cast<std::uint8_t>(tc));
        }
        AS_OUTDO(' ');
      }
      ++offset;
    }

    // current = link (low, high)
    current = ApplesoftVariables::makeWord(
        variables_const().readByte(current),
        variables_const().readByte(static_cast<std::uint16_t>(current + 1u)));
    variables().AS_LOWTR = current;
  }

  AS_CRDO();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_STORE (inclusive) .. AS_L_STORE_1 (exclusive)
// Name normalization: none (assembler label AS_STORE kept verbatim).
void AS_STORE() {
  AS_GETARYPT();

  const std::uint16_t descriptorAddress = variables_const().AS_LOWTR;
  const std::uint8_t sizeHi = variables_const().readByte(
      static_cast<std::uint16_t>(descriptorAddress + 3u));
  std::uint8_t sizeLo = variables_const().readByte(
      static_cast<std::uint16_t>(descriptorAddress + 2u));

  bool borrow = (sizeLo < 1u);
  sizeLo = static_cast<std::uint8_t>(sizeLo - 1u);
  std::uint8_t adjustedHi = sizeHi;
  if (borrow) {
    adjustedHi = static_cast<std::uint8_t>(adjustedHi - 1u);
  }

  ApplesoftVariables::setLowByte(variables().AS_LINNUM, sizeLo);
  ApplesoftVariables::setHighByte(variables().AS_LINNUM, adjustedHi);

  AS_L_STORE_1();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_L_STORE_1 (inclusive) .. AS_RECALL (exclusive)
// Name normalization: none (assembler label AS_L_STORE_1 kept verbatim).
void AS_L_STORE_1() {
  MON_WRITE();
  AS_TAPEPNT();
  MON_WRITE();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_RECALL (inclusive) .. AS_L_RECALL_1 (exclusive)
// Name normalization: none (assembler label AS_RECALL kept verbatim).
void AS_RECALL() {
  AS_GETARYPT();
  MON_READ();

  const std::uint16_t descriptorAddress = variables_const().AS_LOWTR;
  const std::uint16_t incomingSize = ApplesoftVariables::makeWord(
      variables_const().readByte(
          static_cast<std::uint16_t>(descriptorAddress + 2u)),
      variables_const().readByte(
          static_cast<std::uint16_t>(descriptorAddress + 3u)));
  if (incomingSize < variables_const().AS_LINNUM) {
    AS_MEMERR();
    return;
  }

  AS_L_RECALL_1();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_L_RECALL_1 (inclusive) .. AS_HGR2 (exclusive)
// Name normalization: none (assembler label AS_L_RECALL_1 kept verbatim).
void AS_L_RECALL_1() {
  AS_TAPEPNT();
  MON_READ();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GETARYPT (inclusive) .. AS_HTAB (exclusive)
// Name normalization: none (assembler label AS_GETARYPT kept verbatim).
void AS_GETARYPT() {
  variables().AS_SUBFLG = 0x40u;
  (void)AS_PTRGET();
  variables().AS_SUBFLG = 0u;
  AS_VARTIO();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_TAPEPNT (inclusive) .. AS_GETARYPT (exclusive)
// Name normalization: none (assembler label AS_TAPEPNT kept verbatim).
void AS_TAPEPNT() {
  const std::uint16_t lowtr = variables_const().AS_LOWTR;
  const std::uint16_t linnum = variables_const().AS_LINNUM;

  // MON_A2 points to LOWTR + LINNUM for tape block transfer bounds.
  variables().MON_A2 = static_cast<std::uint16_t>(lowtr + linnum);

  // ROM loads descriptor byte at LOWTR+4 before GETARY2.
  variables().AS_NUMDIM =
      variables_const().readByte(static_cast<std::uint16_t>(lowtr + 4u));
  AS_GETARY2();

  // HIGHDS ($94/$95) maps to AS_ARYPNT in current variable modeling.
  variables().MON_A1 = variables_const().AS_ARYPNT;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_SAVE (inclusive) .. AS_LOAD (exclusive)
// Name normalization: none (assembler label AS_SAVE kept verbatim).
void AS_SAVE() {

  const std::uint16_t programEnd = variables_const().AS_PRGEND;
  const std::uint16_t textTable = variables_const().AS_TXTTAB;
  const std::uint16_t programAS_Length =
      static_cast<std::uint16_t>(programEnd - textTable);
  variables().AS_LINNUM = programAS_Length;

  AS_VARTIO();
  MON_WRITE();
  AS_PROGIO();
  MON_WRITE();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_LOAD (inclusive) .. AS_VARTIO (exclusive)
// Name normalization: none (assembler label AS_LOAD kept verbatim).
void AS_LOAD() {

  AS_VARTIO();
  MON_READ();

  const std::uint16_t textTable = variables_const().AS_TXTTAB;
  const std::uint16_t programAS_Length = variables_const().AS_LINNUM;
  variables().AS_VARTAB =
      static_cast<std::uint16_t>(textTable + programAS_Length);

  variables().AS_LOCK = variables_const().AS_TEMPPT;

  AS_PROGIO();
  MON_READ();

  if ((variables_const().AS_LOCK & 0x80u) != 0u) {
    (void)AS_SETPTRS();
    return;
  }

  AS_FIX_LINKS();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_VARTIO (inclusive) .. AS_PROGIO (exclusive)
// Name normalization: none (assembler label AS_VARTIO kept verbatim).
void AS_VARTIO() {

  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
  constexpr std::uint8_t kAS_TEMPPT = ApplesoftVariables::ZP_AS_TEMPPT;
  variables().MON_A1 = ApplesoftVariables::makeWord(kAS_LINNUM, 0x00u);
  variables().MON_A2 = ApplesoftVariables::makeWord(kAS_TEMPPT, 0x00u);
  variables().AS_LOCK = 0x00;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PROGIO (inclusive) .. AS_RUN (exclusive)
// Name normalization: none (assembler label AS_PROGIO kept verbatim).
void AS_PROGIO() {

  variables().MON_A1 = variables_const().AS_TXTTAB;
  variables().MON_A2 = variables_const().AS_VARTAB;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_WRITE (inclusive) .. MON_BELL (exclusive)
// Name normalization: none (assembler label MON_WRITE kept verbatim).
void MON_WRITE() {
  // Monitor tape write handler: emit bytes in [A1, A2) and then emit checksum.
  // The ROM loop updates A1 until it reaches A2 via NXTA1 carry behavior.
  // We model that range with one unified 16-bit pointer representation.

  MON_HEADR(0x40u);

  std::uint16_t a1Ptr = variables_const().MON_A1;
  const std::uint16_t a2Limit = variables_const().MON_A2;
  std::uint8_t runningChecksum = 0xffu;

  while (a1Ptr != a2Limit) {
    const std::uint8_t dataByte = ReadProgramByte(a1Ptr);
    runningChecksum = static_cast<std::uint8_t>(runningChecksum ^ dataByte);

    // Placeholder cassette serialization through I/O companion storage.
    for (std::uint8_t bit = 0u; bit < 8u; ++bit) {
      const std::uint8_t state =
          static_cast<std::uint8_t>((dataByte >> (7u - bit)) & 0x01u);
      variables().writeByte(IOPorts::ADDR_MON_TAPE_OUTPUT, state);
    }
    a1Ptr = static_cast<std::uint16_t>(a1Ptr + 1u);
  }

  variables().MON_A1 = a1Ptr;

  // Emit checksum byte before returning (ROM path branches to BELL next).
  for (std::uint8_t bit = 0u; bit < 8u; ++bit) {
    const std::uint8_t state =
        static_cast<std::uint8_t>((runningChecksum >> (7u - bit)) & 0x01u);
    variables().writeByte(IOPorts::ADDR_MON_TAPE_OUTPUT, state);
  }
  MON_BELL();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_READ (inclusive) .. MON_RD2 (exclusive)
// Name normalization: none (assembler label MON_READ kept verbatim).
void MON_READ() {
  // FIND TAPEIN EDGE, DELAY 3.5 SECONDS, INIT CHKSUM=$FF, FIND EDGE AGAIN.
  MON_RD2BIT();
  MON_HEADR(0x16u);
  variables().MON_CHKSUM = 0xffu;
  MON_RD2BIT();

  // READ does not terminate; it falls through directly into RD2.
  MON_RD2();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CALL (inclusive) .. AS_IN_NUMBER (exclusive)
// Name normalization: none (assembler label AS_CALL kept verbatim).
void AS_CALL() {
  //
  // Execute machine language subroutine at expression address.
  // Evaluates the numeric expression to a 16-bit address, then performs
  // an indirect jump to that address. The called routine returns with RTS.

  AS_FRMNUM();
  AS_GETADR();

  const std::uint16_t callAddress = variables_const().AS_LINNUM;
  auto callRoutine = reinterpret_cast<void (*)()>(callAddress);
  callRoutine();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_IN_NUMBER (inclusive) .. AS_PR_NUMBER (exclusive)
// Name normalization: none (assembler label AS_IN_NUMBER kept verbatim).
void AS_IN_NUMBER() {

  const std::uint8_t slot = AS_GETBYT();
  MON_INPORT(slot);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PR_NUMBER (inclusive) .. AS_PLOTFNS (exclusive)
// Name normalization: none (assembler label AS_PR_NUMBER kept verbatim).
void AS_PR_NUMBER() {

  const std::uint8_t slot = AS_GETBYT();
  MON_OUTPORT(slot);
}

// Monitor tape I/O and debug helpers (stubs for incremental porting).

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_RD2 (inclusive) .. MON_RD3 (exclusive)
// Name normalization: none (assembler label MON_RD2 kept verbatim).
void MON_RD2() {
  // LOOK FOR SYNC BIT (SHORT 0): loop while carry remains set.
  std::uint8_t attempts = 0x24u;
  while (attempts != 0u) {
    const bool carry_set = MON_RDBIT();
    if (!carry_set) {
      break;
    }
    --attempts;
  }

  // SKIP SECOND SYNC H-CYCLE, then prime the next-bit index for RD3.
  (void)MON_RDBIT();
  constexpr std::uint8_t kIndexForZeroOneTest = 0x3bu;
  (void)kIndexForZeroOneTest;

  // RD2 does not terminate; it falls through directly into RD3.
  MON_RD3();
}

void MON_RD2BIT() {
  // Find a tape input edge transition with a bounded poll loop.
  const std::uint8_t initial = static_cast<std::uint8_t>(
      variables_const().readByte(IOPorts::ADDR_MON_TAPE_INPUT) & 0x80u);
  for (std::uint16_t spins = 0u; spins < 4096u; ++spins) {
    const std::uint8_t current = static_cast<std::uint8_t>(
        variables_const().readByte(IOPorts::ADDR_MON_TAPE_INPUT) & 0x80u);
    if (current != initial) {
      break;
    }
  }
}

void MON_HEADR(std::uint8_t delay_code) {
  // Delay loop used by monitor tape paths; bounded and deterministic.
  const std::uint16_t outer = static_cast<std::uint16_t>(delay_code) * 64u;
  volatile std::uint16_t sink = 0u;
  for (std::uint16_t i = 0u; i < outer; ++i) {
    sink = static_cast<std::uint16_t>(sink + (i ^ delay_code));
  }
  (void)sink;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_RD3 (inclusive) .. MON_PRERR (exclusive)
// Name normalization: none (assembler label MON_RD3 kept verbatim).
void MON_RD3() {
  constexpr std::uint8_t kReadLoopIndex = 0x3bu;
  constexpr std::uint8_t kCompensatedIndex = 0x35u;

  std::uint8_t bitTimingIndex = kReadLoopIndex;
  bool carry_set = false;
  do {
    const std::uint8_t value = MON_RDBYTE();
    const std::uint16_t a1Ptr = variables_const().MON_A1;
    WriteProgramByte(a1Ptr, value);

    const std::uint8_t runningChecksum =
        static_cast<std::uint8_t>(value ^ variables_const().MON_CHKSUM);
    variables().MON_CHKSUM = runningChecksum;

    carry_set = MON_NXTA1();
    bitTimingIndex = kCompensatedIndex;
  } while (!carry_set);

  const std::uint8_t checksumByte = MON_RDBYTE();
  const std::uint8_t runningChecksum = variables_const().MON_CHKSUM;
  if (checksumByte == runningChecksum) {
    MON_BELL();
    return;
  }

  // No terminating jump: ROM falls through directly into PRERR.
  (void)bitTimingIndex;
  MON_PRERR();
}

bool MON_RDBIT() {
  // Return carry-equivalent from monitor tape input bit 7.
  return (variables_const().readByte(IOPorts::ADDR_MON_TAPE_INPUT) & 0x80u) !=
         0u;
}

std::uint8_t MON_RDBYTE() {
  // Read 8 bits MSB-first from the monitor tape input helper.
  std::uint8_t value = 0u;
  for (std::uint8_t i = 0u; i < 8u; ++i) {
    value = static_cast<std::uint8_t>(value << 1u);
    if (MON_RDBIT()) {
      value = static_cast<std::uint8_t>(value | 0x01u);
    }
  }
  return value;
}

bool MON_NXTA1() {
  // Increment A1 and return carry-equivalent (A1 >= A2 after increment).
  std::uint16_t a1 = variables_const().MON_A1;
  const std::uint16_t a2 = variables_const().MON_A2;

  a1 = static_cast<std::uint16_t>(a1 + 1u);
  variables().MON_A1 = a1;
  return a1 >= a2;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_RESTORE (inclusive) .. MON_SAVE (exclusive)
// Name normalization: none (assembler label MON_RESTORE kept verbatim).
void MON_RESTORE() {
  // Restore 6502 register state from zero-page storage (used by debug
  // software). Restores accumulator, X, Y registers and processor status flags
  // from fixed locations. Original sequence: Load A from $48 (status), push;
  // Load A from $45; Load X from $46; Load Y from $47; Restore processor flags
  // via PLP; RTS.

  // Read saved processor status (this will be re-pushed via PLP emulation).
  const std::uint8_t saved_status = variables_const().MON_STATUS;

  // Read saved register values from zero-page storage.
  const std::uint8_t saved_reg_a = variables_const().MON_DEBUG_REG_A;
  const std::uint8_t saved_reg_x = variables_const().MON_DEBUG_REG_X;
  const std::uint8_t saved_reg_y = variables_const().MON_DEBUG_REG_Y;

  // This runtime does not execute a 6502 CPU core, so there is no processor
  // register file to load. Preserve monitor-visible saved-register state so
  // subsequent monitor/debug routines observe the restored values.
  variables().MON_STATUS = saved_status;
  variables().MON_DEBUG_REG_A = saved_reg_a;
  variables().MON_DEBUG_REG_X = saved_reg_x;
  variables().MON_DEBUG_REG_Y = saved_reg_y;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: MON_PRERR (inclusive) .. MON_BELL (exclusive)
// Name normalization: none (assembler label MON_PRERR kept verbatim).
void MON_PRERR() {
  // PRINT "ERR", THEN BELL.
  MON_COUT(0xc5u); // 'E' with high bit set
  MON_COUT(0xd2u); // 'R' with high bit set
  MON_COUT(0xd2u); // 'R' with high bit set

  // PRERR does not terminate; it falls through directly into BELL.
  MON_BELL();
}

} // namespace applesoft::asm_port