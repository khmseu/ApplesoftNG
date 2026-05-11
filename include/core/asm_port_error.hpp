#pragma once

#include "core/asm_port_qt_error.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

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
void STROUT(std::string_view text);
void INPRT();
void STKINI();
void HANDLERR();
bool IsOnErr();
bool IsDirectMode();
void OUTQUES();

} // namespace applesoft::asm_port
