#include "core/asm_port_strlt2.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"

#include <algorithm>
#include <cstdint>

namespace applesoft::asm_port {

void CHKSTR();
void GARBAG();
void STRINI(std::uint8_t length);
void STRSPA(std::uint8_t length);
void MOVINS();
std::uint8_t FRETMP(std::uint16_t descriptorAddress);
bool FRETMS(std::uint16_t descriptorAddress);
std::uint8_t GETBYT();
void IQERR();
void CHKCLS();
void CONINT();
void SNGFLT(std::uint8_t value);

namespace {

std::uint8_t read_CHARAC() {
    return variables_const().readByte(ApplesoftVariables::ZP_CHARAC);
}

std::uint8_t read_ENDCHR() {
    return variables_const().readByte(ApplesoftVariables::ZP_ENDCHR);
}

void write_STRNG1(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_STRNG1, value);
}

void write_FAC(std::uint8_t v) {
    variables().writeByte(ApplesoftVariables::ZP_FAC, v);
}

void write_FAC_pointer(std::uint16_t value) {
    variables().writeWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 1u), value);
}

void write_STRNG2(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_STRNG2, value);
}

std::uint8_t read_TEMPPT() {
    return variables_const().readByte(ApplesoftVariables::ZP_TEMPPT);
}

void write_TEMPPT(std::uint8_t value) {
    variables().writeByte(ApplesoftVariables::ZP_TEMPPT, value);
}

void write_LASTPT(std::uint8_t value) {
    variables().writeByte(ApplesoftVariables::ZP_LASTPT, value);
}

std::uint8_t read_GARFLG() {
    return variables_const().readByte(ApplesoftVariables::ZP_GARFLG);
}

void write_GARFLG(std::uint8_t value) {
    variables().writeByte(ApplesoftVariables::ZP_GARFLG, value);
}

std::uint16_t read_FRETOP() {
    return variables_const().readWord(ApplesoftVariables::ZP_FRETOP);
}

void write_FRETOP(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_FRETOP, value);
}

void write_FRESPC(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_FRESPC, value);
}

std::uint16_t read_MEMSIZ() {
    return variables_const().readWord(ApplesoftVariables::ZP_MEMSIZ);
}

std::uint16_t read_STREND() {
    return variables_const().readWord(ApplesoftVariables::ZP_STREND);
}

std::uint16_t read_VARTAB() {
    return variables_const().readWord(ApplesoftVariables::ZP_VARTAB);
}

std::uint16_t read_ARYTAB() {
    return variables_const().readWord(ApplesoftVariables::ZP_ARYTAB);
}

std::uint16_t read_INDEX() {
    return variables_const().readWord(ApplesoftVariables::ZP_INDEX);
}

void write_INDEX(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_INDEX, value);
}

std::uint16_t read_LOWTR() {
    return variables_const().readWord(ApplesoftVariables::ZP_LOWTR);
}

void write_LOWTR(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_LOWTR, value);
}

std::uint16_t read_FNCNAM() {
    return variables_const().readWord(ApplesoftVariables::ZP_FNCNAM);
}

void write_FNCNAM(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_FNCNAM, value);
}

void clear_FNCNAM_hi() {
    variables().writeByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FNCNAM + 1u), 0u);
}

std::uint8_t read_DSCLEN() {
    return variables_const().readByte(ApplesoftVariables::ZP_DSCLEN);
}

void write_DSCLEN(std::uint8_t value) {
    variables().writeByte(ApplesoftVariables::ZP_DSCLEN, value);
}

std::uint8_t read_LENGTH() {
    return variables_const().readByte(ApplesoftVariables::ZP_LENGTH);
}

void write_LENGTH(std::uint8_t value) {
    // Dual-use storage: LENGTH aliases JMPADRS low-byte in ROM.
    variables().writeByte(ApplesoftVariables::ZP_LENGTH, value);
}

std::uint16_t read_ARYPNT() {
    return variables_const().readWord(ApplesoftVariables::ZP_ARYPNT);
}

void write_ARYPNT(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_ARYPNT, value);
}

void write_HIGHTR(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_HIGHTR, value);
}

void write_DSCPTR(std::uint16_t value) {
    variables().writeWord(ApplesoftVariables::ZP_DSCPTR, value);
}

std::uint16_t read_DSCPTR() {
    return variables_const().readWord(ApplesoftVariables::ZP_DSCPTR);
}

std::uint16_t read_STRNG1() {
    return variables_const().readWord(ApplesoftVariables::ZP_STRNG1);
}

std::uint16_t read_FRESPC() {
    return variables_const().readWord(ApplesoftVariables::ZP_FRESPC);
}

std::uint16_t read_FAC_descriptor_address() {
    return variables_const().readWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 3u));
}

void write_FAC_descriptor_address(std::uint16_t value) {
    variables().writeWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 3u), value);
}

std::uint8_t g_jerr_error = ERR_FRMCPX;

void JERR();
void PUTEMP(std::uint8_t tempDescriptorAddress);
std::uint16_t GETSPA(std::uint8_t length);
void FIND_HIGHEST_STRING();
void CHECK_SIMPLE_VARIABLE();
void CHECK_VARIABLE(std::uint8_t descriptorOffset);
void CHECK_BUMP();
void CHECK_EXIT();
void MOVE_HIGHEST_STRING_TO_TOP();
void PUTNEW();
void CAT();
void MOVSTR(std::uint8_t x, std::uint8_t y, std::uint8_t length);
void MOVSTR_1(std::uint8_t length);
std::uint8_t FRESTR();
std::uint8_t FREFAC();

// TODO(asm-port): port FRM_ELEMENT label.
void FRM_ELEMENT() {}

// TODO(asm-port): port FRMEVL_2 label.
void FRMEVL_2() {}

struct SubstringSetupResult {
    std::uint8_t firstParameter;
    std::uint8_t sourceLength;
};

bool SUBSTRING_SETUP(SubstringSetupResult& out) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SUBSTRING_SETUP (inclusive) .. LEN (exclusive)
    // Name normalization: none (assembler label SUBSTRING_SETUP kept verbatim).
    CHKCLS();

    const std::uint8_t first = variables_const().readByte(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 4u));
    if (first == 0u) {
        IQERR();
        return false;
    }

    const std::uint16_t descriptor = read_DSCPTR();
    const auto descriptorPtr = variables_const().pointer(descriptor);
    const std::uint8_t sourceLength = descriptorPtr.read(0u);
    const std::uint16_t source = ApplesoftVariables::makeWord(descriptorPtr.read(1u), descriptorPtr.read(2u));
    write_INDEX(source);

    out.firstParameter = first;
    out.sourceLength = sourceLength;
    return true;
}

void SUBSTRING_BUILD(std::uint8_t leftStart, std::uint8_t copyLength) {
    STRSPA(copyLength);
    (void)FRETMP(read_DSCPTR());

    const std::uint16_t source = read_INDEX();
    write_INDEX(static_cast<std::uint16_t>(source + leftStart));
    MOVSTR_1(copyLength);
    PUTNEW();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GETSPA (inclusive) .. GARBAG (exclusive)
// Name normalization: none (assembler label GETSPA kept verbatim).
std::uint16_t GETSPA(std::uint8_t length) {
    // lsr GARFLG: clear sign bit while preserving lower bits.
    write_GARFLG(static_cast<std::uint8_t>(read_GARFLG() >> 1u));

    while (true) {
        // Pointer candidate lifted: FRETOP/FRETOP+1 and STREND/STREND+1 are unified words.
        const std::uint16_t startAddress = static_cast<std::uint16_t>(read_FRETOP() - length);
        const std::uint16_t strend = read_STREND();

        if (startAddress >= strend) {
            write_FRETOP(startAddress);
            write_FRESPC(startAddress);
            return startAddress;
        }

        g_jerr_error = ERR_MEMFULL;
        if ((read_GARFLG() & 0x80u) != 0u) {
            JERR();
            return startAddress;
        }

        // Original control flow falls through GETSPA error path into GARBAG.
        GARBAG();
        write_GARFLG(0x80u);
    }
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: FIND_HIGHEST_STRING (inclusive) .. CHECK_SIMPLE_VARIABLE (exclusive)
// Name normalization: none (assembler label FIND_HIGHEST_STRING kept verbatim).
void FIND_HIGHEST_STRING() {
    clear_FNCNAM_hi();
    write_LOWTR(read_STREND());

    // Scan temporary descriptors.
    write_INDEX(ApplesoftVariables::ZP_TEMPST);
    while (read_INDEX() < read_TEMPPT()) {
        CHECK_VARIABLE(0u);
    }

    // Scan simple variables.
    write_DSCLEN(7u);
    write_INDEX(read_VARTAB());
    while (read_INDEX() < read_ARYTAB()) {
        CHECK_SIMPLE_VARIABLE();
    }

    // Scan arrays for string descriptors.
    write_ARYPNT(read_ARYTAB());
    write_DSCLEN(3u);
    while (read_ARYPNT() < read_STREND()) {
        const std::uint16_t arrayDescriptor = read_ARYPNT();
        write_INDEX(arrayDescriptor);

        const auto descriptor = variables_const().pointer(arrayDescriptor);
        const std::uint8_t firstChar = descriptor.read(0u);
        const std::uint8_t secondChar = descriptor.read(1u);
        const std::uint16_t nextArray = static_cast<std::uint16_t>(
            arrayDescriptor + ApplesoftVariables::makeWord(descriptor.read(2u), descriptor.read(3u)));
        write_ARYPNT(nextArray);

        const bool isStringArray = (secondChar & 0x80u) != 0u && (firstChar & 0x80u) == 0u;
        if (!isStringArray) {
            continue;
        }

        const std::uint8_t numDims = descriptor.read(4u);
        const std::uint16_t firstElement = static_cast<std::uint16_t>(
            arrayDescriptor + static_cast<std::uint16_t>(numDims * 2u + 5u));
        write_INDEX(firstElement);

        while (read_INDEX() < read_ARYPNT()) {
            CHECK_VARIABLE(0u);
        }
    }

    // Control transfers to MOVE_HIGHEST_STRING_TO_TOP in ROM.
    MOVE_HIGHEST_STRING_TO_TOP();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHECK_SIMPLE_VARIABLE (inclusive) .. CHECK_VARIABLE (exclusive)
// Name normalization: none (assembler label CHECK_SIMPLE_VARIABLE kept verbatim).
void CHECK_SIMPLE_VARIABLE() {
    const auto descriptor = variables_const().pointer(read_INDEX());
    const std::uint8_t firstChar = descriptor.read(0u);
    if ((firstChar & 0x80u) != 0u) {
        CHECK_BUMP();
        return;
    }

    const std::uint8_t secondChar = descriptor.read(1u);
    if ((secondChar & 0x80u) == 0u) {
        CHECK_BUMP();
        return;
    }

    CHECK_VARIABLE(2u);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHECK_VARIABLE (inclusive) .. CHECK_BUMP (exclusive)
// Name normalization: none (assembler label CHECK_VARIABLE kept verbatim).
void CHECK_VARIABLE(std::uint8_t descriptorOffset) {
    const std::uint16_t descriptorAddress = read_INDEX();
    const auto descriptor = variables_const().pointer(descriptorAddress);
    const std::uint8_t length = descriptor.read(descriptorOffset);
    if (length == 0u) {
        CHECK_BUMP();
        return;
    }

    const std::uint16_t stringAddress = ApplesoftVariables::makeWord(
        descriptor.read(static_cast<std::uint16_t>(descriptorOffset + 1u)),
        descriptor.read(static_cast<std::uint16_t>(descriptorOffset + 2u)));

    // Ignore strings already collected at/above FRETOP.
    if (stringAddress >= read_FRETOP()) {
        CHECK_BUMP();
        return;
    }

    // Keep highest string below FRETOP.
    if (stringAddress < read_LOWTR()) {
        CHECK_BUMP();
        return;
    }

    write_LOWTR(stringAddress);
    write_FNCNAM(descriptorAddress);
    write_LENGTH(read_DSCLEN());
    CHECK_BUMP();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHECK_BUMP (inclusive) .. CHECK_EXIT (exclusive)
// Name normalization: none (assembler label CHECK_BUMP kept verbatim).
void CHECK_BUMP() {
    write_INDEX(static_cast<std::uint16_t>(read_INDEX() + read_DSCLEN()));
    CHECK_EXIT();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHECK_EXIT (inclusive) .. MOVE_HIGHEST_STRING_TO_TOP (exclusive)
// Name normalization: none (assembler label CHECK_EXIT kept verbatim).
void CHECK_EXIT() {
    // RTS in ROM; state is already in INDEX and Y=0 equivalent in this model.
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MOVE_HIGHEST_STRING_TO_TOP (inclusive) .. CAT (exclusive)
// Name normalization: none (assembler label MOVE_HIGHEST_STRING_TO_TOP kept verbatim).
void MOVE_HIGHEST_STRING_TO_TOP() {
    const std::uint16_t descriptorAddress = read_FNCNAM();
    if (ApplesoftVariables::highByte(descriptorAddress) == 0u) {
        CHECK_EXIT();
        return;
    }

    auto descriptor = variables().pointer(descriptorAddress);
    const std::uint8_t lengthSlotOffset = static_cast<std::uint8_t>((read_LENGTH() & 0x04u) >> 1u);
    write_LENGTH(lengthSlotOffset);

    const std::uint8_t stringLength = descriptor.read(lengthSlotOffset);
    const std::uint16_t sourceAddress = read_LOWTR();
    const std::uint16_t highSource = static_cast<std::uint16_t>(sourceAddress + stringLength);
    write_HIGHTR(highSource);

    // HIGHDS/HIGHDS+1 aliases ARYPNT in this memory layout.
    const std::uint16_t destination = static_cast<std::uint16_t>(read_FRETOP() - stringLength);
    write_ARYPNT(destination);

    // Preserve copy intent of BLTU2 for mapped memory regions.
    if (stringLength != 0u) {
        for (std::uint16_t i = stringLength; i > 0u; --i) {
            const std::uint8_t b = variables_const().readByte(static_cast<std::uint16_t>(sourceAddress + i - 1u));
            variables().writeByte(static_cast<std::uint16_t>(destination + i - 1u), b);
        }
    }

    write_FRETOP(destination);

    const std::uint8_t pointerOffset = static_cast<std::uint8_t>(lengthSlotOffset + 1u);
    descriptor.write(ApplesoftVariables::lowByte(destination), pointerOffset);
    descriptor.write(ApplesoftVariables::highByte(destination), static_cast<std::uint16_t>(pointerOffset + 1u));

    // Continue scanning until no more non-empty strings remain below FRETOP.
    FIND_HIGHEST_STRING();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CAT (inclusive) .. MOVINS (exclusive)
// Name normalization: none (assembler label CAT kept verbatim).
static void CAT_impl() {
    const std::uint16_t firstDescriptor = read_FAC_descriptor_address();
    write_STRNG1(firstDescriptor);

    FRM_ELEMENT();
    CHKSTR();

    const std::uint16_t secondDescriptor = read_FAC_descriptor_address();
    const auto first = variables_const().pointer(firstDescriptor);
    const auto second = variables_const().pointer(secondDescriptor);

    const std::uint16_t totalLength = static_cast<std::uint16_t>(first.read() + second.read());
    if (totalLength > 0xffu) {
        ERROR(ERR_STRLONG);
        return;
    }

    STRINI(static_cast<std::uint8_t>(totalLength));
    MOVINS();

    const std::uint8_t secondLength = FRETMP(read_DSCPTR());
    MOVSTR_1(secondLength);

    (void)FRETMP(read_STRNG1());
    PUTNEW();
    FRMEVL_2();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MOVSTR (inclusive) .. MOVSTR_1 (exclusive)
// Name normalization: none (assembler label MOVSTR kept verbatim).
void MOVSTR(std::uint8_t x, std::uint8_t y, std::uint8_t length) {
    // Pointer candidate lifted: INDEX low/high is one conceptual source pointer.
    write_INDEX(ApplesoftVariables::makeWord(x, y));
    MOVSTR_1(length);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MOVSTR_1 (inclusive) .. FRESTR (exclusive)
// Name normalization: none (assembler label MOVSTR_1 kept verbatim).
void MOVSTR_1(std::uint8_t length) {
    const std::uint16_t source = read_INDEX();
    const std::uint16_t destination = read_FRESPC();

    for (std::uint16_t i = 0; i < length; ++i) {
        const std::uint8_t b = variables_const().readByte(static_cast<std::uint16_t>(source + i));
        variables().writeByte(static_cast<std::uint16_t>(destination + i), b);
    }

    // Carry-chain FRESPC/FRESPC+1 update lifted to unified pointer arithmetic.
    write_FRESPC(static_cast<std::uint16_t>(destination + length));
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: FRESTR (inclusive) .. FREFAC (exclusive)
// Name normalization: none (assembler label FRESTR kept verbatim).
std::uint8_t FRESTR() {
    CHKSTR();
    return FREFAC();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: FREFAC (inclusive) .. FRETMP (exclusive)
// Name normalization: none (assembler label FREFAC kept verbatim).
std::uint8_t FREFAC() {
    return FRETMP(read_FAC_descriptor_address());
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PUTNEW (inclusive) .. JERR (exclusive)
// Name normalization: none (assembler label PUTNEW kept verbatim).
void PUTNEW() {
    const std::uint8_t tempDescriptorAddress = read_TEMPPT();
    const std::uint8_t maxTempDescriptorAddress = static_cast<std::uint8_t>(ApplesoftVariables::ZP_TEMPST + 9u);
    if (tempDescriptorAddress == maxTempDescriptorAddress) {
        g_jerr_error = ERR_FRMCPX;
        JERR();
        return;
    }

    // Branch target in ROM is PUTEMP.
    PUTEMP(tempDescriptorAddress);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: JERR (inclusive) .. PUTEMP (exclusive)
// Name normalization: none (assembler label JERR kept verbatim).
void JERR() {
    ERROR(g_jerr_error);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PUTEMP (inclusive) .. GETSPA (exclusive)
// Name normalization: none (assembler label PUTEMP kept verbatim).
void PUTEMP(std::uint8_t tempDescriptorAddress) {
    auto tempDescriptor = variables().pointer(tempDescriptorAddress);
    tempDescriptor.write(variables_const().readByte(ApplesoftVariables::ZP_FAC));
    tempDescriptor.write(variables_const().readByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 1u)), 1u);
    tempDescriptor.write(variables_const().readByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 2u)), 2u);

    // FAC+3/FAC+4 is one logical pointer to the temp descriptor.
    write_FAC_descriptor_address(static_cast<std::uint16_t>(tempDescriptorAddress));

    // VALTYP=$ff marks FAC as string.
    variables().writeByte(ApplesoftVariables::ZP_VALTYP, 0xffu);
    write_LASTPT(tempDescriptorAddress);

    write_TEMPPT(static_cast<std::uint8_t>(tempDescriptorAddress + 3u));
}

void STRLIT(std::uint16_t address);
} // namespace

// Labels: STRINI (inclusive) .. STRSPA (exclusive)
// Name normalization: none (assembler label STRINI kept verbatim).
void STRINI(std::uint8_t length) {
    // Pointer candidate lifted: FAC+3/FAC+4 is one descriptor pointer.
    const std::uint16_t descriptorAddress = read_FAC_descriptor_address();
    write_DSCPTR(descriptorAddress);

    // Original control flow falls through directly into STRSPA.
    STRSPA(length);
}

void CAT() {
    CAT_impl();
}

// Labels: STRSPA (inclusive) .. L_STRSPA_1 (exclusive)
// Name normalization: none (assembler label STRSPA kept verbatim).
void STRSPA(std::uint8_t length) {
    // GETSPA: allocate space at bottom of string space (A=length).
    // Returns address in Y,X.
    const std::uint16_t allocatedAddress = GETSPA(length);

    // Store length and address into FAC.
    variables().writeByte(ApplesoftVariables::ZP_FAC, length);
    write_FAC_pointer(allocatedAddress);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: LEN (inclusive) .. GETSTR (exclusive)
// Name normalization: none (assembler label LEN kept verbatim).
//
// "LEN" built-in: get string length and float it into FAC.
void LEN() {
    const std::uint8_t length = GETSTR();
    SNGFLT(length);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GETSTR (inclusive) .. ASC (exclusive)
// Name normalization: none (assembler label GETSTR kept verbatim).
//
// Free the FAC string if temporary, clear VALTYP to numeric, return length.
// After return INDEX points at the string data.
std::uint8_t GETSTR() {
    const std::uint8_t length = FRESTR();
    variables().writeByte(ApplesoftVariables::ZP_VALTYP, 0u);
    return length;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: ASC (inclusive) .. GOIQ (exclusive)
// Name normalization: none (assembler label ASC kept verbatim).
//
// "ASC" built-in: return ASCII value of first character.
// GOIQ ($e6f2) is a one-instruction trampoline to IQERR; inlined here.
void ASC() {
    const std::uint8_t length = GETSTR();
    if (length == 0u) {
        IQERR();  // GOIQ: jmp IQERR — illegal quantity for empty string
        return;
    }
    const std::uint16_t strAddr = read_INDEX();
    const std::uint8_t ch = variables_const().readByte(strAddr);
    SNGFLT(ch);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: GARBAG (inclusive) .. FIND_HIGHEST_STRING (exclusive)
// Name normalization: none (assembler label GARBAG kept verbatim).
void GARBAG() {
    // Collect from top down: initialize FRETOP from MEMSIZ, then fall through.
    write_FRETOP(read_MEMSIZ());
    FIND_HIGHEST_STRING();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MOVINS (inclusive) .. MOVSTR (exclusive)
// Name normalization: none (assembler label MOVINS kept verbatim).
void MOVINS() {
    const auto descriptor = variables_const().pointer(read_STRNG1());
    const std::uint8_t length = descriptor.read(0u);
    const std::uint8_t x = descriptor.read(1u);
    const std::uint8_t y = descriptor.read(2u);
    MOVSTR(x, y, length);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: FRETMP (inclusive) .. FRETMS (exclusive)
// Name normalization: none (assembler label FRETMP kept verbatim).
std::uint8_t FRETMP(std::uint16_t descriptorAddress) {
    write_INDEX(descriptorAddress);
    const bool isTemporary = FRETMS(descriptorAddress);

    const auto descriptor = variables_const().pointer(descriptorAddress);
    const std::uint8_t length = descriptor.read(0u);
    const std::uint16_t stringAddress = ApplesoftVariables::makeWord(descriptor.read(1u), descriptor.read(2u));

    if (isTemporary && stringAddress == read_FRETOP()) {
        write_FRETOP(static_cast<std::uint16_t>(read_FRETOP() + length));
    }

    write_INDEX(stringAddress);
    return length;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: FRETMS (inclusive) .. CHRSTR (exclusive)
// Name normalization: none (assembler label FRETMS kept verbatim).
bool FRETMS(std::uint16_t descriptorAddress) {
    const std::uint8_t a = ApplesoftVariables::lowByte(descriptorAddress);
    const std::uint8_t y = ApplesoftVariables::highByte(descriptorAddress);
    const std::uint8_t lastpt = variables_const().readByte(ApplesoftVariables::ZP_LASTPT);
    const std::uint8_t lastptHi = variables_const().readByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_LASTPT + 1u));

    if (y != lastptHi || a != lastpt) {
        return false;
    }

    // Descriptor is latest temporary: release by rewinding TEMPPT/LASTPT.
    write_TEMPPT(a);
    write_LASTPT(static_cast<std::uint8_t>(a - 3u));
    return true;
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHRSTR (inclusive) .. LEFTSTR (exclusive)
// Name normalization: none (assembler label CHRSTR kept verbatim).
//
// CHR$() built-in: parse the argument byte via CONINT, allocate a 1-byte string
// with STRSPA, store the character into the newly allocated space, then publish
// a temporary string descriptor with PUTNEW.
// ROM pops two return addresses before jumping to PUTNEW because CHRSTR was
// reached via the UNFNC dispatch JSR chain; in C++ the call frames unwind
// normally so no explicit pop is needed.
void CHRSTR() {
    // CONINT: evaluate the current FAC argument as an integer byte (0-255).
    // The ROM convention is X register = result byte.  In the C++ port, CONINT
    // writes the converted value into FAC[4] (the X-register proxy at ZP_FAC+4);
    // we read it back from there.
    CONINT();
    const std::uint8_t ch = variables_const().readByte(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 4u));

    // Allocate space for the 1-byte string.  STRSPA sets FAC[0]=1 (length) and
    // FAC+1,2 = allocated address.
    STRSPA(1u);

    // sta (FAC+1),Y  with Y=0: write the character to the allocated string space.
    // FAC+1/FAC+2 is one unified pointer (write_FAC_pointer uses the same address).
    const std::uint16_t strData = variables_const().readWord(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 1u));
    variables().writeByte(strData, ch);

    // jmp PUTNEW: convert the raw FAC string data into a temporary descriptor.
    PUTNEW();
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: LEFTSTR (inclusive) .. RIGHTSTR (exclusive)
// Name normalization: none (assembler label LEFTSTR kept verbatim).
void LEFTSTR() {
    SubstringSetupResult setup{};
    if (!SUBSTRING_SETUP(setup)) {
        return;
    }

    const std::uint8_t copyLength = std::min(setup.firstParameter, setup.sourceLength);
    SUBSTRING_BUILD(0u, copyLength);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: RIGHTSTR (inclusive) .. MIDSTR (exclusive)
// Name normalization: none (assembler label RIGHTSTR kept verbatim).
void RIGHTSTR() {
    SubstringSetupResult setup{};
    if (!SUBSTRING_SETUP(setup)) {
        return;
    }

    const std::uint8_t copyLength = std::min(setup.firstParameter, setup.sourceLength);
    const std::uint8_t leftStart = static_cast<std::uint8_t>(setup.sourceLength - copyLength);
    SUBSTRING_BUILD(leftStart, copyLength);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: MIDSTR (inclusive) .. SUBSTRING_SETUP (exclusive)
// Name normalization: none (assembler label MIDSTR kept verbatim).
void MIDSTR() {
    std::uint8_t requestedWidth = 0xffu;
    if (CHRGOT() != static_cast<std::uint8_t>(')')) {
        CHKCOM();
        requestedWidth = GETBYT();
    }

    SubstringSetupResult setup{};
    if (!SUBSTRING_SETUP(setup)) {
        return;
    }

    const std::uint8_t leftStart = static_cast<std::uint8_t>(setup.firstParameter - 1u);
    if (leftStart >= setup.sourceLength) {
        SUBSTRING_BUILD(leftStart, 0u);
        return;
    }

    const std::uint8_t remaining = static_cast<std::uint8_t>(setup.sourceLength - leftStart);
    const std::uint8_t copyLength = std::min(remaining, requestedWidth);
    SUBSTRING_BUILD(leftStart, copyLength);
}

void STRLT2(std::uint16_t address) {
    // BUILD A DESCRIPTOR FOR STRING STARTING AT address
    // AND TERMINATED BY $00, (CHARAC), OR (ENDCHR)

    const std::uint16_t start = address;
    const auto startPtr = variables_const().pointer(start);
    write_STRNG1(start);
    write_FAC_pointer(start);

    std::uint8_t length = 0;
    bool include_quote_in_length = false;

    // Find end of string, terminated by $00 or alternate terminators.
    while (true) {
        const std::uint8_t ch = startPtr.read(length);
        if (ch == 0) {
            break;
        }
        if (ch == read_CHARAC() || ch == read_ENDCHR()) {
            include_quote_in_length = (ch == static_cast<std::uint8_t>(0x22));
            break;
        }
        ++length;
    }

    if (include_quote_in_length) {
        ++length;
    }

    write_FAC(length);

    const std::uint16_t endAddress = startPtr.advanced(length).address();
    write_STRNG2(endAddress);

    // If source is not on page 0 or page 2, branch directly to PUTNEW.
    const std::uint8_t startPage = ApplesoftVariables::highByte(start);
    if (startPage != 0 && startPage != 2) {
        PUTNEW();
        return;
    }

    // For page 0/2 source, allocate and move string before PUTNEW handling.
    STRINI(length);
    MOVSTR(ApplesoftVariables::lowByte(start), ApplesoftVariables::highByte(start), length);

    // Fall-through target in original control flow is PUTNEW.
    PUTNEW();
}

} // namespace applesoft::asm_port
