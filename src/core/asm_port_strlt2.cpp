#include "core/asm_port_strlt2.hpp"

#include "core/applesoft_variables.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"

#include <cstdint>

namespace applesoft::asm_port {
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

std::uint16_t read_FAC_descriptor_address() {
    return variables_const().readWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 1u));
}

void write_FAC_descriptor_address(std::uint16_t value) {
    variables().writeWord(static_cast<std::uint8_t>(ApplesoftVariables::ZP_FAC + 1u), value);
}

std::uint8_t g_jerr_error = ERR_FRMCPX;

void JERR();
void PUTEMP(std::uint8_t tempDescriptorAddress);
std::uint16_t GETSPA(std::uint8_t length);
void STRSPA(std::uint8_t length);
void GARBAG();
void FIND_HIGHEST_STRING();
void CHECK_SIMPLE_VARIABLE();
void CHECK_VARIABLE(std::uint8_t descriptorOffset);
void CHECK_BUMP();
void CHECK_EXIT();
void MOVE_HIGHEST_STRING_TO_TOP();

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
// Labels: GARBAG (inclusive) .. FIND_HIGHEST_STRING (exclusive)
// Name normalization: none (assembler label GARBAG kept verbatim).
void GARBAG() {
    // Collect from top down: initialize FRETOP from MEMSIZ, then fall through.
    write_FRETOP(read_MEMSIZ());
    FIND_HIGHEST_STRING();
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
// Labels: STRINI (inclusive) .. STRSPA (exclusive)
// Name normalization: none (assembler label STRINI kept verbatim).
void STRINI(std::uint8_t length) {
    // Pointer candidate lifted: FAC+3/FAC+4 is one descriptor pointer.
    write_DSCPTR(read_FAC_descriptor_address());

    // Original control flow falls through directly into STRSPA.
    STRSPA(length);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: STRSPA (inclusive) .. STRLIT (exclusive)
// Name normalization: none (assembler label STRSPA kept verbatim).
void STRSPA(std::uint8_t length) {
    const std::uint16_t allocated = GETSPA(length);
    write_FAC(length);
    write_FAC_descriptor_address(allocated);
}

// TODO(asm-port): port MOVSTR label.
void MOVSTR(std::uint8_t /*x*/, std::uint8_t /*y*/) {}

void MOVSTR(std::uint16_t address) {
    MOVSTR(ApplesoftVariables::lowByte(address), ApplesoftVariables::highByte(address));
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

} // namespace

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
    MOVSTR(start);

    // Fall-through target in original control flow is PUTNEW.
    PUTNEW();
}

} // namespace applesoft::asm_port
