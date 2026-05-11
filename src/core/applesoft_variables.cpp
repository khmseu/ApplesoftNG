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
    if (address == 0x01ffu) {
        return INPUT_BUFFER_MINUS_1;
    }

    if (address >= 0x0200u && address <= 0x02ffu) {
        return INPUT_BUFFER_PAGE[address - 0x0200u];
    }

    if (address == 0xc000u) {
        return KEYBOARD;
    }

    switch (address) {
    case 0x000d: return CHARAC;
    case 0x000e: return ENDCHR;
    case 0x000f: return NUMDIM;
    case 0x0010: return DIMFLG;
    case 0x0011: return VALTYP;
    case 0x0012: return VALTYP_PLUS_1;
    case 0x0014: return SUBFLG;
    case 0x0015: return INPUTFLG;
    case 0x0016: return CPRMASK;
    case 0x0024: return MON_CH;
    case 0x0033: return MON_PROMPT;
    case 0x003c: return lowByte(MON_A1);
    case 0x003d: return highByte(MON_A1);
    case 0x003e: return lowByte(MON_A2);
    case 0x003f: return highByte(MON_A2);
    case 0x0050: return lowByte(LINNUM);
    case 0x0051: return highByte(LINNUM);
    case 0x0052: return TEMPPT;
    case 0x005e: return lowByte(INDEX);
    case 0x005f: return highByte(INDEX);
    case 0x0062: return RESULT[0];
    case 0x0063: return RESULT[1];
    case 0x0064: return RESULT[2];
    case 0x0065: return RESULT[3];
    case 0x0067: return lowByte(TXTTAB);
    case 0x0068: return highByte(TXTTAB);
    case 0x0069: return lowByte(VARTAB);
    case 0x006a: return highByte(VARTAB);
    case 0x006b: return lowByte(ARYTAB);
    case 0x006c: return highByte(ARYTAB);
    case 0x006d: return lowByte(STREND);
    case 0x006e: return highByte(STREND);
    case 0x006f: return lowByte(FRETOP);
    case 0x0070: return highByte(FRETOP);
    case 0x0073: return lowByte(MEMSIZ);
    case 0x0074: return highByte(MEMSIZ);
    case 0x0075: return lowByte(CURLIN);
    case 0x0076: return highByte(CURLIN);
    case 0x0077: return lowByte(OLDLIN);
    case 0x0078: return highByte(OLDLIN);
    case 0x0079: return lowByte(OLDTEXT);
    case 0x007a: return highByte(OLDTEXT);
    case 0x007b: return lowByte(DATLIN);
    case 0x007c: return highByte(DATLIN);
    case 0x007d: return lowByte(DATPTR);
    case 0x007e: return highByte(DATPTR);
    case 0x007f: return lowByte(INPTR);
    case 0x0080: return highByte(INPTR);
    case 0x0081: return lowByte(VARNAM);
    case 0x0082: return highByte(VARNAM);
    case 0x0083: return lowByte(VARPNT);
    case 0x0084: return highByte(VARPNT);
    case 0x0085: return lowByte(FORPNT);
    case 0x0086: return highByte(FORPNT);
    case 0x0087: return lowByte(TXPSV);
    case 0x0088: return highByte(TXPSV);
    case 0x0089: return CPRTYP;
    case 0x008c: return lowByte(DSCPTR);
    case 0x008d: return highByte(DSCPTR);
    case 0x0091: return lowByte(JMPADRS);
    case 0x0092: return highByte(JMPADRS);
    case 0x0094: return lowByte(ARYPNT);
    case 0x0095: return highByte(ARYPNT);
    case 0x0096: return lowByte(HIGHTR);
    case 0x0097: return highByte(HIGHTR);
    case 0x0099: return INDX;
    case 0x009b: return lowByte(LOWTR);
    case 0x009c: return highByte(LOWTR);
    case 0x009d: return FAC[0];
    case 0x009e: return FAC[1];
    case 0x009f: return FAC[2];
    case 0x00a0: return FAC[3];
    case 0x00a1: return FAC[4];
    case 0x00a2: return FAC_SIGN;
    case 0x00a5: return ARG[0];
    case 0x00a6: return ARG[1];
    case 0x00a7: return ARG[2];
    case 0x00a8: return ARG[3];
    case 0x00a9: return ARG[4];
    case 0x00aa: return ARG[5];
    case 0x00ab: return lowByte(STRNG1);
    case 0x00ac: return highByte(STRNG1);
    case 0x00ad: return lowByte(STRNG2);
    case 0x00ae: return highByte(STRNG2);
    case 0x00af: return lowByte(PRGEND);
    case 0x00b0: return highByte(PRGEND);
    case 0x00b8: return lowByte(TXTPTR);
    case 0x00b9: return highByte(TXTPTR);
    case 0x00d6: return LOCK;
    case 0x00d8: return ERRFLG;
    case 0x00f0: return FIRST;
    case 0x00f1: return SPEEDZ;
    case 0x00f2: return TRCFLG;
    case 0x00f3: return FLASH_BIT;
    case 0x00f8: return REMSTK;
    default: return 0;
    }
}

void ApplesoftVariables::writeByte(std::uint16_t address, std::uint8_t value) {
    if (address == 0x01ffu) {
        INPUT_BUFFER_MINUS_1 = value;
        return;
    }

    if (address >= 0x0200u && address <= 0x02ffu) {
        INPUT_BUFFER_PAGE[address - 0x0200u] = value;
        return;
    }

    if (address == 0xc000u) {
        KEYBOARD = value;
        return;
    }

    switch (address) {
    case 0x000d: CHARAC = value; return;
    case 0x000e: ENDCHR = value; return;
    case 0x000f: NUMDIM = value; return;
    case 0x0010: DIMFLG = value; return;
    case 0x0011: VALTYP = value; return;
    case 0x0012: VALTYP_PLUS_1 = value; return;
    case 0x0014: SUBFLG = value; return;
    case 0x0015: INPUTFLG = value; return;
    case 0x0016: CPRMASK = value; return;
    case 0x0024: MON_CH = value; return;
    case 0x0033: MON_PROMPT = value; return;
    case 0x003c: setLowByte(MON_A1, value); return;
    case 0x003d: setHighByte(MON_A1, value); return;
    case 0x003e: setLowByte(MON_A2, value); return;
    case 0x003f: setHighByte(MON_A2, value); return;
    case 0x0050: setLowByte(LINNUM, value); return;
    case 0x0051: setHighByte(LINNUM, value); return;
    case 0x0052: TEMPPT = value; return;
    case 0x005e: setLowByte(INDEX, value); return;
    case 0x005f: setHighByte(INDEX, value); return;
    case 0x0062: RESULT[0] = value; return;
    case 0x0063: RESULT[1] = value; return;
    case 0x0064: RESULT[2] = value; return;
    case 0x0065: RESULT[3] = value; return;
    case 0x0067: setLowByte(TXTTAB, value); return;
    case 0x0068: setHighByte(TXTTAB, value); return;
    case 0x0069: setLowByte(VARTAB, value); return;
    case 0x006a: setHighByte(VARTAB, value); return;
    case 0x006b: setLowByte(ARYTAB, value); return;
    case 0x006c: setHighByte(ARYTAB, value); return;
    case 0x006d: setLowByte(STREND, value); return;
    case 0x006e: setHighByte(STREND, value); return;
    case 0x006f: setLowByte(FRETOP, value); return;
    case 0x0070: setHighByte(FRETOP, value); return;
    case 0x0073: setLowByte(MEMSIZ, value); return;
    case 0x0074: setHighByte(MEMSIZ, value); return;
    case 0x0075: setLowByte(CURLIN, value); return;
    case 0x0076: setHighByte(CURLIN, value); return;
    case 0x0077: setLowByte(OLDLIN, value); return;
    case 0x0078: setHighByte(OLDLIN, value); return;
    case 0x0079: setLowByte(OLDTEXT, value); return;
    case 0x007a: setHighByte(OLDTEXT, value); return;
    case 0x007b: setLowByte(DATLIN, value); return;
    case 0x007c: setHighByte(DATLIN, value); return;
    case 0x007d: setLowByte(DATPTR, value); return;
    case 0x007e: setHighByte(DATPTR, value); return;
    case 0x007f: setLowByte(INPTR, value); return;
    case 0x0080: setHighByte(INPTR, value); return;
    case 0x0081: setLowByte(VARNAM, value); return;
    case 0x0082: setHighByte(VARNAM, value); return;
    case 0x0083: setLowByte(VARPNT, value); return;
    case 0x0084: setHighByte(VARPNT, value); return;
    case 0x0085: setLowByte(FORPNT, value); return;
    case 0x0086: setHighByte(FORPNT, value); return;
    case 0x0087: setLowByte(TXPSV, value); return;
    case 0x0088: setHighByte(TXPSV, value); return;
    case 0x0089: CPRTYP = value; return;
    case 0x008c: setLowByte(DSCPTR, value); return;
    case 0x008d: setHighByte(DSCPTR, value); return;
    case 0x0091: setLowByte(JMPADRS, value); return;
    case 0x0092: setHighByte(JMPADRS, value); return;
    case 0x0094: setLowByte(ARYPNT, value); return;
    case 0x0095: setHighByte(ARYPNT, value); return;
    case 0x0096: setLowByte(HIGHTR, value); return;
    case 0x0097: setHighByte(HIGHTR, value); return;
    case 0x0099: INDX = value; return;
    case 0x009b: setLowByte(LOWTR, value); return;
    case 0x009c: setHighByte(LOWTR, value); return;
    case 0x009d: FAC[0] = value; return;
    case 0x009e: FAC[1] = value; return;
    case 0x009f: FAC[2] = value; return;
    case 0x00a0: FAC[3] = value; return;
    case 0x00a1: FAC[4] = value; return;
    case 0x00a2: FAC_SIGN = value; return;
    case 0x00a5: ARG[0] = value; return;
    case 0x00a6: ARG[1] = value; return;
    case 0x00a7: ARG[2] = value; return;
    case 0x00a8: ARG[3] = value; return;
    case 0x00a9: ARG[4] = value; return;
    case 0x00aa: ARG[5] = value; return;
    case 0x00ab: setLowByte(STRNG1, value); return;
    case 0x00ac: setHighByte(STRNG1, value); return;
    case 0x00ad: setLowByte(STRNG2, value); return;
    case 0x00ae: setHighByte(STRNG2, value); return;
    case 0x00af: setLowByte(PRGEND, value); return;
    case 0x00b0: setHighByte(PRGEND, value); return;
    case 0x00b8: setLowByte(TXTPTR, value); return;
    case 0x00b9: setHighByte(TXTPTR, value); return;
    case 0x00d6: LOCK = value; return;
    case 0x00d8: ERRFLG = value; return;
    case 0x00f0: FIRST = value; return;
    case 0x00f1: SPEEDZ = value; return;
    case 0x00f2: TRCFLG = value; return;
    case 0x00f3: FLASH_BIT = value; return;
    case 0x00f8: REMSTK = value; return;
    default: return;
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
