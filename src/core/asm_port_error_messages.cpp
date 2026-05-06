// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: ERROR_MESSAGES (inclusive) .. QT_ERROR (exclusive)
// Name normalization: none
//
// In ROM, error messages are stored as packed bytes with bit 7 set on the last
// character of each message, and callers index by byte offset. This conversion
// preserves lookup semantics by taking the original offset and returning the
// decoded message text.

#include "core/asm_port_error_messages.hpp"

namespace applesoft::asm_port {

std::string_view ERROR_MESSAGES(std::uint8_t offset) {
    switch (offset) {
    case ERR_NOFOR:     return "NEXT WITHOUT FOR";
    case ERR_SYNTAX:    return "SYNTAX";
    case ERR_NOGOSUB:   return "RETURN WITHOUT GOSUB";
    case ERR_NODATA:    return "OUT OF DATA";
    case ERR_ILLQTY:    return "ILLEGAL QUANTITY";
    case ERR_OVERFLOW:  return "OVERFLOW";
    case ERR_MEMFULL:   return "OUT OF MEMORY";
    case ERR_UNDEFSTAT: return "UNDEF'D STATEMENT";
    case ERR_BADSUBS:   return "BAD SUBSCRIPT";
    case ERR_REDIMD:    return "REDIM'D ARRAY";
    case ERR_ZERODIV:   return "DIVISION BY ZERO";
    case ERR_ILLDIR:    return "ILLEGAL DIRECT";
    case ERR_BADTYPE:   return "TYPE MISMATCH";
    case ERR_STRLONG:   return "STRING TOO LONG";
    case ERR_FRMCPX:    return "FORMULA TOO COMPLEX";
    case ERR_CANTCONT:  return "CAN'T CONTINUE";
    case ERR_UNDEFFUNC: return "UNDEF'D FUNCTION";
    default:            return {};
    }
}

} // namespace applesoft::asm_port
