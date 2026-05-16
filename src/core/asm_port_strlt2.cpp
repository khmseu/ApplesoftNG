#include "core/asm_port_strlt2.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"

#include <algorithm>
#include <cstdint>

namespace applesoft::asm_port {

void AS_CHKSTR();
void AS_GARBAG();
void AS_STRINI(std::uint8_t length);
void AS_STRSPA(std::uint8_t length);
void AS_MOVINS();
std::uint8_t AS_FRETMP(std::uint16_t descriptorAddress);
bool AS_FRETMS(std::uint16_t descriptorAddress);
std::uint8_t AS_GETBYT();
void AS_IQERR();
void AS_CHKCLS();
void AS_CONINT();
void AS_SNGFLT(std::uint8_t value);

namespace {

std::uint8_t read_AS_CHARAC() { return variables_const().AS_CHARAC; }

std::uint8_t read_AS_ENDCHR() { return variables_const().AS_ENDCHR; }

void write_AS_STRNG1(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_STRNG1, value);
}

void write_AS_FAC(std::uint8_t v) { variables().AS_FAC[0] = v; }

void write_AS_FAC_pointer(std::uint16_t value) {
  variables().writeWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 1u), value);
}

void write_AS_STRNG2(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_STRNG2, value);
}

std::uint8_t read_AS_TEMPPT() { return variables_const().AS_TEMPPT; }

void write_AS_TEMPPT(std::uint8_t value) { variables().AS_TEMPPT = value; }

void write_AS_LASTPT(std::uint8_t value) { variables().AS_LASTPT = value; }

std::uint8_t read_AS_GARFLG() { return variables_const().AS_GARFLG; }

void write_AS_GARFLG(std::uint8_t value) { variables().AS_GARFLG = value; }

std::uint16_t read_AS_FRETOP() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_FRETOP);
}

void write_AS_FRETOP(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_FRETOP, value);
}

void write_AS_FRESPC(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_FRESPC, value);
}

std::uint16_t read_AS_MEMSIZ() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_MEMSIZ);
}

std::uint16_t read_AS_STREND() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_STREND);
}

std::uint16_t read_AS_VARTAB() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_VARTAB);
}

std::uint16_t read_AS_ARYTAB() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_ARYTAB);
}

std::uint16_t read_AS_INDEX() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_INDEX);
}

void write_AS_INDEX(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_INDEX, value);
}

std::uint16_t read_AS_LOWTR() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_LOWTR);
}

void write_AS_LOWTR(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_LOWTR, value);
}

std::uint16_t read_AS_FNCNAM() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_FNCNAM);
}

void write_AS_FNCNAM(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_FNCNAM, value);
}

void clear_AS_FNCNAM_hi() {
  auto &vars = variables();
  ApplesoftVariables::setHighByte(vars.AS_FNCNAM, 0u);
}

std::uint8_t read_AS_DSCLEN() { return variables_const().AS_DSCLEN; }

void write_AS_DSCLEN(std::uint8_t value) { variables().AS_DSCLEN = value; }

std::uint8_t read_AS_LENGTH() {
  return variables_const().readByte(ApplesoftVariables::ZP_AS_LENGTH);
}

void write_AS_LENGTH(std::uint8_t value) {
  // Dual-use storage: AS_LENGTH aliases AS_JMPADRS low-byte in ROM.
  variables().writeByte(ApplesoftVariables::ZP_AS_LENGTH, value);
}

std::uint16_t read_AS_ARYPNT() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_ARYPNT);
}

void write_AS_ARYPNT(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_ARYPNT, value);
}

void write_AS_HIGHTR(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_HIGHTR, value);
}

void write_AS_DSCPTR(std::uint16_t value) {
  variables().writeWord(ApplesoftVariables::ZP_AS_DSCPTR, value);
}

std::uint16_t read_AS_DSCPTR() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_DSCPTR);
}

std::uint16_t read_AS_STRNG1() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_STRNG1);
}

std::uint16_t read_AS_FRESPC() {
  return variables_const().readWord(ApplesoftVariables::ZP_AS_FRESPC);
}

std::uint16_t read_AS_FAC_descriptor_address() {
  return variables_const().readWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 3u));
}

void write_AS_FAC_descriptor_address(std::uint16_t value) {
  variables().writeWord(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 3u), value);
}

std::uint8_t g_jerr_error = AS_ERR_FRMCPX;

void AS_JERR();
void AS_PUTEMP(std::uint8_t tempDescriptorAddress);
std::uint16_t AS_GETSPA(std::uint8_t length);
void AS_FIND_HIGHEST_STRING();
void AS_CHECK_SIMPLE_VARIABLE();
void AS_CHECK_VARIABLE(std::uint8_t descriptorOffset);
void AS_CHECK_BUMP();
void AS_CHECK_EXIT();
void AS_MOVE_HIGHEST_STRING_TO_TOP();
void AS_PUTNEW();
void AS_CAT();
void AS_MOVSTR(std::uint8_t x, std::uint8_t y, std::uint8_t length);
void AS_MOVSTR_1(std::uint8_t length);
std::uint8_t AS_FRESTR();

// TODO(asm-port): port AS_FRM_ELEMENT label.
void AS_FRM_ELEMENT() {}

// TODO(asm-port): port AS_FRMEVL_2 label.
void AS_FRMEVL_2() {}

struct SubstringSetupResult {
  std::uint8_t firstParameter;
  std::uint8_t sourceAS_Length;
};

bool AS_SUBSTRING_SETUP(SubstringSetupResult &out) {
  // Source:
  // SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
  // AS_Labels: AS_SUBSTRING_SETUP (inclusive) .. AS_LEN (exclusive)
  // Name normalization: none (assembler label AS_SUBSTRING_SETUP kept
  // verbatim).
  AS_CHKCLS();

  const std::uint8_t first = variables_const().readByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_FAC + 4u));
  if (first == 0u) {
    AS_IQERR();
    return false;
  }

  const std::uint16_t descriptor = read_AS_DSCPTR();
  const auto descriptorPtr = variables_const().pointer(descriptor);
  const std::uint8_t sourceAS_Length = descriptorPtr.read(0u);
  const std::uint16_t source = ApplesoftVariables::makeWord(
      descriptorPtr.read(1u), descriptorPtr.read(2u));
  write_AS_INDEX(source);

  out.firstParameter = first;
  out.sourceAS_Length = sourceAS_Length;
  return true;
}

void SUBSTRING_BUILD(std::uint8_t leftStart, std::uint8_t copyAS_Length) {
  AS_STRSPA(copyAS_Length);
  (void)AS_FRETMP(read_AS_DSCPTR());

  const std::uint16_t source = read_AS_INDEX();
  write_AS_INDEX(static_cast<std::uint16_t>(source + leftStart));
  AS_MOVSTR_1(copyAS_Length);
  AS_PUTNEW();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_GETSPA (inclusive) .. AS_GARBAG (exclusive)
// Name normalization: none (assembler label AS_GETSPA kept verbatim).
std::uint16_t AS_GETSPA(std::uint8_t length) {
  // lsr AS_GARFLG: clear sign bit while preserving lower bits.
  write_AS_GARFLG(static_cast<std::uint8_t>(read_AS_GARFLG() >> 1u));

  while (true) {
    // Pointer candidate lifted: AS_FRETOP/AS_FRETOP+1 and AS_STREND/AS_STREND+1
    // are unified words.
    const std::uint16_t startAddress =
        static_cast<std::uint16_t>(read_AS_FRETOP() - length);
    const std::uint16_t strend = read_AS_STREND();

    if (startAddress >= strend) {
      write_AS_FRETOP(startAddress);
      write_AS_FRESPC(startAddress);
      return startAddress;
    }

    g_jerr_error = AS_ERR_MEMFULL;
    if ((read_AS_GARFLG() & 0x80u) != 0u) {
      AS_JERR();
      return startAddress;
    }

    // Original control flow falls through AS_GETSPA error path into AS_GARBAG.
    AS_GARBAG();
    write_AS_GARFLG(0x80u);
  }
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FIND_HIGHEST_STRING (inclusive) .. AS_CHECK_SIMPLE_VARIABLE
// (exclusive) Name normalization: none (assembler label AS_FIND_HIGHEST_STRING
// kept verbatim).
void AS_FIND_HIGHEST_STRING() {
  clear_AS_FNCNAM_hi();
  write_AS_LOWTR(read_AS_STREND());

  // Scan temporary descriptors.
  write_AS_INDEX(ApplesoftVariables::ZP_AS_TEMPST);
  while (read_AS_INDEX() < read_AS_TEMPPT()) {
    AS_CHECK_VARIABLE(0u);
  }

  // Scan simple variables.
  write_AS_DSCLEN(7u);
  write_AS_INDEX(read_AS_VARTAB());
  while (read_AS_INDEX() < read_AS_ARYTAB()) {
    AS_CHECK_SIMPLE_VARIABLE();
  }

  // Scan arrays for string descriptors.
  write_AS_ARYPNT(read_AS_ARYTAB());
  write_AS_DSCLEN(3u);
  while (read_AS_ARYPNT() < read_AS_STREND()) {
    const std::uint16_t arrayDescriptor = read_AS_ARYPNT();
    write_AS_INDEX(arrayDescriptor);

    const auto descriptor = variables_const().pointer(arrayDescriptor);
    const std::uint8_t firstChar = descriptor.read(0u);
    const std::uint8_t secondChar = descriptor.read(1u);
    const std::uint16_t nextArray = static_cast<std::uint16_t>(
        arrayDescriptor +
        ApplesoftVariables::makeWord(descriptor.read(2u), descriptor.read(3u)));
    write_AS_ARYPNT(nextArray);

    const bool isStringArray =
        (secondChar & 0x80u) != 0u && (firstChar & 0x80u) == 0u;
    if (!isStringArray) {
      continue;
    }

    const std::uint8_t numDims = descriptor.read(4u);
    const std::uint16_t firstElement = static_cast<std::uint16_t>(
        arrayDescriptor + static_cast<std::uint16_t>(numDims * 2u + 5u));
    write_AS_INDEX(firstElement);

    while (read_AS_INDEX() < read_AS_ARYPNT()) {
      AS_CHECK_VARIABLE(0u);
    }
  }

  // Control transfers to AS_MOVE_HIGHEST_STRING_TO_TOP in ROM.
  AS_MOVE_HIGHEST_STRING_TO_TOP();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHECK_SIMPLE_VARIABLE (inclusive) .. AS_CHECK_VARIABLE
// (exclusive) Name normalization: none (assembler label
// AS_CHECK_SIMPLE_VARIABLE kept verbatim).
void AS_CHECK_SIMPLE_VARIABLE() {
  const auto descriptor = variables_const().pointer(read_AS_INDEX());
  const std::uint8_t firstChar = descriptor.read(0u);
  if ((firstChar & 0x80u) != 0u) {
    AS_CHECK_BUMP();
    return;
  }

  const std::uint8_t secondChar = descriptor.read(1u);
  if ((secondChar & 0x80u) == 0u) {
    AS_CHECK_BUMP();
    return;
  }

  AS_CHECK_VARIABLE(2u);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHECK_VARIABLE (inclusive) .. AS_CHECK_BUMP (exclusive)
// Name normalization: none (assembler label AS_CHECK_VARIABLE kept verbatim).
void AS_CHECK_VARIABLE(std::uint8_t descriptorOffset) {
  const std::uint16_t descriptorAddress = read_AS_INDEX();
  const auto descriptor = variables_const().pointer(descriptorAddress);
  const std::uint8_t length = descriptor.read(descriptorOffset);
  if (length == 0u) {
    AS_CHECK_BUMP();
    return;
  }

  const std::uint16_t stringAddress = ApplesoftVariables::makeWord(
      descriptor.read(static_cast<std::uint16_t>(descriptorOffset + 1u)),
      descriptor.read(static_cast<std::uint16_t>(descriptorOffset + 2u)));

  // Ignore strings already collected at/above AS_FRETOP.
  if (stringAddress >= read_AS_FRETOP()) {
    AS_CHECK_BUMP();
    return;
  }

  // Keep highest string below AS_FRETOP.
  if (stringAddress < read_AS_LOWTR()) {
    AS_CHECK_BUMP();
    return;
  }

  write_AS_LOWTR(stringAddress);
  write_AS_FNCNAM(descriptorAddress);
  write_AS_LENGTH(read_AS_DSCLEN());
  AS_CHECK_BUMP();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHECK_BUMP (inclusive) .. AS_CHECK_EXIT (exclusive)
// Name normalization: none (assembler label AS_CHECK_BUMP kept verbatim).
void AS_CHECK_BUMP() {
  write_AS_INDEX(
      static_cast<std::uint16_t>(read_AS_INDEX() + read_AS_DSCLEN()));
  AS_CHECK_EXIT();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHECK_EXIT (inclusive) .. AS_MOVE_HIGHEST_STRING_TO_TOP
// (exclusive) Name normalization: none (assembler label AS_CHECK_EXIT kept
// verbatim).
void AS_CHECK_EXIT() {
  // ROM epilogue: ldx INDEX+1 ; ldy #0 ; rts.
  // This port models the return-register outcomes as local values because
  // callers in the current C++ path consume AS_INDEX directly.
  [[maybe_unused]] const std::uint8_t xOnReturn =
      ApplesoftVariables::highByte(read_AS_INDEX());
  [[maybe_unused]] constexpr std::uint8_t yOnReturn = 0u;
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_MOVE_HIGHEST_STRING_TO_TOP (inclusive) .. AS_CAT (exclusive)
// Name normalization: none (assembler label AS_MOVE_HIGHEST_STRING_TO_TOP kept
// verbatim).
void AS_MOVE_HIGHEST_STRING_TO_TOP() {
  const std::uint16_t descriptorAddress = read_AS_FNCNAM();
  if (ApplesoftVariables::highByte(descriptorAddress) == 0u) {
    AS_CHECK_EXIT();
    return;
  }

  auto descriptor = variables().pointer(descriptorAddress);
  const std::uint8_t lengthSlotOffset =
      static_cast<std::uint8_t>((read_AS_LENGTH() & 0x04u) >> 1u);
  write_AS_LENGTH(lengthSlotOffset);

  const std::uint8_t stringAS_Length = descriptor.read(lengthSlotOffset);
  const std::uint16_t sourceAddress = read_AS_LOWTR();
  const std::uint16_t highSource =
      static_cast<std::uint16_t>(sourceAddress + stringAS_Length);
  write_AS_HIGHTR(highSource);

  // AS_HIGHDS/AS_HIGHDS+1 aliases AS_ARYPNT in this memory layout.
  const std::uint16_t destination =
      static_cast<std::uint16_t>(read_AS_FRETOP() - stringAS_Length);
  write_AS_ARYPNT(destination);

  // Preserve copy intent of AS_BLTU2 for mapped memory regions.
  if (stringAS_Length != 0u) {
    for (std::uint16_t i = stringAS_Length; i > 0u; --i) {
      const std::uint8_t b = variables_const().readByte(
          static_cast<std::uint16_t>(sourceAddress + i - 1u));
      variables().writeByte(static_cast<std::uint16_t>(destination + i - 1u),
                            b);
    }
  }

  write_AS_FRETOP(destination);

  const std::uint8_t pointerOffset =
      static_cast<std::uint8_t>(lengthSlotOffset + 1u);
  descriptor.write(ApplesoftVariables::lowByte(destination), pointerOffset);
  descriptor.write(ApplesoftVariables::highByte(destination),
                   static_cast<std::uint16_t>(pointerOffset + 1u));

  // Continue scanning until no more non-empty strings remain below AS_FRETOP.
  AS_FIND_HIGHEST_STRING();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CAT (inclusive) .. AS_MOVINS (exclusive)
// Name normalization: none (assembler label AS_CAT kept verbatim).
static void AS_CAT_impl() {
  const std::uint16_t firstDescriptor = read_AS_FAC_descriptor_address();
  write_AS_STRNG1(firstDescriptor);

  AS_FRM_ELEMENT();
  AS_CHKSTR();

  const std::uint16_t secondDescriptor = read_AS_FAC_descriptor_address();
  const auto first = variables_const().pointer(firstDescriptor);
  const auto second = variables_const().pointer(secondDescriptor);

  const std::uint16_t totalAS_Length =
      static_cast<std::uint16_t>(first.read() + second.read());
  if (totalAS_Length > 0xffu) {
    AS_ERROR(AS_ERR_STRLONG);
    return;
  }

  AS_STRINI(static_cast<std::uint8_t>(totalAS_Length));
  AS_MOVINS();

  const std::uint8_t secondAS_Length = AS_FRETMP(read_AS_DSCPTR());
  AS_MOVSTR_1(secondAS_Length);

  (void)AS_FRETMP(read_AS_STRNG1());
  AS_PUTNEW();
  AS_FRMEVL_2();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_MOVSTR (inclusive) .. AS_MOVSTR_1 (exclusive)
// Name normalization: none (assembler label AS_MOVSTR kept verbatim).
void AS_MOVSTR(std::uint8_t x, std::uint8_t y, std::uint8_t length) {
  // Pointer candidate lifted: AS_INDEX low/high is one conceptual source
  // pointer.
  write_AS_INDEX(ApplesoftVariables::makeWord(x, y));
  AS_MOVSTR_1(length);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_MOVSTR_1 (inclusive) .. AS_FRESTR (exclusive)
// Name normalization: none (assembler label AS_MOVSTR_1 kept verbatim).
void AS_MOVSTR_1(std::uint8_t length) {
  const std::uint16_t source = read_AS_INDEX();
  const std::uint16_t destination = read_AS_FRESPC();

  for (std::uint16_t i = 0; i < length; ++i) {
    const std::uint8_t b =
        variables_const().readByte(static_cast<std::uint16_t>(source + i));
    variables().writeByte(static_cast<std::uint16_t>(destination + i), b);
  }

  // Carry-chain AS_FRESPC/AS_FRESPC+1 update lifted to unified pointer
  // arithmetic.
  write_AS_FRESPC(static_cast<std::uint16_t>(destination + length));
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FRESTR (inclusive) .. AS_FREFAC (exclusive)
// Name normalization: none (assembler label AS_FRESTR kept verbatim).
std::uint8_t AS_FRESTR() {
  AS_CHKSTR();
  return AS_FREFAC();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FREFAC (inclusive) .. AS_FRETMP (exclusive)
// Name normalization: none (assembler label AS_FREFAC kept verbatim).
std::uint8_t AS_FREFAC_impl() {
  return AS_FRETMP(read_AS_FAC_descriptor_address());
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_PUTNEW (inclusive) .. AS_JERR (exclusive)
// Name normalization: none (assembler label AS_PUTNEW kept verbatim).
void AS_PUTNEW() {
  const std::uint8_t tempDescriptorAddress = read_AS_TEMPPT();
  const std::uint8_t maxTempDescriptorAddress =
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_TEMPST + 9u);
  if (tempDescriptorAddress == maxTempDescriptorAddress) {
    g_jerr_error = AS_ERR_FRMCPX;
    AS_JERR();
    return;
  }

  // Branch target in ROM is AS_PUTEMP.
  AS_PUTEMP(tempDescriptorAddress);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_JERR (inclusive) .. AS_PUTEMP (exclusive)
// Name normalization: none (assembler label AS_JERR kept verbatim).
void AS_JERR() { AS_ERROR(g_jerr_error); }

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_PUTEMP (inclusive) .. AS_GETSPA (exclusive)
// Name normalization: none (assembler label AS_PUTEMP kept verbatim).
void AS_PUTEMP(std::uint8_t tempDescriptorAddress) {
  auto tempDescriptor = variables().pointer(tempDescriptorAddress);
  const auto &vars = variables_const();
  tempDescriptor.write(vars.AS_FAC[0]);
  tempDescriptor.write(vars.AS_FAC[1], 1u);
  tempDescriptor.write(vars.AS_FAC[2], 2u);

  // AS_FAC+3/AS_FAC+4 is one logical pointer to the temp descriptor.
  write_AS_FAC_descriptor_address(
      static_cast<std::uint16_t>(tempDescriptorAddress));

  // AS_VALTYP=$ff marks AS_FAC as string.
  variables().AS_VALTYP = 0xffu;
  write_AS_LASTPT(tempDescriptorAddress);

  write_AS_TEMPPT(static_cast<std::uint8_t>(tempDescriptorAddress + 3u));
}

void AS_STRLIT(std::uint16_t address);
} // namespace

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FREFAC (inclusive) .. AS_FRETMP (exclusive)
// Name normalization: none (assembler label AS_FREFAC kept verbatim).
std::uint8_t AS_FREFAC() { return AS_FREFAC_impl(); }

// AS_Labels: AS_STRINI (inclusive) .. AS_STRSPA (exclusive)
// Name normalization: none (assembler label AS_STRINI kept verbatim).
void AS_STRINI(std::uint8_t length) {
  // Pointer candidate lifted: AS_FAC+3/AS_FAC+4 is one descriptor pointer.
  const std::uint16_t descriptorAddress = read_AS_FAC_descriptor_address();
  write_AS_DSCPTR(descriptorAddress);

  // Original control flow falls through directly into AS_STRSPA.
  AS_STRSPA(length);
}

void AS_CAT() { AS_CAT_impl(); }

// AS_Labels: AS_STRSPA (inclusive) .. AS_L_STRSPA_1 (exclusive)
// Name normalization: none (assembler label AS_STRSPA kept verbatim).
void AS_STRSPA(std::uint8_t length) {
  // AS_GETSPA: allocate space at bottom of string space (A=length).
  // Returns address in Y,X.
  const std::uint16_t allocatedAddress = AS_GETSPA(length);

  // Store length and address into AS_FAC.
  variables().AS_FAC[0] = length;
  write_AS_FAC_pointer(allocatedAddress);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_LEN (inclusive) .. AS_GETSTR (exclusive)
// Name normalization: none (assembler label AS_LEN kept verbatim).
//
// "AS_LEN" built-in: get string length and float it into AS_FAC.
void AS_LEN() {
  const std::uint8_t length = AS_GETSTR();
  AS_SNGFLT(length);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_GETSTR (inclusive) .. AS_ASC (exclusive)
// Name normalization: none (assembler label AS_GETSTR kept verbatim).
//
// Free the AS_FAC string if temporary, clear AS_VALTYP to numeric, return
// length. After return AS_INDEX points at the string data.
std::uint8_t AS_GETSTR() {
  const std::uint8_t length = AS_FRESTR();
  variables().AS_VALTYP = 0u;
  return length;
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_ASC (inclusive) .. AS_GOIQ (exclusive)
// Name normalization: none (assembler label AS_ASC kept verbatim).
//
// "AS_ASC" built-in: return AS_ASCII value of first character.
// AS_GOIQ ($e6f2) is a one-instruction trampoline to AS_IQERR; inlined here.
void AS_ASC() {
  const std::uint8_t length = AS_GETSTR();
  if (length == 0u) {
    AS_IQERR(); // AS_GOIQ: jmp AS_IQERR — illegal quantity for empty string
    return;
  }
  const std::uint16_t strAddr = read_AS_INDEX();
  const std::uint8_t ch = variables_const().readByte(strAddr);
  AS_SNGFLT(ch);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_GARBAG (inclusive) .. AS_FIND_HIGHEST_STRING (exclusive)
// Name normalization: none (assembler label AS_GARBAG kept verbatim).
void AS_GARBAG() {
  // Collect from top down: initialize AS_FRETOP from AS_MEMSIZ, then fall
  // through.
  write_AS_FRETOP(read_AS_MEMSIZ());
  AS_FIND_HIGHEST_STRING();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_MOVINS (inclusive) .. AS_MOVSTR (exclusive)
// Name normalization: none (assembler label AS_MOVINS kept verbatim).
void AS_MOVINS() {
  const auto descriptor = variables_const().pointer(read_AS_STRNG1());
  const std::uint8_t length = descriptor.read(0u);
  const std::uint8_t x = descriptor.read(1u);
  const std::uint8_t y = descriptor.read(2u);
  AS_MOVSTR(x, y, length);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FRETMP (inclusive) .. AS_FRETMS (exclusive)
// Name normalization: none (assembler label AS_FRETMP kept verbatim).
std::uint8_t AS_FRETMP(std::uint16_t descriptorAddress) {
  write_AS_INDEX(descriptorAddress);
  const bool isTemporary = AS_FRETMS(descriptorAddress);

  const auto descriptor = variables_const().pointer(descriptorAddress);
  const std::uint8_t length = descriptor.read(0u);
  const std::uint16_t stringAddress =
      ApplesoftVariables::makeWord(descriptor.read(1u), descriptor.read(2u));

  if (isTemporary && stringAddress == read_AS_FRETOP()) {
    write_AS_FRETOP(static_cast<std::uint16_t>(read_AS_FRETOP() + length));
  }

  write_AS_INDEX(stringAddress);
  return length;
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_FRETMS (inclusive) .. AS_CHRSTR (exclusive)
// Name normalization: none (assembler label AS_FRETMS kept verbatim).
bool AS_FRETMS(std::uint16_t descriptorAddress) {
  const std::uint8_t a = ApplesoftVariables::lowByte(descriptorAddress);
  const std::uint8_t y = ApplesoftVariables::highByte(descriptorAddress);
  const std::uint8_t lastpt = variables_const().AS_LASTPT;
  const std::uint8_t lastptHi = variables_const().readByte(
      static_cast<std::uint8_t>(ApplesoftVariables::ZP_AS_LASTPT + 1u));

  if (y != lastptHi || a != lastpt) {
    return false;
  }

  // Descriptor is latest temporary: release by rewinding AS_TEMPPT/AS_LASTPT.
  write_AS_TEMPPT(a);
  write_AS_LASTPT(static_cast<std::uint8_t>(a - 3u));
  return true;
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_CHRSTR (inclusive) .. AS_LEFTSTR (exclusive)
// Name normalization: none (assembler label AS_CHRSTR kept verbatim).
//
// CHR$() built-in: parse the argument byte via AS_CONINT, allocate a 1-byte
// string with AS_STRSPA, store the character into the newly allocated space,
// then publish a temporary string descriptor with AS_PUTNEW. ROM pops two
// return addresses before jumping to AS_PUTNEW because AS_CHRSTR was reached
// via the AS_UNFNC dispatch JSR chain; in C++ the call frames unwind normally
// so no explicit pop is needed.
void AS_CHRSTR() {
  // AS_CONINT: evaluate the current AS_FAC argument as an integer byte (0-255).
  // The ROM convention is X register = result byte.  In the C++ port, AS_CONINT
  // writes the converted value into AS_FAC[4] (the X-register proxy at
  // ZP_AS_FAC+4); we read it back from there.
  AS_CONINT();
  const std::uint8_t ch = variables_const().AS_FAC[4];

  // Allocate space for the 1-byte string.  AS_STRSPA sets AS_FAC[0]=1 (length)
  // and AS_FAC+1,2 = allocated address.
  AS_STRSPA(1u);

  // sta (AS_FAC+1),Y  with Y=0: write the character to the allocated string
  // space. AS_FAC+1/AS_FAC+2 is one unified pointer (write_AS_FAC_pointer uses
  // the same address).
  const std::uint16_t strData = ApplesoftVariables::makeWord(
      variables_const().AS_FAC[1], variables_const().AS_FAC[2]);
  variables().writeByte(strData, ch);

  // jmp AS_PUTNEW: convert the raw AS_FAC string data into a temporary
  // descriptor.
  AS_PUTNEW();
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_LEFTSTR (inclusive) .. AS_RIGHTSTR (exclusive)
// Name normalization: none (assembler label AS_LEFTSTR kept verbatim).
void AS_LEFTSTR() {
  SubstringSetupResult setup{};
  if (!AS_SUBSTRING_SETUP(setup)) {
    return;
  }

  const std::uint8_t copyAS_Length =
      std::min(setup.firstParameter, setup.sourceAS_Length);
  SUBSTRING_BUILD(0u, copyAS_Length);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_RIGHTSTR (inclusive) .. AS_MIDSTR (exclusive)
// Name normalization: none (assembler label AS_RIGHTSTR kept verbatim).
void AS_RIGHTSTR() {
  SubstringSetupResult setup{};
  if (!AS_SUBSTRING_SETUP(setup)) {
    return;
  }

  const std::uint8_t copyAS_Length =
      std::min(setup.firstParameter, setup.sourceAS_Length);
  const std::uint8_t leftStart =
      static_cast<std::uint8_t>(setup.sourceAS_Length - copyAS_Length);
  SUBSTRING_BUILD(leftStart, copyAS_Length);
}

// Source:
// SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// AS_Labels: AS_MIDSTR (inclusive) .. AS_SUBSTRING_SETUP (exclusive)
// Name normalization: none (assembler label AS_MIDSTR kept verbatim).
void AS_MIDSTR() {
  std::uint8_t requestedWidth = 0xffu;
  if (AS_CHRGOT() != static_cast<std::uint8_t>(')')) {
    AS_CHKCOM();
    requestedWidth = AS_GETBYT();
  }

  SubstringSetupResult setup{};
  if (!AS_SUBSTRING_SETUP(setup)) {
    return;
  }

  const std::uint8_t leftStart =
      static_cast<std::uint8_t>(setup.firstParameter - 1u);
  if (leftStart >= setup.sourceAS_Length) {
    SUBSTRING_BUILD(leftStart, 0u);
    return;
  }

  const std::uint8_t remaining =
      static_cast<std::uint8_t>(setup.sourceAS_Length - leftStart);
  const std::uint8_t copyAS_Length = std::min(remaining, requestedWidth);
  SUBSTRING_BUILD(leftStart, copyAS_Length);
}

void AS_STRLT2(std::uint16_t address) {
  // BUILD A DESCRIPTOR AS_FOR AS_STRING STARTING AT address
  // AND TERMINATED BY $00, (AS_CHARAC), AS_OR (AS_ENDCHR)

  const std::uint16_t start = address;
  const auto startPtr = variables_const().pointer(start);
  write_AS_STRNG1(start);
  write_AS_FAC_pointer(start);

  std::uint8_t length = 0;
  bool include_quote_in_length = false;

  // Find end of string, terminated by $00 or alternate terminators.
  while (true) {
    const std::uint8_t ch = startPtr.read(length);
    if (ch == 0) {
      break;
    }
    if (ch == read_AS_CHARAC() || ch == read_AS_ENDCHR()) {
      include_quote_in_length = (ch == static_cast<std::uint8_t>(0x22));
      break;
    }
    ++length;
  }

  if (include_quote_in_length) {
    ++length;
  }

  write_AS_FAC(length);

  const std::uint16_t endAddress = startPtr.advanced(length).address();
  write_AS_STRNG2(endAddress);

  // If source is not on page 0 or page 2, branch directly to AS_PUTNEW.
  const std::uint8_t startPage = ApplesoftVariables::highByte(start);
  if (startPage != 0 && startPage != 2) {
    AS_PUTNEW();
    return;
  }

  // For page 0/2 source, allocate and move string before AS_PUTNEW handling.
  AS_STRINI(length);
  AS_MOVSTR(ApplesoftVariables::lowByte(start),
            ApplesoftVariables::highByte(start), length);

  // Fall-through target in original control flow is AS_PUTNEW.
  AS_PUTNEW();
}

} // namespace applesoft::asm_port
