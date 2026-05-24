#include "core/asm_port_error_handling.hpp"

#include "core/asm_port_clear.hpp"
#include "core/asm_port_control_flow.hpp"
#include "core/asm_port_core.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_qt_error.hpp"
#include "platform/asm_port_outdo.hpp"

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ERROR (inclusive) .. AS_PRINT_ERROR_LINNUM (exclusive)
// Name normalization: none (assembler label AS_ERROR kept verbatim).
void AS_ERROR(std::uint8_t error_code_offset) {
  gPendingErrorCode = error_code_offset;

  if (IsOnErr()) {
    AS_HANDLERR();
    return;
  }

  AS_CRDO();
  AS_OUTQUES();

  // Print the message from AS_ERROR_MESSAGES
  AS_STROUT(AS_ERROR_MESSAGES(AS_ERROR_MESSAGES_ptr(error_code_offset)));

  AS_STKINI();
  AS_PRINT_ERROR_LINNUM();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_MEMERR (inclusive) .. AS_ERROR (exclusive)
// Name normalization: none (assembler label AS_MEMERR kept verbatim).
std::uint8_t AS_MEMERR() {
  AS_ERROR(AS_ERR_MEMFULL);
  return AS_ERR_MEMFULL;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PRINT_ERROR_LINNUM (inclusive) .. AS_RESTART (exclusive)
// Name normalization: none (assembler label AS_PRINT_ERROR_LINNUM kept
// verbatim).
void AS_PRINT_ERROR_LINNUM() {
  // AS_QT_ERROR(0) is " AS_ERROR" + BELL
  AS_PRINT_ERROR_LINNUM(
      std::string_view(AS_QT_ERROR(0))); // Use index 0 for " AS_ERROR"
}

void AS_PRINT_ERROR_LINNUM(std::string_view prefix) {
  AS_STROUT(prefix);

  if (!IsDirectMode()) {
    AS_INPRT(); // Prints " IN " + line number
  }

  AS_RESTART();
}

void AS_SYNERR() {
  // Source: applesoft.o65.lst:1404
  AS_ERROR(AS_ERR_SYNTAX);
}

void AS_JER() { AS_ERROR(gJerErrorCode); }

void AS_IQERR() {
  // Source: applesoft.o65.lst:3621
  gJerErrorCode = AS_ERR_ILLQTY;
  AS_JER();
}

} // namespace applesoft::asm_port
