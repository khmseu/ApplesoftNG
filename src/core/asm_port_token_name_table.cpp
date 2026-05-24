// The original ROM stores token names as a packed character stream with bit 7
// set on the final character of each token. This port exposes the same table
// semantics as decoded token text indexed by (token - 0x80).

#include "core/asm_port_token_name_table.hpp"

#include <array>
#include <cstring>

namespace {

using applesoft::asm_port::ApplesoftDualPointer;

constexpr std::array<const char *, 107> kTokenNameTable = {
    "END",        "AS_FOR",     "AS_NEXT",   "AS_DATA",    "AS_INPUT",
    "AS_DEL",     "AS_DIM",     "AS_READ",   "AS_GR",      "AS_TEXT",
    "PR#",        "IN#",        "AS_CALL",   "AS_PLOT",    "AS_HLIN",
    "AS_VLIN",    "AS_HGR2",    "AS_HGR",    "AS_HCOLOR=", "AS_HPLOT",
    "AS_DRAW",    "AS_XDRAW",   "AS_HTAB",   "HOME",       "AS_ROT=",
    "AS_SCALE=",  "AS_SHLOAD",  "AS_TRACE",  "AS_NOTRACE", "AS_NORMAL",
    "AS_INVERSE", "AS_FLASH",   "AS_COLOR=", "AS_POP",     "AS_VTAB",
    "AS_HIMEM:",  "AS_LOMEM:",  "AS_ONERR",  "AS_RESUME",  "AS_RECALL",
    "AS_STORE",   "AS_SPEED=",  "AS_LET",    "AS_GOTO",    "AS_RUN",
    "AS_IF",      "AS_RESTORE", "&",         "AS_GOSUB",   "AS_RETURN",
    "AS_REM",     "AS_STOP",    "ON",        "AS_WAIT",    "AS_LOAD",
    "AS_SAVE",    "AS_DEF",     "AS_POKE",   "AS_PRINT",   "AS_CONT",
    "AS_LIST",    "AS_CLEAR",   "AS_GET",    "AS_NEW",     "TAB(",
    "TO",         "FN",         "SPC(",      "THEN",       "AT",
    "NOT",        "AS_STEP",    "+",         "-",          "*",
    "/",          "^",          "AND",       "AS_OR",      ">",
    "=",          "<",          "AS_SGN",    "AS_INT",     "AS_ABS",
    "AS_USR",     "AS_FRE",     "SCRN(",     "AS_PDL",     "AS_POS",
    "AS_SQR",     "AS_RND",     "AS_LOG",    "AS_EXP",     "AS_COS",
    "AS_SIN",     "AS_TAN",     "AS_ATN",    "AS_PEEK",    "AS_LEN",
    "AS_STR$",    "AS_VAL",     "AS_ASC",    "CHR$",       "AS_LEFT$",
    "RIGHT$",     "MID$"};

constexpr char kEmptyTokenName[] = "";

} // namespace

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_TOKEN_NAME_TABLE (inclusive) .. AS_ERROR_MESSAGES (exclusive)
// Name normalization: minimal suffix required for C++ overload disambiguation.
ApplesoftDualPointer<const std::uint8_t>
AS_TOKEN_NAME_TABLE_ptr(std::size_t index) {
  if (index >= kTokenNameTable.size()) {
    return ApplesoftDualPointer<const std::uint8_t>::native(
        reinterpret_cast<const std::uint8_t *>(kEmptyTokenName));
  }

  return ApplesoftDualPointer<const std::uint8_t>::native(
      reinterpret_cast<const std::uint8_t *>(kTokenNameTable[index]));
}

std::string_view
AS_TOKEN_NAME_TABLE(ApplesoftDualPointer<const std::uint8_t> token_ptr) {
  if (!token_ptr.isNative()) {
    return {};
  }

  const std::uint8_t *bytes = token_ptr.nativePointer();
  if (bytes == nullptr || bytes[0] == 0u) {
    return {};
  }

  const char *text = reinterpret_cast<const char *>(bytes);
  return std::string_view(text, std::strlen(text));
}

std::string_view AS_TOKEN_NAME_TABLE(std::size_t index) {
  return AS_TOKEN_NAME_TABLE(AS_TOKEN_NAME_TABLE_ptr(index));
}

} // namespace applesoft::asm_port
