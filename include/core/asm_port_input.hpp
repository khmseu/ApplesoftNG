#pragma once

#include <cstdint>

namespace applesoft::asm_port {
void AS_INPUT();
void AS_NXIN();
void AS_READ();
void AS_INPUT_FLAG_ZERO(std::uint16_t input_ptr);
void AS_PROCESS_INPUT_LIST(std::uint16_t input_ptr, std::uint8_t input_flag);
void AS_INPUTERR();
void AS_READERR();
void AS_ERLIN(std::uint16_t lineNumber);
void AS_INPERR();
void AS_RESPERR();
void AS_GET();

} // namespace applesoft::asm_port