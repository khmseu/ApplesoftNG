// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ERROR_MESSAGES (inclusive) .. AS_QT_ERROR (exclusive)
// Name normalization: none
//
// In ROM, error messages are stored as packed bytes with bit 7 set on the last
// character of each message, and callers index by byte offset. This conversion
// preserves lookup semantics by taking the original offset and returning the
// decoded message text.

#include "core/asm_port_error_messages.hpp"

namespace applesoft::asm_port {

std::string_view AS_ERROR_MESSAGES(std::uint8_t offset) {
  switch (offset) {
  case AS_ERR_NOFOR:
    return "AS_NEXT WITHOUT AS_FOR";
  case AS_ERR_SYNTAX:
    return "SYNTAX";
  case AS_ERR_NOGOSUB:
    return "AS_RETURN WITHOUT AS_GOSUB";
  case AS_ERR_NODATA:
    return "OUT OF AS_DATA";
  case AS_ERR_ILLQTY:
    return "ILLEGAL QUANTITY";
  case AS_ERR_OVERFLOW:
    return "AS_OVERFLOW";
  case AS_ERR_MEMFULL:
    return "OUT OF MEMORY";
  case AS_ERR_UNDEFSTAT:
    return "UNDEF'D STATEMENT";
  case AS_ERR_BADSUBS:
    return "BAD SUBSCRIPT";
  case AS_ERR_REDIMD:
    return "REDIM'D AS_ARRAY";
  case AS_ERR_ZERODIV:
    return "AS_DIVISION BY AS_ZERO";
  case AS_ERR_ILLDIR:
    return "ILLEGAL DIRECT";
  case AS_ERR_BADTYPE:
    return "TYPE MISMATCH";
  case AS_ERR_STRLONG:
    return "AS_STRING TOO AS_LONG";
  case AS_ERR_FRMCPX:
    return "AS_FORMULA TOO COMPLEX";
  case AS_ERR_CANTCONT:
    return "CAN'T AS_CONTINUE";
  case AS_ERR_UNDEFFUNC:
    return "UNDEF'D AS_FUNCTION";
  default:
    return {};
  }
}

} // namespace applesoft::asm_port
