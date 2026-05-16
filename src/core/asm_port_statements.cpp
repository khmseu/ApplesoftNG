#include "core/applesoft_variables.hpp"
#include "core/asm_port_clear.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_token_name_table.hpp"
#include "core/io_ports.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace applesoft::asm_port {

constexpr std::size_t kTokenCount = 107;
constexpr std::uint8_t kTokenBase = 0x80u;

std::uint16_t ReadZeroPageWord(std::uint8_t address);
std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void SetTextPointer(std::uint16_t address);
void AS_RESTART();
void MON_WRITE();
void MON_READ();
void MON_INPORT(std::uint8_t slot);
void MON_OUTPORT(std::uint8_t slot);
void MON_RD2();
void MON_RD3();
void MON_RD2BIT();
void MON_HEADR(std::uint8_t delay_code);
bool MON_RDBIT();
std::uint8_t MON_RDBYTE();
bool MON_NXTA1();
void MON_BELL();
void MON_PRERR();
void MON_RESTORE();
bool AS_SETPTRS();
void AS_FIX_LINKS();
void AS_VARTIO();
void AS_PROGIO();
std::uint16_t AS_PTRGET();
void AS_SYNCHR(std::uint8_t expected);
void AS_FRMEVL();
void AS_FRMNUM();
bool AS_CHKVAL(std::uint8_t savedValTyp);
void AS_ROUND_FAC();
void AS_AYINT();
void AS_SETFOR();
void AS_STRINI(std::uint8_t length);
void AS_MOVINS();
bool AS_FRETMS(std::uint16_t descriptorAddress);
void AS_LET2(std::uint8_t savedValTypPlus1);
void AS_PUTSTR();
void AS_LINGET();
bool AS_FNDLIN();
std::uint8_t AS_CHRGOT();
std::uint8_t AS_CHRGET();
bool IsStatementEndOfParsedInput();
bool AS_ISCNTC();
void AS_CRDO();
void AS_LINPRT();
void MON_COUT(std::uint8_t value);
void AS_OUTDO(std::uint8_t value);
bool AS_FL1(std::uint16_t startAddress);
void AS_RESTORE();
void AS_STKINI();
std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);
bool AS_NEW_impl();
void AS_SCRTCH_impl();
bool AS_SETPTRS_impl();
bool AS_CLEAR_impl();
void AS_CLEARC_impl();
void AS_STXTPT_impl();
void AS_GO_TO_LINE();
void AS_GETADR();
void AS_SNGFLT(std::uint8_t value);
std::uint8_t AS_GTNUM();
std::uint8_t AS_COMBYTE();
void AS_RTS_10();

bool AS_NEW_impl() {
  if (!IsStatementEndOfParsedInput()) {
    return false;
  }

  AS_SCRTCH_impl();
  return true;
}

void AS_SCRTCH_impl() {
  constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  constexpr std::uint8_t kAS_LOCK = ApplesoftVariables::ZP_AS_LOCK;
  constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
  constexpr std::uint8_t kAS_PRGEND = ApplesoftVariables::ZP_AS_PRGEND;
  constexpr std::uint8_t kAS_ARYTAB = ApplesoftVariables::ZP_AS_ARYTAB;
  constexpr std::uint8_t kAS_STREND = ApplesoftVariables::ZP_AS_STREND;
  constexpr std::uint8_t kAS_MEMSIZ = ApplesoftVariables::ZP_AS_MEMSIZ;
  constexpr std::uint8_t kAS_FRETOP = ApplesoftVariables::ZP_AS_FRETOP;

  const std::uint16_t txtTabAddr = ReadZeroPageWord(kAS_TXTTAB);
  WriteZeroPageByte(kAS_LOCK, 0);
  WriteProgramByte(txtTabAddr, 0);
  WriteProgramByte(static_cast<std::uint16_t>(txtTabAddr + 1u), 0);

  const std::uint16_t nextFree = static_cast<std::uint16_t>(txtTabAddr + 2u);
  WriteZeroPageWord(kAS_VARTAB, nextFree);
  WriteZeroPageWord(kAS_PRGEND, nextFree);
  WriteZeroPageWord(kAS_FRETOP, ReadZeroPageWord(kAS_MEMSIZ));
  WriteZeroPageWord(kAS_ARYTAB, ReadZeroPageWord(kAS_VARTAB));
  WriteZeroPageWord(kAS_STREND, ReadZeroPageWord(kAS_VARTAB));

  AS_SETPTRS_impl();
}

bool AS_SETPTRS_impl() {
  AS_STXTPT();
  AS_CLEAR();
  return true;
}

bool AS_NEW() { return AS_NEW_impl(); }

bool AS_SETPTRS() { return AS_SETPTRS_impl(); }

void AS_SCRTCH() { AS_SCRTCH_impl(); }

void AS_RUN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_RUN (inclusive) .. AS_GOSUB (exclusive)
  // Name normalization: none (assembler label AS_RUN kept verbatim).
  constexpr std::uint8_t kAS_CURLIN_hi =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_CURLIN + 1u);

  std::uint8_t curlinHi = ReadZeroPageByte(kAS_CURLIN_hi);
  WriteZeroPageByte(kAS_CURLIN_hi, static_cast<std::uint8_t>(curlinHi - 1));

  const std::uint8_t currentChar = AS_CHRGOT();
  if (currentChar == 0) {
    AS_SETPTRS();
    return;
  }

  AS_CLEARC();
  AS_GO_TO_LINE();
}

void AS_PEEK() {
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;

  const std::uint16_t savedAS_Linnum = ReadZeroPageWord(kAS_LINNUM);
  AS_GETADR();
  const std::uint8_t value = ReadProgramByte(ReadZeroPageWord(kAS_LINNUM));
  WriteZeroPageWord(kAS_LINNUM, savedAS_Linnum);
  AS_SNGFLT(value);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_POKE (inclusive) .. AS_WAIT (exclusive)
// Name normalization: none (assembler label AS_POKE kept verbatim).
void AS_POKE() {
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;

  const std::uint8_t value = AS_GTNUM();
  WriteProgramByte(ReadZeroPageWord(kAS_LINNUM), value);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_WAIT (inclusive) .. AS_FADDH (exclusive)
// Name normalization: none (assembler label AS_WAIT kept verbatim).
void AS_WAIT() {
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;

  const std::uint8_t mask = AS_GTNUM();
  WriteZeroPageByte(kAS_FORPNT, mask);

  std::uint8_t xorMask = 0u;
  if (AS_CHRGOT() != 0u) {
    xorMask = AS_COMBYTE();
  }
  WriteZeroPageByte(static_cast<std::uint8_t>(kAS_FORPNT + 1u), xorMask);

  while (true) {
    const std::uint8_t value = ReadProgramByte(ReadZeroPageWord(kAS_LINNUM));
    const std::uint8_t masked =
        static_cast<std::uint8_t>((value ^ xorMask) & mask);
    if (masked != 0u) {
      AS_RTS_10();
      return;
    }
  }
}

void AS_RTS_10() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_RTS_10 (inclusive) .. AS_FADDH (exclusive)
  // Name normalization: RTS_10 -> AS_RTS_10 virtual Applesoft prefix only.
  // ROM label RTS_10 is a shared return target for AS_WAIT.
  return;
}

struct TokenMatch {
  std::uint8_t code;
  std::uint8_t length;
  std::string_view name;
};

std::uint8_t read_AS_INPUT_BUFFER(std::uint8_t index) {
  return variables_const().pointer(0x0200u).read(index);
}

void write_AS_INPUT_BUFFER_minus_5(std::uint8_t index, std::uint8_t value) {
  variables().pointer(0x01fbu).write(value, index);
}

void SetTextPointerToInputBufferMinus1() {
  // TODO(asm-port): compute the actual AS_INPUT_BUFFER-1 address in the runtime
  // model.
  SetTextPointer(0x01ffu);
}

std::optional<TokenMatch> MatchToken(std::uint8_t index) {
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

std::uint8_t ScanAheadOffsetForData(std::uint8_t terminator) {
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
      const std::uint16_t textPtr = ReadZeroPageWord(kAS_TXTPTR);
      const std::uint8_t ch = variables_const().pointer(textPtr).read(offset);
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

void AS_ADDON(std::uint8_t offset) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_ADDON (inclusive) .. AS_DATAN (exclusive)
  // Name normalization: none (assembler label AS_ADDON kept verbatim).

  constexpr std::uint8_t kAS_TXTPTR = ApplesoftVariables::ZP_AS_TXTPTR;

  const std::uint16_t textPtr = ReadZeroPageWord(kAS_TXTPTR);
  WriteZeroPageWord(kAS_TXTPTR, static_cast<std::uint16_t>(textPtr + offset));
}

std::uint8_t AS_DATAN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_DATAN (inclusive) .. AS_REMN (exclusive)
  // Name normalization: none (assembler label AS_DATAN kept verbatim).

  return ScanAheadOffsetForData(static_cast<std::uint8_t>(':'));
}

void AS_DATA() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_DATA (inclusive) .. AS_ADDON (exclusive)
  // Name normalization: none (assembler label AS_DATA kept verbatim).

  const std::uint8_t offset = AS_DATAN();
  AS_ADDON(offset);
}

void AS_LET() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_LET (inclusive) .. AS_LET2 (exclusive)
  // Name normalization: none (assembler label AS_LET kept verbatim).

  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;
  constexpr std::uint8_t kTOKEN_EQUAL = 0xd0;
  constexpr std::uint8_t kAS_VALTYP = ApplesoftVariables::ZP_AS_VALTYP;

  const std::uint16_t variablePtr = AS_PTRGET();
  WriteZeroPageWord(kAS_FORPNT, variablePtr);

  AS_SYNCHR(kTOKEN_EQUAL);

  const std::uint8_t savedValTyp = ReadZeroPageByte(kAS_VALTYP);
  const std::uint8_t savedValTypPlus1 =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_VALTYP + 1u));

  AS_FRMEVL();

  if (AS_CHKVAL(savedValTyp)) {
    // AS_LET_STRING branch falls through to AS_PUTSTR in ROM.
    AS_PUTSTR();
    return;
  }

  // Explicitly model AS_LET -> AS_LET2 fall-through.
  AS_LET2(savedValTypPlus1);
}

void AS_LET2(std::uint8_t savedValTypPlus1) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_LET2 (inclusive) .. AS_PUTSTR (exclusive)
  // Name normalization: none (assembler label AS_LET2 kept verbatim).

  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;
  constexpr std::uint8_t kAS_FAC_PLUS_3 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 3u);
  constexpr std::uint8_t kAS_FAC_PLUS_4 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 4u);

  // Positive means real variable; ROM jumps directly to AS_SETFOR.
  if ((savedValTypPlus1 & 0x80u) == 0u) {
    AS_SETFOR();
    return;
  }

  AS_ROUND_FAC();
  AS_AYINT();

  const std::uint16_t forPtr = ReadZeroPageWord(kAS_FORPNT);
  auto forPtrByte = variables().pointer(forPtr);
  forPtrByte.write(ReadZeroPageByte(kAS_FAC_PLUS_3));
  forPtrByte.write(ReadZeroPageByte(kAS_FAC_PLUS_4), 1u);
}

void AS_PUTSTR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PUTSTR (inclusive) .. AS_PR_STRING (exclusive)
  // Name normalization: none (assembler label AS_PUTSTR kept verbatim).

  constexpr std::uint8_t kAS_FAC_PLUS_3 =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 3u);
  constexpr std::uint8_t kAS_DSCPTR = ApplesoftVariables::ZP_AS_DSCPTR;
  constexpr std::uint8_t kAS_FORPNT = ApplesoftVariables::ZP_AS_FORPNT;
  constexpr std::uint8_t kAS_FRETOP = ApplesoftVariables::ZP_AS_FRETOP;
  constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
  constexpr std::uint8_t kAS_STRNG1 = ApplesoftVariables::ZP_AS_STRNG1;

  const std::uint16_t facDescriptor = ReadZeroPageWord(kAS_FAC_PLUS_3);
  const auto facDescriptorPtr = variables_const().pointer(facDescriptor);
  auto readDescriptorByte = [&](std::uint8_t offset) {
    return facDescriptorPtr.read(offset);
  };

  std::uint16_t descriptorPointer = ReadZeroPageWord(kAS_FAC_PLUS_3);

  const std::uint8_t descDataHigh = readDescriptorByte(2);
  const std::uint8_t fretopHigh =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_FRETOP + 1u));

  bool useExistingDescriptor = false;
  bool descriptorIsVariable = false;

  if (descDataHigh < fretopHigh) {
    useExistingDescriptor = true;
  } else if (descDataHigh == fretopHigh) {
    const std::uint8_t descDataAS_Low = readDescriptorByte(1);
    if (descDataAS_Low < ReadZeroPageByte(kAS_FRETOP)) {
      useExistingDescriptor = true;
    }
  }

  if (!useExistingDescriptor) {
    if (descriptorPointer >= ReadZeroPageWord(kAS_VARTAB)) {
      descriptorIsVariable = true;
    }
  }

  if (descriptorIsVariable) {
    AS_STRINI(readDescriptorByte(0));
    WriteZeroPageWord(kAS_STRNG1, ReadZeroPageWord(kAS_DSCPTR));
    AS_MOVINS();
    descriptorPointer = kAS_FAC_PLUS_3;
  }

  WriteZeroPageWord(kAS_DSCPTR, descriptorPointer);

  (void)AS_FRETMS(descriptorPointer);

  const std::uint16_t source = ReadZeroPageWord(kAS_DSCPTR);
  const std::uint16_t dest = ReadZeroPageWord(kAS_FORPNT);
  const auto sourcePtr = variables_const().pointer(source);
  auto destPtr = variables().pointer(dest);
  for (std::uint8_t i = 0; i < 3; ++i) {
    destPtr.write(sourcePtr.read(i), i);
  }
}

void DeleteExistingAS_Line() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_NUMBERED_LINE delete block (inclusive) .. AS_PUT_NEW_LINE
  // (exclusive) Name normalization: C++ helper; corresponds to the inline
  // delete block inside AS_NUMBERED_LINE (T:0471–T:04b5) in the assembler
  // listing.

  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;
  constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;

  const std::uint16_t lowtr = ReadZeroPageWord(kAS_LOWTR);
  const std::uint16_t nextAS_Line = ApplesoftVariables::makeWord(
      variables_const().readByte(lowtr),
      variables_const().readByte(static_cast<std::uint16_t>(lowtr + 1u)));

  const std::uint16_t lineSize =
      static_cast<std::uint16_t>(nextAS_Line - lowtr);
  const std::uint16_t vartab = ReadZeroPageWord(kAS_VARTAB);

  const std::uint16_t moveCount =
      static_cast<std::uint16_t>(vartab - nextAS_Line);
  for (std::uint16_t i = 0; i < moveCount; ++i) {
    variables().writeByte(static_cast<std::uint16_t>(lowtr + i),
                          variables_const().readByte(
                              static_cast<std::uint16_t>(nextAS_Line + i)));
  }

  WriteZeroPageWord(kAS_VARTAB, static_cast<std::uint16_t>(vartab - lineSize));
}

void InsertNewAS_Line() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PUT_NEW_LINE (inclusive) .. AS_FIX_LINKS (exclusive)
  // Name normalization: C++ helper; corresponds to AS_PUT_NEW_LINE
  // (T:04b5–T:04f2).

  constexpr std::uint16_t kTokenBuf =
      static_cast<std::uint16_t>(ApplesoftVariables::ADDR_AS_INPUT_BUFFER - 5u);
  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;
  constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
  constexpr std::uint8_t kAS_STREND = ApplesoftVariables::ZP_AS_STREND;
  constexpr std::uint8_t kAS_FRETOP = ApplesoftVariables::ZP_AS_FRETOP;
  constexpr std::uint8_t kAS_MEMSIZ = ApplesoftVariables::ZP_AS_MEMSIZ;
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;

  if (variables_const().readByte(ApplesoftVariables::ADDR_AS_INPUT_BUFFER) ==
      0u) {
    return;
  }

  WriteZeroPageWord(kAS_FRETOP, ReadZeroPageWord(kAS_MEMSIZ));

  std::uint16_t tokenAS_Len = 0;
  while (variables_const().readByte(
             static_cast<std::uint16_t>(kTokenBuf + tokenAS_Len)) != 0u) {
    ++tokenAS_Len;
  }

  const std::uint16_t lineSize = static_cast<std::uint16_t>(tokenAS_Len + 5u);
  const std::uint16_t lowtr = ReadZeroPageWord(kAS_LOWTR);
  const std::uint16_t vartab = ReadZeroPageWord(kAS_VARTAB);

  for (std::uint16_t i = vartab; i > lowtr; --i) {
    const std::uint16_t src = static_cast<std::uint16_t>(i - 1u);
    variables().writeByte(static_cast<std::uint16_t>(src + lineSize),
                          variables_const().readByte(src));
  }

  const std::uint16_t linnum = ReadZeroPageWord(kAS_LINNUM);
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
  WriteZeroPageWord(kAS_VARTAB, newVartab);
  WriteZeroPageWord(kAS_STREND, newVartab);
}

void AS_PARSE_INPUT_LINE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PARSE_INPUT_LINE (inclusive) .. AS_FNDLIN (exclusive)
  // Name normalization: none (assembler label AS_PARSE_INPUT_LINE kept
  // verbatim).

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

void HandleNumberedAS_Line() {
  AS_LINGET();
  AS_PARSE_INPUT_LINE();

  if (AS_FNDLIN()) {
    DeleteExistingAS_Line();
  }

  InsertNewAS_Line();
  AS_FIX_LINKS();
}

std::uint16_t GetTextTablePointer() {
  constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  return ReadZeroPageWord(kAS_TXTTAB);
}

bool IsEndOfProgram(std::uint16_t current) { return current == 0u; }

std::uint16_t AdvanceToNextAS_Line(std::uint16_t current) {
  // The original AS_FIX_LINKS routine scans from the current line until it
  // finds the end-of-line marker, then computes the address of the next line.
  std::uint16_t offset = 4u;
  while (variables_const().readByte(
             static_cast<std::uint16_t>(current + offset)) != 0u) {
    ++offset;
  }

  return static_cast<std::uint16_t>(current + offset + 1u);
}

void WriteForwardPointer(std::uint16_t current, std::uint16_t next) {
  variables().writeByte(current, ApplesoftVariables::lowByte(next));
  variables().writeByte(static_cast<std::uint16_t>(current + 1u),
                        ApplesoftVariables::highByte(next));
}

void AS_FIX_LINKS() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FIX_LINKS (inclusive) .. AS_INLIN (exclusive)
  // Name normalization: none (assembler label AS_FIX_LINKS kept verbatim).

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

bool AS_FNDLIN() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FNDLIN (inclusive) .. AS_FL1 (exclusive)
  // Name normalization: none (assembler label AS_FNDLIN kept verbatim).

  constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;

  // Assembler falls through from AS_FNDLIN directly into AS_FL1 with
  // A=AS_TXTTAB, X=AS_TXTTAB+1.
  return AS_FL1(ReadZeroPageWord(kAS_TXTTAB));
}

bool AS_FL1(std::uint16_t startAddress) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_FL1 (inclusive) .. AS_NEW (exclusive)
  // Name normalization: none (assembler label AS_FL1 kept verbatim).

  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;

  const std::uint8_t targetAS_Lo = ReadZeroPageByte(kAS_LINNUM);
  const std::uint8_t targetHi =
      ReadZeroPageByte(static_cast<std::uint8_t>(kAS_LINNUM + 1u));

  std::uint16_t current = startAddress;

  while (true) {
    WriteZeroPageWord(kAS_LOWTR, current);

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

void AS_DEL() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_DEL (inclusive) .. AS_GR (exclusive)
  // Name normalization: none (assembler label AS_DEL kept verbatim).

  constexpr std::uint8_t kAS_PRGEND = ApplesoftVariables::ZP_AS_PRGEND;
  constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
  constexpr std::uint8_t kAS_DEST = ApplesoftVariables::ZP_AS_DEST;
  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;

  const std::uint16_t prgend = ReadZeroPageWord(kAS_PRGEND);
  WriteZeroPageWord(kAS_VARTAB, prgend);

  AS_LINGET();
  AS_FNDLIN();
  WriteZeroPageWord(kAS_DEST, ReadZeroPageWord(kAS_LOWTR));

  AS_SYNCHR(static_cast<std::uint8_t>(','));
  AS_LINGET();

  std::uint8_t linnumAS_Lo = ReadZeroPageByte(kAS_LINNUM);
  if (linnumAS_Lo == 0xffu) {
    WriteZeroPageByte(kAS_LINNUM, 0u);
    const std::uint8_t linnumHi =
        ReadZeroPageByte(static_cast<std::uint8_t>(kAS_LINNUM + 1u));
    WriteZeroPageByte(static_cast<std::uint8_t>(kAS_LINNUM + 1u),
                      static_cast<std::uint8_t>(linnumHi + 1u));
  } else {
    WriteZeroPageByte(kAS_LINNUM, static_cast<std::uint8_t>(linnumAS_Lo + 1u));
  }

  AS_FNDLIN();

  const std::uint16_t lowtr = ReadZeroPageWord(kAS_LOWTR);
  const std::uint16_t dest = ReadZeroPageWord(kAS_DEST);
  if (lowtr < dest) {
    return;
  }

  const std::uint16_t vartab = ReadZeroPageWord(kAS_VARTAB);
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
  WriteZeroPageWord(kAS_VARTAB, newVartab);

  AS_FIX_LINKS();
}

void AS_LIST() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_LIST (inclusive) .. AUTO (exclusive)
  // Name normalization: none (assembler label AS_LIST kept verbatim).

  constexpr std::uint8_t kAS_LOWTR = ApplesoftVariables::ZP_AS_LOWTR;
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
  constexpr std::uint8_t kMON_CH = ApplesoftVariables::ZP_MON_CH;
  constexpr std::uint8_t kAS_CURLIN = ApplesoftVariables::ZP_AS_CURLIN;

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

  std::uint16_t endRange = ReadZeroPageWord(kAS_LINNUM);
  if (endRange == 0u) {
    endRange = 0xffffu;
  }

  std::uint16_t current = ReadZeroPageWord(kAS_LOWTR);
  while (current != 0u) {
    if (AS_ISCNTC()) {
      break;
    }

    AS_CRDO();

    const std::uint16_t currentAS_Line = ApplesoftVariables::makeWord(
        variables_const().readByte(static_cast<std::uint16_t>(current + 2u)),
        variables_const().readByte(static_cast<std::uint16_t>(current + 3u)));
    if (currentAS_Line > endRange) {
      break;
    }

    WriteZeroPageWord(kAS_CURLIN, currentAS_Line);
    AS_LINPRT();
    WriteZeroPageByte(kMON_CH, 5u);

    std::uint16_t offset = 4u;
    while (true) {
      const std::uint8_t ch = variables_const().readByte(
          static_cast<std::uint16_t>(current + offset));
      if (ch == 0u) {
        break;
      }
      AS_OUTDO(static_cast<std::uint8_t>(ch & 0x7fu));
      ++offset;
    }

    current = static_cast<std::uint16_t>(current + offset + 1u);
    WriteZeroPageWord(kAS_LOWTR, current);
  }

  AS_CRDO();
}

void AS_SAVE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SAVE (inclusive) .. AS_LOAD (exclusive)
  // Name normalization: none (assembler label AS_SAVE kept verbatim).

  constexpr std::uint8_t kAS_PRGEND = ApplesoftVariables::ZP_AS_PRGEND;
  constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;

  const std::uint16_t programEnd = ReadZeroPageWord(kAS_PRGEND);
  const std::uint16_t textTable = ReadZeroPageWord(kAS_TXTTAB);
  const std::uint16_t programAS_Length =
      static_cast<std::uint16_t>(programEnd - textTable);
  WriteZeroPageWord(kAS_LINNUM, programAS_Length);

  AS_VARTIO();
  MON_WRITE();
  AS_PROGIO();
  MON_WRITE();
}

void AS_LOAD() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_LOAD (inclusive) .. AS_VARTIO (exclusive)
  // Name normalization: none (assembler label AS_LOAD kept verbatim).

  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
  constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
  constexpr std::uint8_t kAS_TEMPPT = ApplesoftVariables::ZP_AS_TEMPPT;
  constexpr std::uint8_t kAS_LOCK = ApplesoftVariables::ZP_AS_LOCK;

  AS_VARTIO();
  MON_READ();

  const std::uint16_t textTable = ReadZeroPageWord(kAS_TXTTAB);
  const std::uint16_t programAS_Length = ReadZeroPageWord(kAS_LINNUM);
  WriteZeroPageWord(kAS_VARTAB,
                    static_cast<std::uint16_t>(textTable + programAS_Length));

  WriteZeroPageByte(kAS_LOCK, ReadZeroPageByte(kAS_TEMPPT));

  AS_PROGIO();
  MON_READ();

  if ((ReadZeroPageByte(kAS_LOCK) & 0x80u) != 0u) {
    (void)AS_SETPTRS();
    return;
  }

  AS_FIX_LINKS();
}

void AS_VARTIO() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_VARTIO (inclusive) .. AS_PROGIO (exclusive)
  // Name normalization: none (assembler label AS_VARTIO kept verbatim).

  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;
  constexpr std::uint8_t kAS_TEMPPT = ApplesoftVariables::ZP_AS_TEMPPT;
  constexpr std::uint8_t kAS_LOCK = ApplesoftVariables::ZP_AS_LOCK;
  constexpr std::uint8_t kMON_A1L = ApplesoftVariables::ZP_MON_A1;
  constexpr std::uint8_t kMON_A1H =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A1 + 1u);
  constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
  constexpr std::uint8_t kMON_A2H =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A2 + 1u);

  WriteZeroPageByte(kMON_A1L, kAS_LINNUM);
  WriteZeroPageByte(kMON_A1H, 0x00);
  WriteZeroPageByte(kMON_A2L, kAS_TEMPPT);
  WriteZeroPageByte(kMON_A2H, 0x00);
  WriteZeroPageByte(kAS_LOCK, 0x00);
}

void AS_PROGIO() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PROGIO (inclusive) .. AS_RUN (exclusive)
  // Name normalization: none (assembler label AS_PROGIO kept verbatim).

  constexpr std::uint8_t kAS_TXTTAB = ApplesoftVariables::ZP_AS_TXTTAB;
  constexpr std::uint8_t kAS_VARTAB = ApplesoftVariables::ZP_AS_VARTAB;
  constexpr std::uint8_t kMON_A1L = ApplesoftVariables::ZP_MON_A1;
  constexpr std::uint8_t kMON_A1H =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A1 + 1u);
  constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;
  constexpr std::uint8_t kMON_A2H =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_MON_A2 + 1u);

  WriteZeroPageWord(kMON_A1L, ReadZeroPageWord(kAS_TXTTAB));
  WriteZeroPageWord(kMON_A2L, ReadZeroPageWord(kAS_VARTAB));

  // Keep symbolic names visible for monitor register parity.
  (void)kMON_A1H;
  (void)kMON_A2H;
}

void MON_WRITE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // MON_Labels: WRITE (inclusive) .. READ (exclusive)
  // Name normalization: none (assembler label WRITE is prefixed with MON_ in
  // C++).
  //
  // Monitor tape write handler: emit bytes in [A1, A2) and then emit checksum.
  // The ROM loop updates A1 until it reaches A2 via NXTA1 carry behavior.
  // We model that range with one unified 16-bit pointer representation.

  constexpr std::uint8_t kMON_A1L = ApplesoftVariables::ZP_MON_A1;
  constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;

  MON_HEADR(0x40u);

  std::uint16_t a1Ptr = ReadZeroPageWord(kMON_A1L);
  const std::uint16_t a2Limit = ReadZeroPageWord(kMON_A2L);
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

  WriteZeroPageWord(kMON_A1L, a1Ptr);

  // Emit checksum byte before returning (ROM path branches to BELL next).
  for (std::uint8_t bit = 0u; bit < 8u; ++bit) {
    const std::uint8_t state =
        static_cast<std::uint8_t>((runningChecksum >> (7u - bit)) & 0x01u);
    variables().writeByte(IOPorts::ADDR_MON_TAPE_OUTPUT, state);
  }
  MON_BELL();
}

void MON_READ() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // MON_Labels: READ (inclusive) .. RD2 (exclusive)
  // Name normalization: none (assembler label READ is prefixed with MON_ in
  // C++).

  // FIND TAPEIN EDGE, DELAY 3.5 SECONDS, INIT CHKSUM=$FF, FIND EDGE AGAIN.
  MON_RD2BIT();
  MON_HEADR(0x16u);
  WriteZeroPageByte(ApplesoftVariables::ZP_MON_CHKSUM, 0xffu);
  MON_RD2BIT();

  // READ does not terminate; it falls through directly into RD2.
  MON_RD2();
}

void AS_CALL() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_CALL (inclusive) .. AS_IN_NUMBER (exclusive)
  // Name normalization: none (assembler label AS_CALL kept verbatim).
  //
  // Execute machine language subroutine at expression address.
  // Evaluates the numeric expression to a 16-bit address, then performs
  // an indirect jump to that address. The called routine returns with RTS.

  constexpr std::uint8_t kAS_LINNUM = ApplesoftVariables::ZP_AS_LINNUM;

  AS_FRMNUM();
  AS_GETADR();

  const std::uint16_t callAddress = ReadZeroPageWord(kAS_LINNUM);
  auto callRoutine = reinterpret_cast<void (*)()>(callAddress);
  callRoutine();
}

void AS_IN_NUMBER() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_IN_NUMBER (inclusive) .. AS_PR_NUMBER (exclusive)
  // Name normalization: none (assembler label AS_IN_NUMBER kept verbatim).

  const std::uint8_t slot = AS_GETBYT();
  MON_INPORT(slot);
}

void AS_PR_NUMBER() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_PR_NUMBER (inclusive) .. AS_PLOTFNS (exclusive)
  // Name normalization: none (assembler label AS_PR_NUMBER kept verbatim).

  const std::uint8_t slot = AS_GETBYT();
  MON_OUTPORT(slot);
}

// Monitor tape I/O and debug helpers (stubs for incremental porting).

void MON_RD2() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // MON_Labels: RD2 (inclusive) .. RD3 (exclusive)
  // Name normalization: none (assembler label RD2 is prefixed with MON_ in
  // C++).

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

void MON_RD3() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // MON_Labels: RD3 (inclusive) .. PRERR (exclusive)
  // Name normalization: none (assembler label RD3 is prefixed with MON_ in
  // C++).

  constexpr std::uint8_t kMON_A1L = ApplesoftVariables::ZP_MON_A1;
  constexpr std::uint8_t kMON_CHKSUM = ApplesoftVariables::ZP_MON_CHKSUM;
  constexpr std::uint8_t kReadLoopIndex = 0x3bu;
  constexpr std::uint8_t kCompensatedIndex = 0x35u;

  std::uint8_t bitTimingIndex = kReadLoopIndex;
  bool carry_set = false;
  do {
    const std::uint8_t value = MON_RDBYTE();
    const std::uint16_t a1Ptr = ReadZeroPageWord(kMON_A1L);
    WriteProgramByte(a1Ptr, value);

    const std::uint8_t runningChecksum =
        static_cast<std::uint8_t>(value ^ ReadZeroPageByte(kMON_CHKSUM));
    WriteZeroPageByte(kMON_CHKSUM, runningChecksum);

    carry_set = MON_NXTA1();
    bitTimingIndex = kCompensatedIndex;
  } while (!carry_set);

  const std::uint8_t checksumByte = MON_RDBYTE();
  const std::uint8_t runningChecksum = ReadZeroPageByte(kMON_CHKSUM);
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
  constexpr std::uint8_t kMON_A1L = ApplesoftVariables::ZP_MON_A1;
  constexpr std::uint8_t kMON_A2L = ApplesoftVariables::ZP_MON_A2;

  std::uint16_t a1 = ReadZeroPageWord(kMON_A1L);
  const std::uint16_t a2 = ReadZeroPageWord(kMON_A2L);

  a1 = static_cast<std::uint16_t>(a1 + 1u);
  WriteZeroPageWord(kMON_A1L, a1);
  return a1 >= a2;
}

void MON_RESTORE() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // MON_Labels: RESTORE (inclusive) .. SAVE (exclusive)
  // Name normalization: none (assembler label RESTORE is prefixed with MON_ in
  // C++).
  //
  // Restore 6502 register state from zero-page storage (used by debug
  // software). Restores accumulator, X, Y registers and processor status flags
  // from fixed locations. Original sequence: Load A from $48 (status), push;
  // Load A from $45; Load X from $46; Load Y from $47; Restore processor flags
  // via PLP; RTS.

  constexpr std::uint8_t kMON_STATUS = ApplesoftVariables::ZP_MON_STATUS;
  constexpr std::uint8_t kMON_DEBUG_REG_A =
      ApplesoftVariables::ZP_MON_DEBUG_REG_A;
  constexpr std::uint8_t kMON_DEBUG_REG_X =
      ApplesoftVariables::ZP_MON_DEBUG_REG_X;
  constexpr std::uint8_t kMON_DEBUG_REG_Y =
      ApplesoftVariables::ZP_MON_DEBUG_REG_Y;

  // Read saved processor status (this will be re-pushed via PLP emulation).
  const std::uint8_t saved_status = ReadZeroPageByte(kMON_STATUS);

  // Read saved register values from zero-page storage.
  const std::uint8_t saved_reg_a = ReadZeroPageByte(kMON_DEBUG_REG_A);
  const std::uint8_t saved_reg_x = ReadZeroPageByte(kMON_DEBUG_REG_X);
  const std::uint8_t saved_reg_y = ReadZeroPageByte(kMON_DEBUG_REG_Y);

  // In 6502 emulation, these values would restore the virtual CPU state.
  // TODO(asm-port): Restore processor flags (status register) to emulated CPU
  // state.
  // TODO(asm-port): Wire accumulator restoration into instruction execution
  // pipeline.
  (void)saved_status;
  (void)saved_reg_a;
  (void)saved_reg_x;
  (void)saved_reg_y;
}

void MON_PRERR() {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/cmd.o65.lst
  // MON_Labels: PRERR (inclusive) .. BELL (exclusive)
  // Name normalization: none (assembler label PRERR is prefixed with MON_ in
  // C++).
  //
  // PRINT "ERR", THEN BELL.
  MON_COUT(0xc5u); // 'E' with high bit set
  MON_COUT(0xd2u); // 'R' with high bit set
  MON_COUT(0xd2u); // 'R' with high bit set

  // PRERR does not terminate; it falls through directly into BELL.
  MON_BELL();
}

} // namespace applesoft::asm_port