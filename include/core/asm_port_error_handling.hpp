#pragma once

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

// Target: ERROR (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1337)
// Function: Main error handler for Applesoft BASIC.
void ERROR(std::uint8_t error_code_offset);

// Target: MEMERR (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1329)
std::uint8_t MEMERR();

// Target: PRINT_ERROR_LINNUM (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1356)
void PRINT_ERROR_LINNUM();

// Labels: FIN (inclusive) .. FIN_9 (exclusive)
void FIN();

// Target: SYNER1 (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 1404)
void SYNERR();

// Target: IQERR (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst Line 3621)
void IQERR();

// Target: JER (Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst)
void JER();

} // namespace applesoft::asm_port
