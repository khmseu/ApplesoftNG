// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: TOKEN_NAME_TABLE (inclusive) .. ERROR_MESSAGES (exclusive)
// Name normalization: none
//
// The original ROM stores token names as a packed character stream with bit 7
// set on the final character of each token. This port exposes the same table
// semantics as decoded token text indexed by (token - 0x80).

#include "core/asm_port_token_name_table.hpp"

#include <array>

namespace applesoft::asm_port {

std::string_view TOKEN_NAME_TABLE(std::size_t index) {
    static constexpr std::array<std::string_view, 107> table = {
        "END", "FOR", "NEXT", "DATA", "INPUT", "DEL", "DIM", "READ",
        "GR", "TEXT", "PR#", "IN#", "CALL", "PLOT", "HLIN", "VLIN",
        "HGR2", "HGR", "HCOLOR=", "HPLOT", "DRAW", "XDRAW", "HTAB", "HOME",
        "ROT=", "SCALE=", "SHLOAD", "TRACE", "NOTRACE", "NORMAL", "INVERSE", "FLASH",
        "COLOR=", "POP", "VTAB", "HIMEM:", "LOMEM:", "ONERR", "RESUME", "RECALL",
        "STORE", "SPEED=", "LET", "GOTO", "RUN", "IF", "RESTORE", "&",
        "GOSUB", "RETURN", "REM", "STOP", "ON", "WAIT", "LOAD", "SAVE",
        "DEF", "POKE", "PRINT", "CONT", "LIST", "CLEAR", "GET", "NEW",
        "TAB(", "TO", "FN", "SPC(", "THEN", "AT", "NOT", "STEP",
        "+", "-", "*", "/", "^", "AND", "OR", ">", "=", "<",
        "SGN", "INT", "ABS", "USR", "FRE", "SCRN(", "PDL", "POS",
        "SQR", "RND", "LOG", "EXP", "COS", "SIN", "TAN", "ATN",
        "PEEK", "LEN", "STR$", "VAL", "ASC", "CHR$", "LEFT$", "RIGHT$", "MID$"
    };

    return table[index];
}

} // namespace applesoft::asm_port
