// In ROM, error messages are stored as packed bytes with bit 7 set on the last
// character of each message, and callers index by byte offset. This conversion
// preserves lookup semantics by taking the original offset and returning the
// decoded message text.

#include "core/asm_port_error_messages.hpp"

#include <array>
#include <cstring>

namespace {

using applesoft::asm_port::ApplesoftDualPointer;

struct ErrorMessageEntry {
  std::uint8_t offset;
  const char *text;
};

constexpr std::array<ErrorMessageEntry, 17> kErrorMessageTable = {{
    {applesoft::asm_port::AS_ERR_NOFOR, "AS_NEXT WITHOUT AS_FOR"},
    {applesoft::asm_port::AS_ERR_SYNTAX, "SYNTAX"},
    {applesoft::asm_port::AS_ERR_NOGOSUB, "AS_RETURN WITHOUT AS_GOSUB"},
    {applesoft::asm_port::AS_ERR_NODATA, "OUT OF AS_DATA"},
    {applesoft::asm_port::AS_ERR_ILLQTY, "ILLEGAL QUANTITY"},
    {applesoft::asm_port::AS_ERR_OVERFLOW, "AS_OVERFLOW"},
    {applesoft::asm_port::AS_ERR_MEMFULL, "OUT OF MEMORY"},
    {applesoft::asm_port::AS_ERR_UNDEFSTAT, "UNDEF'D STATEMENT"},
    {applesoft::asm_port::AS_ERR_BADSUBS, "BAD SUBSCRIPT"},
    {applesoft::asm_port::AS_ERR_REDIMD, "REDIM'D AS_ARRAY"},
    {applesoft::asm_port::AS_ERR_ZERODIV, "AS_DIVISION BY AS_ZERO"},
    {applesoft::asm_port::AS_ERR_ILLDIR, "ILLEGAL DIRECT"},
    {applesoft::asm_port::AS_ERR_BADTYPE, "TYPE MISMATCH"},
    {applesoft::asm_port::AS_ERR_STRLONG, "AS_STRING TOO AS_LONG"},
    {applesoft::asm_port::AS_ERR_FRMCPX, "AS_FORMULA TOO COMPLEX"},
    {applesoft::asm_port::AS_ERR_CANTCONT, "CAN'T AS_CONTINUE"},
    {applesoft::asm_port::AS_ERR_UNDEFFUNC, "UNDEF'D AS_FUNCTION"},
}};

const ErrorMessageEntry *findErrorMessage(std::uint8_t offset) {
  for (const auto &entry : kErrorMessageTable) {
    if (entry.offset == offset) {
      return &entry;
    }
  }
  return nullptr;
}

constexpr char kEmptyMessage[] = "";

} // namespace

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ERROR_MESSAGES (inclusive) .. AS_QT_ERROR (exclusive)
// Name normalization: minimal suffix required for C++ overload disambiguation.
ApplesoftDualPointer<const std::uint8_t>
AS_ERROR_MESSAGES_ptr(std::uint8_t offset) {
  const ErrorMessageEntry *entry = findErrorMessage(offset);
  if (entry == nullptr) {
    return ApplesoftDualPointer<const std::uint8_t>::native(
        reinterpret_cast<const std::uint8_t *>(kEmptyMessage));
  }

  return ApplesoftDualPointer<const std::uint8_t>::native(
      reinterpret_cast<const std::uint8_t *>(entry->text));
}

std::string_view
AS_ERROR_MESSAGES(ApplesoftDualPointer<const std::uint8_t> message_ptr) {
  if (!message_ptr.isNative()) {
    return {};
  }

  const std::uint8_t *bytes = message_ptr.nativePointer();
  if (bytes == nullptr || bytes[0] == 0u) {
    return {};
  }

  const char *text = reinterpret_cast<const char *>(bytes);
  return std::string_view(text, std::strlen(text));
}

std::string_view AS_ERROR_MESSAGES(std::uint8_t offset) {
  return AS_ERROR_MESSAGES(AS_ERROR_MESSAGES_ptr(offset));
}

} // namespace applesoft::asm_port
