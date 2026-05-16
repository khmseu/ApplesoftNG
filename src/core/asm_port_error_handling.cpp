#include "core/asm_port_error_handling.hpp"

#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_print.hpp"
#include "core/asm_port_qt_error.hpp"

namespace applesoft::asm_port {

// Helper status functions (often found in asm_port_core.cpp or
// asm_port_error.cpp)
extern std::uint8_t gPendingErrorCode;
extern std::uint8_t gJerErrorCode;

bool IsOnErr();
bool IsDirectMode();
void AS_OUTQUES();
void AS_CRDO();
void AS_STKINI();
void AS_HANDLERR();
void AS_INPRT();
void AS_RESTART();

void AS_ERROR(std::uint8_t error_code_offset) {
  // Source: applesoft.o65.lst:1337

  gPendingErrorCode = error_code_offset;

  if (IsOnErr()) {
    AS_HANDLERR();
    return;
  }

  AS_CRDO();
  AS_OUTQUES();

  // Print the message from AS_ERROR_MESSAGES
  AS_STROUT(AS_ERROR_MESSAGES(error_code_offset));

  AS_STKINI();
  AS_PRINT_ERROR_LINNUM();
}

std::uint8_t AS_MEMERR() {
  // Source: applesoft.o65.lst:1329
  AS_ERROR(AS_ERR_MEMFULL);
  return AS_ERR_MEMFULL;
}

void AS_PRINT_ERROR_LINNUM() {
  // Source: applesoft.o65.lst:1356

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
