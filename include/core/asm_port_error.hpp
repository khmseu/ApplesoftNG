#pragma once

#include "core/asm_port_qt_error.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);

struct ProgramPointer {
    std::uint16_t address = 0;

    std::uint8_t read(std::uint16_t offset = 0) const {
        return ReadProgramByte(static_cast<std::uint16_t>(address + offset));
    }

    void write(std::uint8_t value, std::uint16_t offset = 0) const {
        WriteProgramByte(static_cast<std::uint16_t>(address + offset), value);
    }

    ProgramPointer advanced(std::uint16_t offset) const {
        return ProgramPointer{static_cast<std::uint16_t>(address + offset)};
    }
};

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: CHRGET / CHRGOT
// Name normalization: none (assembler labels kept verbatim).
std::uint8_t CHRGET();
std::uint8_t CHRGOT();
void CONINT();
std::uint8_t GETBYT();
void GTBYTC();
void SNGFLT(std::uint8_t value);

    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ANDOP (inclusive) .. FALSE (exclusive)
    // Name normalization: none (assembler label ANDOP kept verbatim).
    void ANDOP() ;

    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKCOM (inclusive) .. SYNCHR (exclusive)
    // Name normalization: none (assembler label CHKCOM kept verbatim).
    void CHKCOM() ;

    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CHKNUM (inclusive) .. CHKSTR (exclusive)
    // Name normalization: none (assembler label CHKNUM kept verbatim).
    void CHKNUM() ;



    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET (inclusive) .. PTRGET2 (exclusive)
    // Name normalization: none (assembler label PTRGET kept verbatim).
std::uint16_t PTRGET() ;



// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: ERROR (inclusive) .. PRINT_ERROR_LINNUM (exclusive)
// Name normalization: none (assembler label ERROR kept verbatim).
void ERROR(std::uint8_t error_code_offset);

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PRINT_ERROR_LINNUM (inclusive) .. RESTART (exclusive)
// Name normalization: none (assembler label PRINT_ERROR_LINNUM kept verbatim).
void PRINT_ERROR_LINUM(std::string_view prefix = QT_ERROR(QT_ERROR_INDEX));

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: RESTART (inclusive)
// Name normalization: none (assembler label RESTART kept verbatim).
void RESTART();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: RESTORE (inclusive)
// Name normalization: none (assembler label RESTORE kept verbatim).
void RESTORE();

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: FIX_LINKS (inclusive)
// Name normalization: none (assembler label FIX_LINKS kept verbatim).
void FIX_LINKS();

// Helper stubs for error printing and restart handling.
void CRDO();
void INPRT();
void STKINI();
void HANDLERR();
bool IsOnErr();
bool IsDirectMode();
void OUTQUES();

} // namespace applesoft::asm_port
