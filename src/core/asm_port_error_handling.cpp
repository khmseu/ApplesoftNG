#include "core/asm_port_error_handling.hpp"

#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_print.hpp"

namespace applesoft::asm_port {

// Helper status functions (often found in asm_port_core.cpp or asm_port_error.cpp)
extern std::uint8_t gPendingErrorCode;
extern std::uint8_t gJerErrorCode;

bool IsOnErr();
bool IsDirectMode();
void OUTQUES();
void CRDO();
void STKINI();
void HANDLERR();
void INPRT();
void RESTART();

void ERROR(std::uint8_t error_code_offset) {
    // Source: applesoft.o65.lst:1337
    
    gPendingErrorCode = error_code_offset;

    if (IsOnErr()) {
        HANDLERR();
        return;
    }

    CRDO();
    OUTQUES();
    
    // Print the message from ERROR_MESSAGES
    STROUT(ERROR_MESSAGES(error_code_offset));
    
    STKINI();
    PRINT_ERROR_LINNUM();
}

std::uint8_t MEMERR() {
    // Source: applesoft.o65.lst:1329
    ERROR(ERR_MEMFULL);
    return ERR_MEMFULL;
}

void PRINT_ERROR_LINNUM() {
    // Source: applesoft.o65.lst:1356
    
    // QT_ERROR(0) is " ERROR" + BELL
    PRINT_ERROR_LINNUM(QT_ERROR(QT_ERROR_INDEX));
}

void PRINT_ERROR_LINNUM(std::string_view prefix) {
    STROUT(prefix);

    if (!IsDirectMode()) {
        INPRT(); // Prints " IN " + line number
    }

    RESTART();
}

void SYNERR() {
    // Source: applesoft.o65.lst:1404
    ERROR(ERR_SYNTAX);
}

void JER() {
    ERROR(gJerErrorCode);
}

void IQERR() {
    // Source: applesoft.o65.lst:3621
    gJerErrorCode = ERR_ILLQTY;
    JER();
}

} // namespace applesoft::asm_port
