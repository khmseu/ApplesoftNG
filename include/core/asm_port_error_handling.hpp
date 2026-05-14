#pragma once

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

// Target: ERROR (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1337)
// Function: Main error handler for Applesoft BASIC.
// If ON ERR is active, jumps to user handler. Otherwise, prints "?" + message + " ERROR" + " IN [linenum]" and RESTARTs.
// (X) = offset into ERROR_MESSAGES table.
void ERROR(std::uint8_t error_code_offset);

// Target: MEMERR (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1329)
// Function: Entry point for "OM" (Out of Memory) error.
std::uint8_t MEMERR();

// Target: PRINT_ERROR_LINNUM (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1356)
// Function: Prints " ERROR" and BELL, then if not in direct mode, prints " IN [CURLIN]".
// Then enters warm RESTART.
void PRINT_ERROR_LINNUM();
void PRINT_ERROR_LINNUM(std::string_view prefix);

// Target: SYNER1 (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1404)
// Function: Entry point for "?SYNTAX ERROR".
void SYNERR();

// Target: IQERR (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 3621)
// Function: Entry point for "?ILLEGAL QUANTITY ERROR".
void IQERR();

// Target: JER (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst)
// Function: Jumps to ERROR with code in gJerErrorCode.
void JER();

} // namespace applesoft::asm_port
