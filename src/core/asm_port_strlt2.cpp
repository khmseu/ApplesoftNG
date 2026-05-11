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

std::uint16_t read_STREND() {
    return variables_const().readWord(ApplesoftVariables::ZP_STREND);
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

// TODO(asm-port): port GARBAG label.
void GARBAG() {}

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

        GARBAG();
        write_GARFLG(0x80u);
    }
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
