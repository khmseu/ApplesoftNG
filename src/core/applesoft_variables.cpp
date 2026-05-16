#include "core/applesoft_variables.hpp"

#include "core/io_ports.hpp"

namespace applesoft::asm_port {

namespace {

ApplesoftVariables g_variables;

} // namespace

ApplesoftVariables &variables() { return g_variables; }

const ApplesoftVariables &variables_const() { return g_variables; }

std::uint8_t ApplesoftVariables::lowByte(std::uint16_t value) {
  return static_cast<std::uint8_t>(value & 0xffu);
}

std::uint8_t ApplesoftVariables::highByte(std::uint16_t value) {
  return static_cast<std::uint8_t>((value >> 8) & 0xffu);
}

std::uint16_t ApplesoftVariables::makeWord(std::uint8_t low,
                                           std::uint8_t high) {
  return static_cast<std::uint16_t>(static_cast<std::uint16_t>(high) << 8 |
                                    low);
}

void ApplesoftVariables::setLowByte(std::uint16_t &target, std::uint8_t value) {
  target = static_cast<std::uint16_t>((target & 0xff00u) | value);
}

void ApplesoftVariables::setHighByte(std::uint16_t &target,
                                     std::uint8_t value) {
  target = static_cast<std::uint16_t>((target & 0x00ffu) |
                                      (static_cast<std::uint16_t>(value) << 8));
}

std::uint8_t ApplesoftVariables::readByte(std::uint16_t address) const {
  if (address == ADDR_AS_INPUT_BUFFER_MINUS_4) {
    return AS_INPUT_BUFFER_MINUS_4;
  }

  if (address == ADDR_AS_INPUT_BUFFER_MINUS_3) {
    return AS_INPUT_BUFFER_MINUS_3;
  }

  if (address == ADDR_AS_INPUT_BUFFER_MINUS_1) {
    return AS_INPUT_BUFFER_MINUS_1;
  }

  if (address >= ADDR_AS_INPUT_BUFFER &&
      address <= ADDR_AS_INPUT_BUFFER + 0xffu) {
    return AS_INPUT_BUFFER_PAGE[address - ADDR_AS_INPUT_BUFFER];
  }

  if (address >= IOPorts::ADDR_BASE && address < IOPorts::ADDR_END) {
    return ioPorts_const().readByte(address);
  }

  switch (address) {
  case ZP_AS_GOWARM:
    return AS_GOWARM;
  case ZP_AS_GOWARM + 1:
    return lowByte(AS_GOWARM_TARGET);
  case ZP_AS_GOWARM + 2:
    return highByte(AS_GOWARM_TARGET);
  case ZP_AS_GOSTROUT:
    return AS_GOSTROUT;
  case ZP_AS_GOSTROUT + 1:
    return lowByte(AS_GOSTROUT_TARGET);
  case ZP_AS_GOSTROUT + 2:
    return highByte(AS_GOSTROUT_TARGET);
  case ZP_AS_USR:
    return AS_USR;
  case ZP_AS_USR + 1:
    return lowByte(AS_USR_TARGET);
  case ZP_AS_USR + 2:
    return highByte(AS_USR_TARGET);
  case ZP_AS_CHARAC:
    return AS_CHARAC;
  case ZP_AS_ENDCHR:
    return AS_ENDCHR;
  case ZP_AS_NUMDIM:
    return AS_NUMDIM;
  case ZP_AS_DIMFLG:
    return AS_DIMFLG;
  case ZP_AS_VALTYP:
    return AS_VALTYP;
  case ZP_AS_VALTYP_PLUS_1:
    return AS_VALTYP_PLUS_1;
  case ZP_AS_GARFLG:
    return AS_GARFLG;
  case ZP_AS_SUBFLG:
    return AS_SUBFLG;
  case ZP_AS_INPUTFLG:
    return AS_INPUTFLG;
  case ZP_AS_CPRMASK:
    return AS_CPRMASK;
  case ZP_MON_CH:
    return MON_CH;
  case ZP_MON_CHKSUM:
    return MON_CHKSUM;
  case ZP_MON_COLOR:
    return MON_COLOR;
  case ZP_MON_PROMPT:
    return MON_PROMPT;
  case ZP_MON_A1:
    return lowByte(MON_A1);
  case ZP_MON_A1 + 1:
    return highByte(MON_A1);
  case ZP_MON_A2:
    return lowByte(MON_A2);
  case ZP_MON_A2 + 1:
    return highByte(MON_A2);
  case ZP_AS_LINNUM:
    return lowByte(AS_LINNUM);
  case ZP_AS_LINNUM + 1:
    return highByte(AS_LINNUM);
  case ZP_AS_TEMPPT:
    return AS_TEMPPT;
  case ZP_AS_LASTPT:
    return AS_LASTPT;
  case ZP_AS_INDEX:
    return lowByte(AS_INDEX);
  case ZP_AS_INDEX + 1:
    return highByte(AS_INDEX);
  case ZP_AS_RESULT:
    return AS_RESULT[0];
  case ZP_AS_RESULT + 1:
    return AS_RESULT[1];
  case ZP_AS_RESULT + 2:
    return AS_RESULT[2];
  case ZP_AS_RESULT + 3:
    return AS_RESULT[3];
  case ZP_AS_TXTTAB:
    return lowByte(AS_TXTTAB);
  case ZP_AS_TXTTAB + 1:
    return highByte(AS_TXTTAB);
  case ZP_AS_VARTAB:
    return lowByte(AS_VARTAB);
  case ZP_AS_VARTAB + 1:
    return highByte(AS_VARTAB);
  case ZP_AS_ARYTAB:
    return lowByte(AS_ARYTAB);
  case ZP_AS_ARYTAB + 1:
    return highByte(AS_ARYTAB);
  case ZP_AS_STREND:
    return lowByte(AS_STREND);
  case ZP_AS_STREND + 1:
    return highByte(AS_STREND);
  case ZP_AS_FRETOP:
    return lowByte(AS_FRETOP);
  case ZP_AS_FRETOP + 1:
    return highByte(AS_FRETOP);
  case ZP_AS_FRESPC:
    return lowByte(AS_FRESPC);
  case ZP_AS_FRESPC + 1:
    return highByte(AS_FRESPC);
  case ZP_AS_MEMSIZ:
    return lowByte(AS_MEMSIZ);
  case ZP_AS_MEMSIZ + 1:
    return highByte(AS_MEMSIZ);
  case ZP_AS_CURLIN:
    return lowByte(AS_CURLIN);
  case ZP_AS_CURLIN + 1:
    return highByte(AS_CURLIN);
  case ZP_AS_OLDLIN:
    return lowByte(AS_OLDLIN);
  case ZP_AS_OLDLIN + 1:
    return highByte(AS_OLDLIN);
  case ZP_AS_OLDTEXT:
    return lowByte(AS_OLDTEXT);
  case ZP_AS_OLDTEXT + 1:
    return highByte(AS_OLDTEXT);
  case ZP_AS_DATLIN:
    return lowByte(AS_DATLIN);
  case ZP_AS_DATLIN + 1:
    return highByte(AS_DATLIN);
  case ZP_AS_DATPTR:
    return lowByte(AS_DATPTR);
  case ZP_AS_DATPTR + 1:
    return highByte(AS_DATPTR);
  case ZP_AS_INPTR:
    return lowByte(AS_INPTR);
  case ZP_AS_INPTR + 1:
    return highByte(AS_INPTR);
  case ZP_AS_VARNAM:
    return lowByte(AS_VARNAM);
  case ZP_AS_VARNAM + 1:
    return highByte(AS_VARNAM);
  case ZP_AS_VARPNT:
    return lowByte(AS_VARPNT);
  case ZP_AS_VARPNT + 1:
    return highByte(AS_VARPNT);
  case ZP_AS_FORPNT:
    return lowByte(AS_FORPNT);
  case ZP_AS_FORPNT + 1:
    return highByte(AS_FORPNT);
  case ZP_AS_TXPSV:
    return lowByte(AS_TXPSV);
  case ZP_AS_TXPSV + 1:
    return highByte(AS_TXPSV);
  case ZP_AS_CPRTYP:
    return AS_CPRTYP;
  case ZP_AS_FNCNAM:
    return lowByte(AS_FNCNAM);
  case ZP_AS_FNCNAM + 1:
    return highByte(AS_FNCNAM);
  case ZP_AS_DSCPTR:
    return lowByte(AS_DSCPTR);
  case ZP_AS_DSCPTR + 1:
    return highByte(AS_DSCPTR);
  case ZP_AS_DSCLEN:
    return AS_DSCLEN;
  case ZP_AS_JMPADRS:
    return AS_JMPADRS_OPCODE;
  case ZP_AS_JMPADRS + 1:
    return lowByte(AS_JMPADRS);
  case ZP_AS_JMPADRS + 2:
    return highByte(AS_JMPADRS);
  case ZP_AS_ARYPNT:
    return lowByte(AS_ARYPNT);
  case ZP_AS_ARYPNT + 1:
    return highByte(AS_ARYPNT);
  case ZP_AS_HIGHTR:
    return lowByte(AS_HIGHTR);
  case ZP_AS_HIGHTR + 1:
    return highByte(AS_HIGHTR);
  case ZP_AS_INDX:
    return AS_INDX;
  case ZP_AS_LOWTR:
    return lowByte(AS_LOWTR);
  case ZP_AS_LOWTR + 1:
    return highByte(AS_LOWTR);
  case ZP_AS_FAC:
    return AS_FAC[0];
  case ZP_AS_FAC + 1:
    return AS_FAC[1];
  case ZP_AS_FAC + 2:
    return AS_FAC[2];
  case ZP_AS_FAC + 3:
    return AS_FAC[3];
  case ZP_AS_FAC + 4:
    return AS_FAC[4];
  case ZP_AS_FAC_SIGN:
    return AS_FAC_SIGN;
  case ZP_AS_SHIFT_SIGN_EXT:
    return AS_SHIFT_SIGN_EXT;
  case ZP_AS_ARG:
    return AS_ARG[0];
  case ZP_AS_ARG + 1:
    return AS_ARG[1];
  case ZP_AS_ARG + 2:
    return AS_ARG[2];
  case ZP_AS_ARG + 3:
    return AS_ARG[3];
  case ZP_AS_ARG + 4:
    return AS_ARG[4];
  case ZP_AS_ARG + 5:
    return AS_ARG[5];
  case ZP_AS_STRNG1:
    return lowByte(AS_STRNG1);
  case ZP_AS_STRNG1 + 1:
    return highByte(AS_STRNG1);
  case ZP_AS_STRNG2:
    return lowByte(AS_STRNG2);
  case ZP_AS_STRNG2 + 1:
    return highByte(AS_STRNG2);
  case ZP_AS_PRGEND:
    return lowByte(AS_PRGEND);
  case ZP_AS_PRGEND + 1:
    return highByte(AS_PRGEND);
  case ZP_AS_TXTPTR:
    return lowByte(AS_TXTPTR);
  case ZP_AS_TXTPTR + 1:
    return highByte(AS_TXTPTR);
  case ZP_AS_LOCK:
    return AS_LOCK;
  case ZP_AS_ERRFLG:
    return AS_ERRFLG;
  case ZP_AS_FIRST:
    return AS_FIRST;
  case ZP_AS_SPEEDZ:
    return AS_SPEEDZ;
  case ZP_AS_TRCFLG:
    return AS_TRCFLG;
  case ZP_AS_FLASH_BIT:
    return AS_FLASH_BIT;
  case ZP_AS_REMSTK:
    return AS_REMSTK;
  default:
    return general_memory_[address];
  }
}

void ApplesoftVariables::writeByte(std::uint16_t address, std::uint8_t value) {
  if (address == ADDR_AS_INPUT_BUFFER_MINUS_4) {
    AS_INPUT_BUFFER_MINUS_4 = value;
    return;
  }

  if (address == ADDR_AS_INPUT_BUFFER_MINUS_3) {
    AS_INPUT_BUFFER_MINUS_3 = value;
    return;
  }

  if (address == ADDR_AS_INPUT_BUFFER_MINUS_1) {
    AS_INPUT_BUFFER_MINUS_1 = value;
    return;
  }

  if (address >= ADDR_AS_INPUT_BUFFER &&
      address <= ADDR_AS_INPUT_BUFFER + 0xffu) {
    AS_INPUT_BUFFER_PAGE[address - ADDR_AS_INPUT_BUFFER] = value;
    return;
  }

  if (address >= IOPorts::ADDR_BASE && address < IOPorts::ADDR_END) {
    ioPorts().writeByte(address, value);
    return;
  }

  switch (address) {
  case ZP_AS_GOWARM:
    AS_GOWARM = value;
    return;
  case ZP_AS_GOWARM + 1:
    setLowByte(AS_GOWARM_TARGET, value);
    return;
  case ZP_AS_GOWARM + 2:
    setHighByte(AS_GOWARM_TARGET, value);
    return;
  case ZP_AS_GOSTROUT:
    AS_GOSTROUT = value;
    return;
  case ZP_AS_GOSTROUT + 1:
    setLowByte(AS_GOSTROUT_TARGET, value);
    return;
  case ZP_AS_GOSTROUT + 2:
    setHighByte(AS_GOSTROUT_TARGET, value);
    return;
  case ZP_AS_USR:
    AS_USR = value;
    return;
  case ZP_AS_USR + 1:
    setLowByte(AS_USR_TARGET, value);
    return;
  case ZP_AS_USR + 2:
    setHighByte(AS_USR_TARGET, value);
    return;
  case ZP_AS_CHARAC:
    AS_CHARAC = value;
    return;
  case ZP_AS_ENDCHR:
    AS_ENDCHR = value;
    return;
  case ZP_AS_NUMDIM:
    AS_NUMDIM = value;
    return;
  case ZP_AS_DIMFLG:
    AS_DIMFLG = value;
    return;
  case ZP_AS_VALTYP:
    AS_VALTYP = value;
    return;
  case ZP_AS_VALTYP_PLUS_1:
    AS_VALTYP_PLUS_1 = value;
    return;
  case ZP_AS_GARFLG:
    AS_GARFLG = value;
    return;
  case ZP_AS_SUBFLG:
    AS_SUBFLG = value;
    return;
  case ZP_AS_INPUTFLG:
    AS_INPUTFLG = value;
    return;
  case ZP_AS_CPRMASK:
    AS_CPRMASK = value;
    return;
  case ZP_MON_CH:
    MON_CH = value;
    return;
  case ZP_MON_CHKSUM:
    MON_CHKSUM = value;
    return;
  case ZP_MON_COLOR:
    MON_COLOR = value;
    return;
  case ZP_MON_PROMPT:
    MON_PROMPT = value;
    return;
  case ZP_MON_A1:
    setLowByte(MON_A1, value);
    return;
  case ZP_MON_A1 + 1:
    setHighByte(MON_A1, value);
    return;
  case ZP_MON_A2:
    setLowByte(MON_A2, value);
    return;
  case ZP_MON_A2 + 1:
    setHighByte(MON_A2, value);
    return;
  case ZP_AS_LINNUM:
    setLowByte(AS_LINNUM, value);
    return;
  case ZP_AS_LINNUM + 1:
    setHighByte(AS_LINNUM, value);
    return;
  case ZP_AS_TEMPPT:
    AS_TEMPPT = value;
    return;
  case ZP_AS_LASTPT:
    AS_LASTPT = value;
    return;
  case ZP_AS_INDEX:
    setLowByte(AS_INDEX, value);
    return;
  case ZP_AS_INDEX + 1:
    setHighByte(AS_INDEX, value);
    return;
  case ZP_AS_RESULT:
    AS_RESULT[0] = value;
    return;
  case ZP_AS_RESULT + 1:
    AS_RESULT[1] = value;
    return;
  case ZP_AS_RESULT + 2:
    AS_RESULT[2] = value;
    return;
  case ZP_AS_RESULT + 3:
    AS_RESULT[3] = value;
    return;
  case ZP_AS_TXTTAB:
    setLowByte(AS_TXTTAB, value);
    return;
  case ZP_AS_TXTTAB + 1:
    setHighByte(AS_TXTTAB, value);
    return;
  case ZP_AS_VARTAB:
    setLowByte(AS_VARTAB, value);
    return;
  case ZP_AS_VARTAB + 1:
    setHighByte(AS_VARTAB, value);
    return;
  case ZP_AS_ARYTAB:
    setLowByte(AS_ARYTAB, value);
    return;
  case ZP_AS_ARYTAB + 1:
    setHighByte(AS_ARYTAB, value);
    return;
  case ZP_AS_STREND:
    setLowByte(AS_STREND, value);
    return;
  case ZP_AS_STREND + 1:
    setHighByte(AS_STREND, value);
    return;
  case ZP_AS_FRETOP:
    setLowByte(AS_FRETOP, value);
    return;
  case ZP_AS_FRETOP + 1:
    setHighByte(AS_FRETOP, value);
    return;
  case ZP_AS_FRESPC:
    setLowByte(AS_FRESPC, value);
    return;
  case ZP_AS_FRESPC + 1:
    setHighByte(AS_FRESPC, value);
    return;
  case ZP_AS_MEMSIZ:
    setLowByte(AS_MEMSIZ, value);
    return;
  case ZP_AS_MEMSIZ + 1:
    setHighByte(AS_MEMSIZ, value);
    return;
  case ZP_AS_CURLIN:
    setLowByte(AS_CURLIN, value);
    return;
  case ZP_AS_CURLIN + 1:
    setHighByte(AS_CURLIN, value);
    return;
  case ZP_AS_OLDLIN:
    setLowByte(AS_OLDLIN, value);
    return;
  case ZP_AS_OLDLIN + 1:
    setHighByte(AS_OLDLIN, value);
    return;
  case ZP_AS_OLDTEXT:
    setLowByte(AS_OLDTEXT, value);
    return;
  case ZP_AS_OLDTEXT + 1:
    setHighByte(AS_OLDTEXT, value);
    return;
  case ZP_AS_DATLIN:
    setLowByte(AS_DATLIN, value);
    return;
  case ZP_AS_DATLIN + 1:
    setHighByte(AS_DATLIN, value);
    return;
  case ZP_AS_DATPTR:
    setLowByte(AS_DATPTR, value);
    return;
  case ZP_AS_DATPTR + 1:
    setHighByte(AS_DATPTR, value);
    return;
  case ZP_AS_INPTR:
    setLowByte(AS_INPTR, value);
    return;
  case ZP_AS_INPTR + 1:
    setHighByte(AS_INPTR, value);
    return;
  case ZP_AS_VARNAM:
    setLowByte(AS_VARNAM, value);
    return;
  case ZP_AS_VARNAM + 1:
    setHighByte(AS_VARNAM, value);
    return;
  case ZP_AS_VARPNT:
    setLowByte(AS_VARPNT, value);
    return;
  case ZP_AS_VARPNT + 1:
    setHighByte(AS_VARPNT, value);
    return;
  case ZP_AS_FORPNT:
    setLowByte(AS_FORPNT, value);
    return;
  case ZP_AS_FORPNT + 1:
    setHighByte(AS_FORPNT, value);
    return;
  case ZP_AS_TXPSV:
    setLowByte(AS_TXPSV, value);
    return;
  case ZP_AS_TXPSV + 1:
    setHighByte(AS_TXPSV, value);
    return;
  case ZP_AS_CPRTYP:
    AS_CPRTYP = value;
    return;
  case ZP_AS_FNCNAM:
    setLowByte(AS_FNCNAM, value);
    return;
  case ZP_AS_FNCNAM + 1:
    setHighByte(AS_FNCNAM, value);
    return;
  case ZP_AS_DSCPTR:
    setLowByte(AS_DSCPTR, value);
    return;
  case ZP_AS_DSCPTR + 1:
    setHighByte(AS_DSCPTR, value);
    return;
  case ZP_AS_DSCLEN:
    AS_DSCLEN = value;
    return;
  case ZP_AS_JMPADRS:
    AS_JMPADRS_OPCODE = value;
    return;
  case ZP_AS_JMPADRS + 1:
    setLowByte(AS_JMPADRS, value);
    return;
  case ZP_AS_JMPADRS + 2:
    setHighByte(AS_JMPADRS, value);
    return;
  case ZP_AS_ARYPNT:
    setLowByte(AS_ARYPNT, value);
    return;
  case ZP_AS_ARYPNT + 1:
    setHighByte(AS_ARYPNT, value);
    return;
  case ZP_AS_HIGHTR:
    setLowByte(AS_HIGHTR, value);
    return;
  case ZP_AS_HIGHTR + 1:
    setHighByte(AS_HIGHTR, value);
    return;
  case ZP_AS_INDX:
    AS_INDX = value;
    return;
  case ZP_AS_LOWTR:
    setLowByte(AS_LOWTR, value);
    return;
  case ZP_AS_LOWTR + 1:
    setHighByte(AS_LOWTR, value);
    return;
  case ZP_AS_FAC:
    AS_FAC[0] = value;
    return;
  case ZP_AS_FAC + 1:
    AS_FAC[1] = value;
    return;
  case ZP_AS_FAC + 2:
    AS_FAC[2] = value;
    return;
  case ZP_AS_FAC + 3:
    AS_FAC[3] = value;
    return;
  case ZP_AS_FAC + 4:
    AS_FAC[4] = value;
    return;
  case ZP_AS_FAC_SIGN:
    AS_FAC_SIGN = value;
    return;
  case ZP_AS_SHIFT_SIGN_EXT:
    AS_SHIFT_SIGN_EXT = value;
    return;
  case ZP_AS_ARG:
    AS_ARG[0] = value;
    return;
  case ZP_AS_ARG + 1:
    AS_ARG[1] = value;
    return;
  case ZP_AS_ARG + 2:
    AS_ARG[2] = value;
    return;
  case ZP_AS_ARG + 3:
    AS_ARG[3] = value;
    return;
  case ZP_AS_ARG + 4:
    AS_ARG[4] = value;
    return;
  case ZP_AS_ARG + 5:
    AS_ARG[5] = value;
    return;
  case ZP_AS_STRNG1:
    setLowByte(AS_STRNG1, value);
    return;
  case ZP_AS_STRNG1 + 1:
    setHighByte(AS_STRNG1, value);
    return;
  case ZP_AS_STRNG2:
    setLowByte(AS_STRNG2, value);
    return;
  case ZP_AS_STRNG2 + 1:
    setHighByte(AS_STRNG2, value);
    return;
  case ZP_AS_PRGEND:
    setLowByte(AS_PRGEND, value);
    return;
  case ZP_AS_PRGEND + 1:
    setHighByte(AS_PRGEND, value);
    return;
  case ZP_AS_TXTPTR:
    setLowByte(AS_TXTPTR, value);
    return;
  case ZP_AS_TXTPTR + 1:
    setHighByte(AS_TXTPTR, value);
    return;
  case ZP_AS_LOCK:
    AS_LOCK = value;
    return;
  case ZP_AS_ERRFLG:
    AS_ERRFLG = value;
    return;
  case ZP_AS_FIRST:
    AS_FIRST = value;
    return;
  case ZP_AS_SPEEDZ:
    AS_SPEEDZ = value;
    return;
  case ZP_AS_TRCFLG:
    AS_TRCFLG = value;
    return;
  case ZP_AS_FLASH_BIT:
    AS_FLASH_BIT = value;
    return;
  case ZP_AS_REMSTK:
    AS_REMSTK = value;
    return;
  default:
    general_memory_[address] = value;
    return;
  }
}

std::uint16_t ApplesoftVariables::readWord(std::uint16_t address) const {
  const auto ptr = pointer(address);
  const std::uint8_t low = ptr.read();
  const std::uint8_t high = ptr.read(1u);
  return makeWord(low, high);
}

void ApplesoftVariables::writeWord(std::uint16_t address, std::uint16_t value) {
  auto ptr = pointer(address);
  ptr.write(lowByte(value));
  ptr.write(highByte(value), 1u);
}

} // namespace applesoft::asm_port
