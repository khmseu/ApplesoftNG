// The original ROM stores token names as a packed character stream with bit 7
// set on the final character of each token. This port exposes the same table
// semantics as decoded token text indexed by (token - 0x80).

#include "core/asm_port_token_name_table.hpp"

#include <array>

namespace applesoft::asm_port {

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_TOKEN_NAME_TABLE (inclusive) .. AS_ERROR_MESSAGES (exclusive)
// Name normalization: none (assembler label AS_TOKEN_NAME_TABLE kept verbatim).
std::string_view AS_TOKEN_NAME_TABLE(std::size_t index) {
  static constexpr std::array<std::string_view, 107> table = {
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

  return table[index];
}

} // namespace applesoft::asm_port
