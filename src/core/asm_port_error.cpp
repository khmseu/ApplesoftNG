#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"

namespace applesoft::asm_port {

std::int8_t gNumericCompareResult = 0;
bool gNumericCompareCarry = false;
std::uint8_t gFloatInput = 0;

std::uint8_t gPendingErrorCode = AS_ERR_SYNTAX;

std::uint8_t gJerErrorCode = AS_ERR_SYNTAX;

} // namespace applesoft::asm_port
