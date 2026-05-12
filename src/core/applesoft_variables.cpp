#include "core/applesoft_variables.hpp"

namespace applesoft::asm_port {

namespace {

ApplesoftVariables g_variables;

} // namespace

ApplesoftVariables& variables() {
    return g_variables;
}

const ApplesoftVariables& variables_const() {
    return g_variables;
}

std::uint8_t ApplesoftVariables::lowByte(std::uint16_t value) {
    return static_cast<std::uint8_t>(value & 0xffu);
}

std::uint8_t ApplesoftVariables::highByte(std::uint16_t value) {
    return static_cast<std::uint8_t>((value >> 8) & 0xffu);
}

std::uint16_t ApplesoftVariables::makeWord(std::uint8_t low, std::uint8_t high) {
    return static_cast<std::uint16_t>(static_cast<std::uint16_t>(high) << 8 | low);
}

void ApplesoftVariables::setLowByte(std::uint16_t& target, std::uint8_t value) {
    target = static_cast<std::uint16_t>((target & 0xff00u) | value);
}

void ApplesoftVariables::setHighByte(std::uint16_t& target, std::uint8_t value) {
    target = static_cast<std::uint16_t>((target & 0x00ffu) | (static_cast<std::uint16_t>(value) << 8));
}

std::uint8_t ApplesoftVariables::readByte(std::uint16_t address) const {
    if (address == ADDR_INPUT_BUFFER_MINUS_4) {
        return INPUT_BUFFER_MINUS_4;
    }

    if (address == ADDR_INPUT_BUFFER_MINUS_3) {
        return INPUT_BUFFER_MINUS_3;
    }

    if (address == ADDR_INPUT_BUFFER_MINUS_1) {
        return INPUT_BUFFER_MINUS_1;
    }

    if (address >= ADDR_INPUT_BUFFER && address <= ADDR_INPUT_BUFFER + 0xffu) {
        return INPUT_BUFFER_PAGE[address - ADDR_INPUT_BUFFER];
    }

    if (address == ADDR_KEYBOARD) {
        return KEYBOARD;
    }

    switch (address) {
    case ZP_GOWARM: return GOWARM;
    case ZP_GOWARM + 1: return lowByte(GOWARM_TARGET);
    case ZP_GOWARM + 2: return highByte(GOWARM_TARGET);
    case ZP_GOSTROUT: return GOSTROUT;
    case ZP_GOSTROUT + 1: return lowByte(GOSTROUT_TARGET);
    case ZP_GOSTROUT + 2: return highByte(GOSTROUT_TARGET);
    case ZP_USR: return USR;
    case ZP_USR + 1: return lowByte(USR_TARGET);
    case ZP_USR + 2: return highByte(USR_TARGET);
    case ZP_CHARAC: return CHARAC;
    case ZP_ENDCHR: return ENDCHR;
    case ZP_NUMDIM: return NUMDIM;
    case ZP_DIMFLG: return DIMFLG;
    case ZP_VALTYP: return VALTYP;
    case ZP_VALTYP_PLUS_1: return VALTYP_PLUS_1;
    case ZP_GARFLG: return GARFLG;
    case ZP_SUBFLG: return SUBFLG;
    case ZP_INPUTFLG: return INPUTFLG;
    case ZP_CPRMASK: return CPRMASK;
    case ZP_MON_CH: return MON_CH;
    case ZP_MON_PROMPT: return MON_PROMPT;
    case ZP_MON_A1: return lowByte(MON_A1);
    case ZP_MON_A1 + 1: return highByte(MON_A1);
    case ZP_MON_A2: return lowByte(MON_A2);
    case ZP_MON_A2 + 1: return highByte(MON_A2);
    case ZP_LINNUM: return lowByte(LINNUM);
    case ZP_LINNUM + 1: return highByte(LINNUM);
    case ZP_TEMPPT: return TEMPPT;
    case ZP_LASTPT: return LASTPT;
    case ZP_INDEX: return lowByte(INDEX);
    case ZP_INDEX + 1: return highByte(INDEX);
    case ZP_RESULT: return RESULT[0];
    case ZP_RESULT + 1: return RESULT[1];
    case ZP_RESULT + 2: return RESULT[2];
    case ZP_RESULT + 3: return RESULT[3];
    case ZP_TXTTAB: return lowByte(TXTTAB);
    case ZP_TXTTAB + 1: return highByte(TXTTAB);
    case ZP_VARTAB: return lowByte(VARTAB);
    case ZP_VARTAB + 1: return highByte(VARTAB);
    case ZP_ARYTAB: return lowByte(ARYTAB);
    case ZP_ARYTAB + 1: return highByte(ARYTAB);
    case ZP_STREND: return lowByte(STREND);
    case ZP_STREND + 1: return highByte(STREND);
    case ZP_FRETOP: return lowByte(FRETOP);
    case ZP_FRETOP + 1: return highByte(FRETOP);
    case ZP_FRESPC: return lowByte(FRESPC);
    case ZP_FRESPC + 1: return highByte(FRESPC);
    case ZP_MEMSIZ: return lowByte(MEMSIZ);
    case ZP_MEMSIZ + 1: return highByte(MEMSIZ);
    case ZP_CURLIN: return lowByte(CURLIN);
    case ZP_CURLIN + 1: return highByte(CURLIN);
    case ZP_OLDLIN: return lowByte(OLDLIN);
    case ZP_OLDLIN + 1: return highByte(OLDLIN);
    case ZP_OLDTEXT: return lowByte(OLDTEXT);
    case ZP_OLDTEXT + 1: return highByte(OLDTEXT);
    case ZP_DATLIN: return lowByte(DATLIN);
    case ZP_DATLIN + 1: return highByte(DATLIN);
    case ZP_DATPTR: return lowByte(DATPTR);
    case ZP_DATPTR + 1: return highByte(DATPTR);
    case ZP_INPTR: return lowByte(INPTR);
    case ZP_INPTR + 1: return highByte(INPTR);
    case ZP_VARNAM: return lowByte(VARNAM);
    case ZP_VARNAM + 1: return highByte(VARNAM);
    case ZP_VARPNT: return lowByte(VARPNT);
    case ZP_VARPNT + 1: return highByte(VARPNT);
    case ZP_FORPNT: return lowByte(FORPNT);
    case ZP_FORPNT + 1: return highByte(FORPNT);
    case ZP_TXPSV: return lowByte(TXPSV);
    case ZP_TXPSV + 1: return highByte(TXPSV);
    case ZP_CPRTYP: return CPRTYP;
    case ZP_FNCNAM: return lowByte(FNCNAM);
    case ZP_FNCNAM + 1: return highByte(FNCNAM);
    case ZP_DSCPTR: return lowByte(DSCPTR);
    case ZP_DSCPTR + 1: return highByte(DSCPTR);
    case ZP_DSCLEN: return DSCLEN;
    case ZP_JMPADRS: return JMPADRS_OPCODE;
    case ZP_JMPADRS + 1: return lowByte(JMPADRS);
    case ZP_JMPADRS + 2: return highByte(JMPADRS);
    case ZP_ARYPNT: return lowByte(ARYPNT);
    case ZP_ARYPNT + 1: return highByte(ARYPNT);
    case ZP_HIGHTR: return lowByte(HIGHTR);
    case ZP_HIGHTR + 1: return highByte(HIGHTR);
    case ZP_INDX: return INDX;
    case ZP_LOWTR: return lowByte(LOWTR);
    case ZP_LOWTR + 1: return highByte(LOWTR);
    case ZP_FAC: return FAC[0];
    case ZP_FAC + 1: return FAC[1];
    case ZP_FAC + 2: return FAC[2];
    case ZP_FAC + 3: return FAC[3];
    case ZP_FAC + 4: return FAC[4];
    case ZP_FAC_SIGN: return FAC_SIGN;
    case ZP_SHIFT_SIGN_EXT: return SHIFT_SIGN_EXT;
    case ZP_ARG: return ARG[0];
    case ZP_ARG + 1: return ARG[1];
    case ZP_ARG + 2: return ARG[2];
    case ZP_ARG + 3: return ARG[3];
    case ZP_ARG + 4: return ARG[4];
    case ZP_ARG + 5: return ARG[5];
    case ZP_STRNG1: return lowByte(STRNG1);
    case ZP_STRNG1 + 1: return highByte(STRNG1);
    case ZP_STRNG2: return lowByte(STRNG2);
    case ZP_STRNG2 + 1: return highByte(STRNG2);
    case ZP_PRGEND: return lowByte(PRGEND);
    case ZP_PRGEND + 1: return highByte(PRGEND);
    case ZP_TXTPTR: return lowByte(TXTPTR);
    case ZP_TXTPTR + 1: return highByte(TXTPTR);
    case ZP_LOCK: return LOCK;
    case ZP_ERRFLG: return ERRFLG;
    case ZP_FIRST: return FIRST;
    case ZP_SPEEDZ: return SPEEDZ;
    case ZP_TRCFLG: return TRCFLG;
    case ZP_FLASH_BIT: return FLASH_BIT;
    case ZP_REMSTK: return REMSTK;
    default: return general_memory_[address];
    }
}

void ApplesoftVariables::writeByte(std::uint16_t address, std::uint8_t value) {
    if (address == ADDR_INPUT_BUFFER_MINUS_4) {
        INPUT_BUFFER_MINUS_4 = value;
        return;
    }

    if (address == ADDR_INPUT_BUFFER_MINUS_3) {
        INPUT_BUFFER_MINUS_3 = value;
        return;
    }

    if (address == ADDR_INPUT_BUFFER_MINUS_1) {
        INPUT_BUFFER_MINUS_1 = value;
        return;
    }

    if (address >= ADDR_INPUT_BUFFER && address <= ADDR_INPUT_BUFFER + 0xffu) {
        INPUT_BUFFER_PAGE[address - ADDR_INPUT_BUFFER] = value;
        return;
    }

    if (address == ADDR_KEYBOARD) {
        KEYBOARD = value;
        return;
    }

    switch (address) {
    case ZP_GOWARM: GOWARM = value; return;
    case ZP_GOWARM + 1: setLowByte(GOWARM_TARGET, value); return;
    case ZP_GOWARM + 2: setHighByte(GOWARM_TARGET, value); return;
    case ZP_GOSTROUT: GOSTROUT = value; return;
    case ZP_GOSTROUT + 1: setLowByte(GOSTROUT_TARGET, value); return;
    case ZP_GOSTROUT + 2: setHighByte(GOSTROUT_TARGET, value); return;
    case ZP_USR: USR = value; return;
    case ZP_USR + 1: setLowByte(USR_TARGET, value); return;
    case ZP_USR + 2: setHighByte(USR_TARGET, value); return;
    case ZP_CHARAC: CHARAC = value; return;
    case ZP_ENDCHR: ENDCHR = value; return;
    case ZP_NUMDIM: NUMDIM = value; return;
    case ZP_DIMFLG: DIMFLG = value; return;
    case ZP_VALTYP: VALTYP = value; return;
    case ZP_VALTYP_PLUS_1: VALTYP_PLUS_1 = value; return;
    case ZP_GARFLG: GARFLG = value; return;
    case ZP_SUBFLG: SUBFLG = value; return;
    case ZP_INPUTFLG: INPUTFLG = value; return;
    case ZP_CPRMASK: CPRMASK = value; return;
    case ZP_MON_CH: MON_CH = value; return;
    case ZP_MON_PROMPT: MON_PROMPT = value; return;
    case ZP_MON_A1: setLowByte(MON_A1, value); return;
    case ZP_MON_A1 + 1: setHighByte(MON_A1, value); return;
    case ZP_MON_A2: setLowByte(MON_A2, value); return;
    case ZP_MON_A2 + 1: setHighByte(MON_A2, value); return;
    case ZP_LINNUM: setLowByte(LINNUM, value); return;
    case ZP_LINNUM + 1: setHighByte(LINNUM, value); return;
    case ZP_TEMPPT: TEMPPT = value; return;
    case ZP_LASTPT: LASTPT = value; return;
    case ZP_INDEX: setLowByte(INDEX, value); return;
    case ZP_INDEX + 1: setHighByte(INDEX, value); return;
    case ZP_RESULT: RESULT[0] = value; return;
    case ZP_RESULT + 1: RESULT[1] = value; return;
    case ZP_RESULT + 2: RESULT[2] = value; return;
    case ZP_RESULT + 3: RESULT[3] = value; return;
    case ZP_TXTTAB: setLowByte(TXTTAB, value); return;
    case ZP_TXTTAB + 1: setHighByte(TXTTAB, value); return;
    case ZP_VARTAB: setLowByte(VARTAB, value); return;
    case ZP_VARTAB + 1: setHighByte(VARTAB, value); return;
    case ZP_ARYTAB: setLowByte(ARYTAB, value); return;
    case ZP_ARYTAB + 1: setHighByte(ARYTAB, value); return;
    case ZP_STREND: setLowByte(STREND, value); return;
    case ZP_STREND + 1: setHighByte(STREND, value); return;
    case ZP_FRETOP: setLowByte(FRETOP, value); return;
    case ZP_FRETOP + 1: setHighByte(FRETOP, value); return;
    case ZP_FRESPC: setLowByte(FRESPC, value); return;
    case ZP_FRESPC + 1: setHighByte(FRESPC, value); return;
    case ZP_MEMSIZ: setLowByte(MEMSIZ, value); return;
    case ZP_MEMSIZ + 1: setHighByte(MEMSIZ, value); return;
    case ZP_CURLIN: setLowByte(CURLIN, value); return;
    case ZP_CURLIN + 1: setHighByte(CURLIN, value); return;
    case ZP_OLDLIN: setLowByte(OLDLIN, value); return;
    case ZP_OLDLIN + 1: setHighByte(OLDLIN, value); return;
    case ZP_OLDTEXT: setLowByte(OLDTEXT, value); return;
    case ZP_OLDTEXT + 1: setHighByte(OLDTEXT, value); return;
    case ZP_DATLIN: setLowByte(DATLIN, value); return;
    case ZP_DATLIN + 1: setHighByte(DATLIN, value); return;
    case ZP_DATPTR: setLowByte(DATPTR, value); return;
    case ZP_DATPTR + 1: setHighByte(DATPTR, value); return;
    case ZP_INPTR: setLowByte(INPTR, value); return;
    case ZP_INPTR + 1: setHighByte(INPTR, value); return;
    case ZP_VARNAM: setLowByte(VARNAM, value); return;
    case ZP_VARNAM + 1: setHighByte(VARNAM, value); return;
    case ZP_VARPNT: setLowByte(VARPNT, value); return;
    case ZP_VARPNT + 1: setHighByte(VARPNT, value); return;
    case ZP_FORPNT: setLowByte(FORPNT, value); return;
    case ZP_FORPNT + 1: setHighByte(FORPNT, value); return;
    case ZP_TXPSV: setLowByte(TXPSV, value); return;
    case ZP_TXPSV + 1: setHighByte(TXPSV, value); return;
    case ZP_CPRTYP: CPRTYP = value; return;
    case ZP_FNCNAM: setLowByte(FNCNAM, value); return;
    case ZP_FNCNAM + 1: setHighByte(FNCNAM, value); return;
    case ZP_DSCPTR: setLowByte(DSCPTR, value); return;
    case ZP_DSCPTR + 1: setHighByte(DSCPTR, value); return;
    case ZP_DSCLEN: DSCLEN = value; return;
    case ZP_JMPADRS: JMPADRS_OPCODE = value; return;
    case ZP_JMPADRS + 1: setLowByte(JMPADRS, value); return;
    case ZP_JMPADRS + 2: setHighByte(JMPADRS, value); return;
    case ZP_ARYPNT: setLowByte(ARYPNT, value); return;
    case ZP_ARYPNT + 1: setHighByte(ARYPNT, value); return;
    case ZP_HIGHTR: setLowByte(HIGHTR, value); return;
    case ZP_HIGHTR + 1: setHighByte(HIGHTR, value); return;
    case ZP_INDX: INDX = value; return;
    case ZP_LOWTR: setLowByte(LOWTR, value); return;
    case ZP_LOWTR + 1: setHighByte(LOWTR, value); return;
    case ZP_FAC: FAC[0] = value; return;
    case ZP_FAC + 1: FAC[1] = value; return;
    case ZP_FAC + 2: FAC[2] = value; return;
    case ZP_FAC + 3: FAC[3] = value; return;
    case ZP_FAC + 4: FAC[4] = value; return;
    case ZP_FAC_SIGN: FAC_SIGN = value; return;
    case ZP_SHIFT_SIGN_EXT: SHIFT_SIGN_EXT = value; return;
    case ZP_ARG: ARG[0] = value; return;
    case ZP_ARG + 1: ARG[1] = value; return;
    case ZP_ARG + 2: ARG[2] = value; return;
    case ZP_ARG + 3: ARG[3] = value; return;
    case ZP_ARG + 4: ARG[4] = value; return;
    case ZP_ARG + 5: ARG[5] = value; return;
    case ZP_STRNG1: setLowByte(STRNG1, value); return;
    case ZP_STRNG1 + 1: setHighByte(STRNG1, value); return;
    case ZP_STRNG2: setLowByte(STRNG2, value); return;
    case ZP_STRNG2 + 1: setHighByte(STRNG2, value); return;
    case ZP_PRGEND: setLowByte(PRGEND, value); return;
    case ZP_PRGEND + 1: setHighByte(PRGEND, value); return;
    case ZP_TXTPTR: setLowByte(TXTPTR, value); return;
    case ZP_TXTPTR + 1: setHighByte(TXTPTR, value); return;
    case ZP_LOCK: LOCK = value; return;
    case ZP_ERRFLG: ERRFLG = value; return;
    case ZP_FIRST: FIRST = value; return;
    case ZP_SPEEDZ: SPEEDZ = value; return;
    case ZP_TRCFLG: TRCFLG = value; return;
    case ZP_FLASH_BIT: FLASH_BIT = value; return;
    case ZP_REMSTK: REMSTK = value; return;
    default: general_memory_[address] = value; return;
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
