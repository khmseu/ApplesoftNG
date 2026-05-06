#include "core/asm_port_error.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_inlin2.hpp"
#include "core/asm_port_qt_error.hpp"

#include <string_view>

namespace applesoft::asm_port {
namespace {

constexpr std::uint8_t RESTART_PROMPT = ']' | 0x80u;

bool isDigit(std::uint8_t ch) {
    return ch >= '0' && ch <= '9';
}

std::uint8_t CHRGET() {
    // TODO(asm-port): read the next character from the current input buffer.
    return 0;
}

void SetTextPointer(std::uint8_t lo, std::uint8_t hi) {
    // TODO(asm-port): update the TXTPTR pointer into the input buffer.
    (void)lo;
    (void)hi;
}

void ClearErrFlag() {
    // TODO(asm-port): clear the Applesoft ERRFLG state.
}

void MarkDirectMode() {
    // TODO(asm-port): record the current execution mode as direct.
}

void LINGET() {
    // TODO(asm-port): consume the line number prefix from the input buffer.
}

bool FNDLIN() {
    // TODO(asm-port): search for an existing program line matching the current line number.
    return false;
}

void DeleteExistingLine() {
    // TODO(asm-port): delete the existing numbered line and shift later lines down.
}

void InsertNewLine() {
    // TODO(asm-port): make room and copy the new numbered line into the program listing.
}

void PARSE_INPUT_LINE() {
    // TODO(asm-port): parse the current input line and prepare it for execution.
}

void TRACE_() {
    // TODO(asm-port): execute the parsed input line or trace it in the interpreter.
}

void HandleNumberedLine() {
    LINGET();
    PARSE_INPUT_LINE();

    if (FNDLIN()) {
        DeleteExistingLine();
    }

    InsertNewLine();
    FIX_LINKS();
}

} // namespace

void ERROR(std::uint8_t error_code_offset) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ERROR (inclusive) .. PRINT_ERROR_LINNUM (exclusive)
    // Name normalization: none (assembler label ERROR kept verbatim).

    if (IsOnErr()) {
        HANDLERR();
        return;
    }

    CRDO();
    OUTQUES();
    STROUT(ERROR_MESSAGES(error_code_offset));
    STKINI();
    PRINT_ERROR_LINNUM();
}

void PRINT_ERROR_LINNUM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PRINT_ERROR_LINNUM (inclusive) .. RESTART (exclusive)
    // Name normalization: none (assembler label PRINT_ERROR_LINNUM kept verbatim).

    STROUT(QT_ERROR(QT_ERROR_INDEX));

    if (IsDirectMode()) {
        RESTART();
        return;
    }

    INPRT();
    RESTART();
}

void RESTART() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RESTART (inclusive)
    // Name normalization: none (assembler label RESTART kept verbatim).

    CRDO();
    const Inlin2Result inlin2 = INLIN2(RESTART_PROMPT);
    SetTextPointer(inlin2.x, inlin2.y);
    ClearErrFlag();

    const std::uint8_t firstChar = CHRGET();
    if (firstChar == 0) {
        RESTART();
        return;
    }

    MarkDirectMode();

    if (isDigit(firstChar)) {
        HandleNumberedLine();
        return;
    }

    PARSE_INPUT_LINE();
    TRACE_();
}

namespace {

struct LineAddress {
    std::uint8_t lo = 0;
    std::uint8_t hi = 0;
};

std::uint16_t ToWord(LineAddress address) {
    return static_cast<std::uint16_t>(address.hi) << 8 | address.lo;
}

LineAddress FromWord(std::uint16_t value) {
    return LineAddress{static_cast<std::uint8_t>(value & 0xff), static_cast<std::uint8_t>(value >> 8)};
}

void SETPTRS() {
    // TODO(asm-port): initialize the program listing pointers before FIX_LINKS scans the program.
}

LineAddress GetTextTableAddress() {
    // TODO(asm-port): return the current TXTTAB pointer into the program listing.
    return LineAddress{};
}

bool IsEndOfProgram(LineAddress current) {
    // The original FIX_LINKS loop terminates by jumping to RESTART after processing
    // the final program line. A zero address is the safest sentinel for now.
    return current.lo == 0 && current.hi == 0;
}

std::uint8_t ReadProgramByte(LineAddress base, std::uint8_t offset) {
    // TODO(asm-port): read a byte from the program memory buffer at base+offset.
    (void)base;
    (void)offset;
    return 0;
}

LineAddress AdvanceToNextLine(LineAddress current) {
    // The original FIX_LINKS routine scans from the current line until it finds the
    // end-of-line marker, then computes the address of the next line.
    std::uint8_t offset = 4;
    while (ReadProgramByte(current, offset) != 0) {
        ++offset;
    }

    const std::uint16_t nextAddress = ToWord(current) + static_cast<std::uint16_t>(offset) + 1u;
    return FromWord(nextAddress);
}

void WriteForwardPointer(LineAddress current, LineAddress next) {
    // TODO(asm-port): write the low/high bytes of 'next' into the current line's forward-pointer header.
    (void)current;
    (void)next;
}

} // namespace

void FIX_LINKS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIX_LINKS (inclusive) .. INLIN (exclusive)
    // Name normalization: none (assembler label FIX_LINKS kept verbatim).

    SETPTRS();

    LineAddress current = GetTextTableAddress();
    while (true) {
        if (IsEndOfProgram(current)) {
            RESTART();
            return;
        }

        const LineAddress next = AdvanceToNextLine(current);
        WriteForwardPointer(current, next);
        current = next;
    }
}

void CRDO() {
    // TODO(asm-port): print a carriage return or perform monitor return behavior.
}

void STROUT(std::string_view text) {
    // TODO(asm-port): print the given string to the Applesoft console.
    (void)text;
}

void INPRT() {
    // TODO(asm-port): print the current line number when running a program.
}

void STKINI() {
    // TODO(asm-port): restore stack/frame pointers and clean up after error printing.
}

void HANDLERR() {
    // TODO(asm-port): transfer control to the ON ERR handler.
}

bool IsOnErr() {
    // TODO(asm-port): inspect the Applesoft ERRFLG state.
    return false;
}

bool IsDirectMode() {
    // TODO(asm-port): return true when the interpreter is in direct mode.
    return false;
}

void OUTQUES() {
    // TODO(asm-port): print the question mark prompt after an error.
}

} // namespace applesoft::asm_port
