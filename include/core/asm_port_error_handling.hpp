#pragma once

#include "core/asm_port_math.hpp"

#include <cstdint>
#include <string_view>

namespace applesoft::asm_port {

// Target: AS_ERROR (Source:
// SourceMaterial/Combo/asrom.lst
// AS_Line 1337) Function: Main error handler for Applesoft AS_BASIC.
void AS_ERROR(std::uint8_t error_code_offset);

// Target: AS_MEMERR (Source:
// SourceMaterial/Combo/asrom.lst
// AS_Line 1329)
std::uint8_t AS_MEMERR();

// Target: AS_PRINT_ERROR_LINNUM (Source:
// SourceMaterial/Combo/asrom.lst
// AS_Line 1356)
void AS_PRINT_ERROR_LINNUM();
void AS_PRINT_ERROR_LINNUM(std::string_view prefix);

// Target: SYNER1 (Source:
// SourceMaterial/Combo/asrom.lst
// AS_Line 1404)
void AS_SYNERR();

// Target: AS_IQERR (Source:
// SourceMaterial/Combo/asrom.lst
// AS_Line 3621)
void AS_IQERR();

// Target: AS_JER (Source:
// SourceMaterial/Combo/asrom.lst)
void AS_JER();

} // namespace applesoft::asm_port
