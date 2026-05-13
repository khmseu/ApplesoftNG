#include "core/asm_port_error.hpp"
#include "core/asm_port_print.hpp"
#include "platform/asm_port_outdo.hpp"
#include "core/asm_port_error_messages.hpp"
#include "core/asm_port_chkmem.hpp"
#include "core/asm_port_gtforpnt.hpp"
#include "core/asm_port_reason.hpp"
#include "core/io_ports.hpp"
#include "core/applesoft_variables.hpp"
#include "core/asm_port_qt_error.hpp"
#include "core/asm_port_token_address_table.hpp"
#include "core/asm_port_token_name_table.hpp"
#include "core/asm_port_mathtbl.hpp"

#include <array>
#include <optional>
#include <string_view>

namespace applesoft::asm_port {

std::uint8_t ReadZeroPageByte(std::uint8_t address);
void WriteZeroPageByte(std::uint8_t address, std::uint8_t value);
void WriteZeroPageWord(std::uint8_t address, std::uint16_t value);
std::uint16_t ReadZeroPageWord(std::uint8_t address);
constexpr std::uint8_t add_u8(std::uint8_t lhs, std::uint8_t rhs) {
    return static_cast<std::uint8_t>(lhs + rhs);
}
void SetStackPointer(std::uint8_t value);
void PushByteToStack(std::uint8_t value);

void SCRTCH();
void RESTART();
void CRDO();
void FRMNUM();
void GETADR();
std::uint8_t MEMERR();
void CLEARC();
std::uint8_t GETBYT();
void IQERR();

void MON_SETCOL(std::uint8_t color);
void MON_TABV(std::uint8_t row_zero_based);
void MON_INPORT(std::uint8_t slot);
void MON_OUTPORT(std::uint8_t slot);
void MON_PLOT(std::uint8_t y, std::uint8_t x);
void MON_HLINE(std::uint8_t y, std::uint8_t right, std::uint8_t left);
void MON_VLINE(std::uint8_t x, std::uint8_t top);
void MON_HOME();
void MON_SETTXT();
void MON_SETGR();
void NORMAL();
void INVERSE();
void FLASH();
void HOME();
void GR();
void TEXT();
void HTAB();
void HCOLOR();
void HPLOT();
void DRAW();
void XDRAW();
void COLOR();
void VTAB();
void HIMEM();
void LOMEM();
void SPEED();
std::uint8_t PLOTFNS();
std::uint8_t LINCOOR();


void CONTROL_C_TYPED();
bool SETPTRS();
void COLD_START();

std::uint8_t ReadProgramByte(std::uint16_t address);
void WriteProgramByte(std::uint16_t address, std::uint8_t value);

struct ProgramPointer {
    std::uint16_t address = 0;

    std::uint8_t read(std::uint16_t offset = 0) const {
        return ReadProgramByte(static_cast<std::uint16_t>(address + offset));
    }

    void write(std::uint8_t value, std::uint16_t offset = 0) const {
        WriteProgramByte(static_cast<std::uint16_t>(address + offset), value);
    }

    ProgramPointer advanced(std::uint16_t offset) const {
        return ProgramPointer{static_cast<std::uint16_t>(address + offset)};
    }
};

std::uint8_t ReadStackPointer();
void PopReturnAddress();
void PushByteToStack(std::uint8_t value);
void PushTextPointerAddress();
void PushCurrentLineNumber();
void PushTokenTo(std::uint8_t token);
void ApplyFacSign();
void SetBranchTargetToSTEP();
void LOAD_FAC_FROM_YA();
void SYNCHR(std::uint8_t expected);
void CHKNUM();
void CHKSTR();
void FRMNUM();
void SIGN();
void FRM_STACK_2();
void FRM_STACK_3();
void MON_WRITE();
void MON_READ();
void VARTIO();
void PROGIO();
std::uint8_t CurrentStatementChar();
void ADDON(std::uint8_t offset);
std::uint8_t DATAN();
void GOEND();
bool IsEndOfLineAtTextPointer();
bool IsEndOfProgramAtTextPointer();
std::uint16_t ReadLineNumberFromTextPointer();
void AdvanceTextPointerToNextLine();
void DATA();
bool IsRunningMode();
bool IsTraceEnabled();
bool IsStatementEndOfParsedInput();
std::uint8_t REMN();
bool FL1(std::uint16_t startAddress);
bool FL1(std::uint8_t startLo, std::uint8_t startHi);
std::uint8_t PopByteFromStack();
std::uint8_t PeekTopControlTokenAfterGTFORPNT();
void PULL3();
void RTS_5();
void OUTSP();
void EXECUTE_STATEMENT();
void EXECUTE_STATEMENT_1();
void PushForPntFrame();

std::uint8_t CHRGOT();
void LINGET();
void SYNERR();
void LINPRT();
void INPRT();
void PrintDecimalUnsigned(std::uint16_t value);
// void OUTDO();
std::uint8_t MEMERR();

void LET();
void LET2(std::uint8_t savedValTypPlus1);
void PUTSTR();
void IF();
void REM();
void IF_TRUE();
void ONGOTO();
void FOR();
void NEXT();
void STEP();
void NEWSTT();
void TRACE_();
void FRMEVL();
std::uint8_t GETBYT();
std::uint8_t GTNUM();
std::uint8_t COMBYTE();
void GETADR();
void GO_TO_LINE();
void GOTO();
void PEEK();
void POKE();
void WAIT();
void RTS_10();
// std::uint16_t PTRGET();
bool CHKVAL(std::uint8_t savedValTyp);
void ERRDIR();
void ROUND_FAC();
void AYINT();
void SETFOR();
void STRINI(std::uint8_t length);
void MOVINS();
bool FRETMS(std::uint16_t descriptorAddress);
std::uint8_t FRETMP(std::uint16_t descriptorAddress);
void GARBAG();
void SCREEN();
void UNARY();
void OR();
void ANDOP();
void FALSE();
void TRUE();
void RELOPS();
void STRCMP();
void NUMCMP();
void CMPDONE();
void CHKCOM();
void CHKOPN();
void CHKCLS();
void PDL();
void NXDIM();
void DIM();
void PTRGET2();
void PTRGET3();
void BADNAM();
void NAMOK();
void BASIC();
void BASIC2();
void PTRGET4();
bool ISLETC();
void NAME_NOT_FOUND();
void C_ZERO();
void MAKE_NEW_VARIABLE();
void FNC_();
void PARCHK();
void STORE_FACDB_YX_ROUNDED();
void SET_VARPNT_AND_YA();
void GETARY();
void GETARY2();
void NEG32768();
void MAKINT();
void MKINT();
void MI1();
void MI2();
void ARRAY();
void SUBERR();
void IQERR();
void JER();
void USE_OLD_ARRAY();
void MAKE_NEW_ARRAY();
void FIND_ARRAY_ELEMENT();
void FAE_1();
void GSE();
void GME();
void SYNERR();
void STOP_impl(bool shouldPrintBreak);

void PRINT_ERROR_LINNUM();
void PRINT_ERROR_LINNUM(std::string_view prefix);


namespace {


std::int8_t gNumericCompareResult = 0;
bool gNumericCompareCarry = false;
std::uint8_t gFloatInput = 0;

// TODO(asm-port): port MON_SCRN monitor handler.
std::uint8_t MON_SCRN(std::uint8_t /*row*/, std::uint8_t /*column*/) {
    return 0;
}

// TODO(asm-port): port FCOMP label.
std::int8_t FCOMP(std::uint16_t /*argAddress*/) {
    return 0;
}

// TODO(asm-port): port FREFAC label.
// void FREFAC() {}

// TODO(asm-port): port FLOAT label.
void FLOAT() {}


void GIVAYF(std::int16_t value);

// TODO(asm-port): port QINT label.
void QINT() {}

// TODO(asm-port): port FLOAT_1 label.
void FLOAT_1(std::uint8_t exponent) {
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exponent);
}

std::uint8_t gJerErrorCode = ERR_SYNTAX;
std::uint8_t gPendingErrorCode = ERR_SYNTAX;
constexpr std::uint8_t kNEG32768Data[4] = {0x90u, 0x80u, 0x00u, 0x00u};
constexpr std::uint8_t kCZeroData[2] = {0x00u, 0x00u};

// TODO(asm-port): compare temporary ARG and FAC strings and return -1/0/1.
std::int8_t CompareArgAndFacStrings() {
    return 0;
}

} // namespace

// TODO(asm-port): port MON_PREAD monitor paddle reader.
std::uint8_t MON_PREAD() {
    return 0;
}

void SetPendingErrorCode(std::uint8_t errorCode) {
    gPendingErrorCode = errorCode;
}

void SNGFLT(std::uint8_t value) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SNGFLT (inclusive) .. ERRDIR (exclusive)
    // Name normalization: none (assembler label SNGFLT kept verbatim).

    GIVAYF(static_cast<std::int16_t>(value));
}


void ERROR(std::uint8_t error_code_offset) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ERROR (inclusive) .. PRINT_ERROR_LINNUM (exclusive)
    // Name normalization: none (assembler label ERROR kept verbatim).

    gPendingErrorCode = error_code_offset;

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

// void OUTDO() {
//     // TODO(asm-port): write the current output character from the Applesoft line
//     // printer to the console.
// }

void PopReturnAddress();
void PushByteToStack(std::uint8_t value);
void PushWordToStack(std::uint16_t value);
std::uint16_t PopWordFromStack();
void PushTextPointerAddress();
void PushCurrentLineNumber();
void PushTokenTo(std::uint8_t token);


void FRMEVL() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FRMEVL (inclusive) .. FRM_STACK_1 (exclusive)
    // Name normalization: FRMEVL_1/2 and related sublabels are modeled inline.
    //
    // Incremental port note:
    // This now includes the FRM_RECURSE..FRM_STACK_1 tranche by modeling the
    // recursive precedence walk and the stacked-LHS frame handoff to ARG/CPRMASK.

    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;
    constexpr std::uint8_t kCPRTYP = ApplesoftVariables::ZP_CPRTYP;
    constexpr std::uint8_t kCPRMASK = ApplesoftVariables::ZP_CPRMASK;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;
    constexpr std::uint8_t kFAC_SIGN = ApplesoftVariables::ZP_FAC_SIGN;
    constexpr std::uint8_t kARG = ApplesoftVariables::ZP_ARG;
    constexpr std::uint8_t kSGNCPR = ApplesoftVariables::ZP_STRNG1; // SGNCPR shares $AB with STRNG1.
    constexpr std::uint8_t kSTACK_ROOM_BYTES = 1u;
    constexpr std::uint8_t kTOKEN_PLUS = 0xc8u;
    constexpr std::uint8_t kTOKEN_GREATER = 0xcfu;
    constexpr std::uint8_t kTOKEN_EQUAL = 0xd0u;
    constexpr std::uint8_t kTOKEN_LESS = 0xd1u;

    const auto frmevl_eval = [&](auto&& self, std::uint8_t callerPrecedence, bool runEntryBackstep) -> void {
        // FRMEVL entry point only: back TXTPTR up one byte so FRM_ELEMENT starts
        // from the current token. Recursive FRMEVL_1 calls skip this backstep.
        if (runEntryBackstep) {
            const std::uint16_t txtptr = ReadZeroPageWord(kTXTPTR);
            WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(txtptr - 1u));
        }

        // FRMEVL_1 prologue: CHKMEM(1), FRM_ELEMENT.
        CHKMEMState chkmemState{};
        chkmemState.a = kSTACK_ROOM_BYTES;
        if (!CHKMEM(chkmemState).ok) {
            return;
        }
        UNARY();
        WriteZeroPageByte(kCPRTYP, 0u);

        while (true) {
            std::uint8_t token = CHRGOT();

            // FRMEVL_2 relational scan: absorb chains of <, =, >.
            while (token == kTOKEN_GREATER || token == kTOKEN_EQUAL || token == kTOKEN_LESS) {
                std::uint8_t mask = 0u;
                if (token == kTOKEN_GREATER) {
                    mask = 0x01u;
                } else if (token == kTOKEN_EQUAL) {
                    mask = 0x02u;
                } else {
                    mask = 0x04u;
                }

                const std::uint8_t existing = ReadZeroPageByte(kCPRTYP);
                if ((existing & mask) != 0u) {
                    SYNERR();
                    return;
                }

                WriteZeroPageByte(kCPRTYP, static_cast<std::uint8_t>(existing | mask));
                token = CHRGET();
            }

            MathTblEntry pendingEntry{};
            std::uint8_t cprtypForFrame = 0u;
            bool relationalPath = false;

            if (ReadZeroPageByte(kCPRTYP) != 0u) {
                // FRM_RELATIONAL: fold string-vs-numeric state into CPRTYP and
                // treat as MATHTBL M_REL for precedence dispatch.
                relationalPath = true;
                const std::uint8_t relFlags = ReadZeroPageByte(kCPRTYP);
                const bool facIsString = (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u;
                cprtypForFrame = static_cast<std::uint8_t>((relFlags << 1u) | (facIsString ? 1u : 0u));

                const std::uint16_t txtptr = ReadZeroPageWord(kTXTPTR);
                WriteZeroPageWord(kTXTPTR, static_cast<std::uint16_t>(txtptr - 1u));
                pendingEntry = MATHTBL(M_REL_IDX);
            } else {
                // NOTMATH/GOEX: stop when the next token is not an infix operator.
                if (token < kTOKEN_PLUS || token > kTOKEN_LESS) {
                    return;
                }

                // FRMEVL_2_3 special-case (+ with string FAC) is CAT in ROM.
                if (token == kTOKEN_PLUS && (ReadZeroPageByte(kVALTYP) & 0x80u) != 0u) {
                    // TODO(asm-port): route string concatenation to CAT label implementation.
                    return;
                }

                CHKNUM();

                const std::size_t mathIndex = static_cast<std::size_t>(token - kTOKEN_PLUS);
                if (mathIndex > M_REL_IDX) {
                    return;
                }
                pendingEntry = MATHTBL(mathIndex);
            }

            // FRM_PRECEDENCE_TEST/PREFNC: defer lower-or-equal precedence work
            // to the caller stack frame.
            if (callerPrecedence >= pendingEntry.precedence) {
                return;
            }

            // FRM_RECURSE (inclusive) .. FRM_STACK_1 (exclusive): recurse into
            // FRMEVL_1 while carrying pending operator/precedence state.
            const std::array<std::uint8_t, 5> lhsFac = {
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 0u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 1u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 2u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 3u)),
                ReadZeroPageByte(static_cast<std::uint8_t>(kFAC + 4u)),
            };
            const std::uint8_t lhsSign = ReadZeroPageByte(kFAC_SIGN);

            if (!relationalPath) {
                (void)CHRGET();
            }
            self(self, pendingEntry.precedence, false);

            // FRM_PERFORM_2 frame handoff: move stacked left operand to ARG and
            // synthesize CPRMASK/SGNCPR as if popped from the ROM expression stack.
            WriteZeroPageByte(kCPRMASK, static_cast<std::uint8_t>(cprtypForFrame >> 1u));
            for (std::uint8_t i = 0; i < lhsFac.size(); ++i) {
                WriteZeroPageByte(static_cast<std::uint8_t>(kARG + i), lhsFac[i]);
            }
            WriteZeroPageByte(static_cast<std::uint8_t>(kARG + 5u), lhsSign);
            WriteZeroPageByte(kSGNCPR, static_cast<std::uint8_t>(lhsSign ^ ReadZeroPageByte(kFAC_SIGN)));

            if (cprtypForFrame != 0u) {
                WriteZeroPageByte(kCPRTYP, cprtypForFrame);
            }

            if (pendingEntry.handler != nullptr) {
                pendingEntry.handler();
            }
        }
    };

    frmevl_eval(frmevl_eval, 0u, true);
}

// Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
// Labels: PEEK (inclusive) .. POKE (exclusive)
// Name normalization: none (assembler label PEEK kept verbatim).
std::uint16_t PTRGET() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET (inclusive) .. PTRGET2 (exclusive)
    // Name normalization: none (assembler label PTRGET kept verbatim).

    CHRGOT();
    WriteZeroPageByte(ApplesoftVariables::ZP_DIMFLG, 0u); // DIMFLG
    PTRGET3();
    return ReadZeroPageWord(ApplesoftVariables::ZP_VARPNT); // VARPNT
}

void SCREEN() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SCREEN (inclusive) .. UNARY (exclusive)
    // Name normalization: none (assembler label SCREEN kept verbatim).

    constexpr std::uint8_t kFIRST = ApplesoftVariables::ZP_FIRST;

    CHRGET();
    PLOTFNS();

    // PLOTFNS returns row in X and column in FIRST in ROM.
    const std::uint8_t row = ReadZeroPageByte(kFIRST);
    const std::uint8_t column = ReadZeroPageByte(kFIRST);
    const std::uint8_t color = MON_SCRN(row, column);

    SNGFLT(color);
    SYNCHR(static_cast<std::uint8_t>(')'));
}

void UNARY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: UNARY (inclusive) .. OR (exclusive)
    // Name normalization: none (assembler label UNARY kept verbatim).

    constexpr std::uint8_t kTOKEN_SCRN = 0xd7u;

    if (CHRGOT() == kTOKEN_SCRN) {
        // ROM branches back to SCREEN for SCRN(.
        SCREEN();
        return;
    }

    CHRGET();

    // TODO(asm-port): complete unary-function dispatch through UNFNC/JMPADRS.
    FRMEVL();
    CHKNUM();
}

void OR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: OR (inclusive) .. ANDOP (exclusive)
    // Name normalization: none (assembler label OR kept verbatim).

    constexpr std::uint8_t kARG = ApplesoftVariables::ZP_ARG;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;

    if ((ReadZeroPageByte(kARG) | ReadZeroPageByte(kFAC)) != 0u) {
        TRUE();
        return;
    }

    // Fall-through in ROM from OR to ANDOP.
    ANDOP();
}

void ANDOP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ANDOP (inclusive) .. FALSE (exclusive)
    // Name normalization: none (assembler label ANDOP kept verbatim).

    constexpr std::uint8_t kARG = ApplesoftVariables::ZP_ARG;
    constexpr std::uint8_t kFAC = ApplesoftVariables::ZP_FAC;

    if (ReadZeroPageByte(kARG) == 0u || ReadZeroPageByte(kFAC) == 0u) {
        FALSE();
        return;
    }

    // Fall-through in ROM from ANDOP to TRUE.
    TRUE();
}

void FALSE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FALSE (inclusive) .. TRUE (exclusive)
    // Name normalization: none (assembler label FALSE kept verbatim).

    SNGFLT(0u);
}

void TRUE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: TRUE (inclusive) .. RELOPS (exclusive)
    // Name normalization: none (assembler label TRUE kept verbatim).

    SNGFLT(1u);
}

void RELOPS() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: RELOPS (inclusive) .. STRCMP (exclusive)
    // Name normalization: none (assembler label RELOPS kept verbatim).

    constexpr std::uint8_t kCPRTYP = ApplesoftVariables::ZP_CPRTYP;
    constexpr std::uint16_t kARG = ApplesoftVariables::ZP_ARG;

    const std::uint8_t compareTypeFlags = ReadZeroPageByte(kCPRTYP);
    if (CHKVAL(compareTypeFlags)) {
        // Carry set in ROM indicates string compare path.
        STRCMP();
        return;
    }

    gNumericCompareResult = FCOMP(kARG);
    gNumericCompareCarry = gNumericCompareResult >= 0;
    NUMCMP();
}

void STRCMP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: STRCMP (inclusive) .. NUMCMP (exclusive)
    // Name normalization: none (assembler label STRCMP kept verbatim).

    constexpr std::uint8_t kVALTYP = ApplesoftVariables::ZP_VALTYP;
    constexpr std::uint8_t kCPRTYP = ApplesoftVariables::ZP_CPRTYP;

    WriteZeroPageByte(kVALTYP, 0u);
    WriteZeroPageByte(kCPRTYP, static_cast<std::uint8_t>(ReadZeroPageByte(kCPRTYP) - 1u));

    FREFAC();
    (void)FRETMP(ReadZeroPageWord(ApplesoftVariables::ZP_DSCPTR));

    gNumericCompareResult = CompareArgAndFacStrings();
    gNumericCompareCarry = gNumericCompareResult >= 0;
    NUMCMP();
}

void NUMCMP() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NUMCMP (inclusive) .. CMPDONE (exclusive)
    // Name normalization: none (assembler label NUMCMP kept verbatim).

    // ROM reaches CMPDONE with C set only when compare result was negative.
    gNumericCompareCarry = (gNumericCompareResult < 0);
    CMPDONE();
}

void CMPDONE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: CMPDONE (inclusive) .. PDL (exclusive)
    // Name normalization: none (assembler label CMPDONE kept verbatim).

    constexpr std::uint8_t kCPRMASK = ApplesoftVariables::ZP_CPRMASK;

    std::int16_t x = static_cast<std::int16_t>(gNumericCompareResult) + 1;
    if (x < 0) {
        x = 0;
    }

    std::uint8_t a = static_cast<std::uint8_t>(x & 0xff);
    a = static_cast<std::uint8_t>((a << 1) | (gNumericCompareCarry ? 1u : 0u));
    a = static_cast<std::uint8_t>(a & ReadZeroPageByte(kCPRMASK));

    gFloatInput = (a == 0u) ? 0u : 1u;
    SNGFLT(gFloatInput);
    FLOAT();
}

void NXDIM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NXDIM (inclusive) .. DIM (exclusive)
    // Name normalization: none (assembler label NXDIM kept verbatim).

    CHKCOM();
    DIM();
}

void DIM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DIM (inclusive) .. PTRGET (exclusive)
    // Name normalization: none (assembler label DIM kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_DIMFLG, 1u); // DIMFLG non-zero when called from DIM.
    PTRGET2();

    if (CHRGOT() != 0u) {
        NXDIM();
    }
}

void PTRGET2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET2 (inclusive) .. PTRGET3 (exclusive)
    // Name normalization: none (assembler label PTRGET2 kept verbatim).

    PTRGET3();
}

void PTRGET3() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET3 (inclusive) .. BADNAM (exclusive)
    // Name normalization: none (assembler label PTRGET3 kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_VARNAM, CHRGOT()); // VARNAM low byte
    CHRGOT();
    if (!ISLETC()) {
        BADNAM();
        return;
    }

    NAMOK();
}

void BADNAM() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BADNAM (inclusive) .. NAMOK (exclusive)
    // Name normalization: none (assembler label BADNAM kept verbatim).

    SYNERR();
}

void NAMOK() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NAMOK (inclusive) .. BASIC (exclusive)
    // Name normalization: none (assembler label NAMOK kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP, 0u); // VALTYP
    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP_PLUS_1, 0u); // VALTYP+1
    PTRGET4();
}

void BASIC() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BASIC (inclusive) .. BASIC2 (exclusive)
    // Name normalization: none (assembler label BASIC kept verbatim).

    COLD_START();
}

void BASIC2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: BASIC2 (inclusive) .. PTRGET4 (exclusive)
    // Name normalization: none (assembler label BASIC2 kept verbatim).

    RESTART();
}

void PTRGET4() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: PTRGET4 (inclusive) .. ISLETC (exclusive)
    // Name normalization: none (assembler label PTRGET4 kept verbatim).

    std::uint8_t current = CHRGET();
    std::uint8_t secondChar = 0u;

    if ((current >= '0') && (current <= '9')) {
        secondChar = current;
        do {
            current = CHRGET();
        } while ((current >= '0') && (current <= '9'));
    } else {
        if (ISLETC()) {
            secondChar = current;
            do {
                current = CHRGET();
            } while (((current >= '0') && (current <= '9')) || ISLETC());
        }
    }

    if (current == static_cast<std::uint8_t>('$')) {
        WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP, 0xffu); // VALTYP string
        current = CHRGET();
    } else if (current == static_cast<std::uint8_t>('%')) {
        if ((ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) & 0x80u) != 0u) {
            BADNAM();
            return;
        }

        WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP_PLUS_1, 0x80u); // integer mode
        WriteZeroPageByte(ApplesoftVariables::ZP_VARNAM, static_cast<std::uint8_t>(ReadZeroPageByte(ApplesoftVariables::ZP_VARNAM) | 0x80u));
        secondChar = static_cast<std::uint8_t>(secondChar | 0x80u);
        current = CHRGET();
    }

    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_VARNAM, 1u), secondChar); // VARNAM+1

    const std::uint8_t subflg = ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG);
    if (subflg == 0u && current == static_cast<std::uint8_t>('(')) {
        ARRAY();
        return;
    }

    WriteZeroPageByte(ApplesoftVariables::ZP_SUBFLG, 0u); // clear SUBFLG
    NAME_NOT_FOUND();
}

bool ISLETC() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ISLETC (inclusive) .. NAME_NOT_FOUND (exclusive)
    // Name normalization: none (assembler label ISLETC kept verbatim).

    const std::uint8_t ch = CHRGOT();
    return ch >= static_cast<std::uint8_t>('A') && ch <= static_cast<std::uint8_t>('Z');
}

void NAME_NOT_FOUND() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NAME_NOT_FOUND (inclusive) .. C_ZERO (exclusive)
    // Name normalization: none (assembler label NAME_NOT_FOUND kept verbatim).

    // TODO(asm-port): preserve FRM_VARIABLE_CALL return-address special case.
    MAKE_NEW_VARIABLE();
}

void C_ZERO() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: C_ZERO (inclusive) .. MAKE_NEW_VARIABLE (exclusive)
    // Name normalization: none (assembler label C_ZERO kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_RESULT, kCZeroData[0]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 1u), kCZeroData[1]);
}

void MAKE_NEW_VARIABLE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKE_NEW_VARIABLE (inclusive) .. SET_VARPNT_AND_YA (exclusive)
    // Name normalization: none (assembler label MAKE_NEW_VARIABLE kept verbatim).

    const std::uint16_t arytab = ReadZeroPageWord(ApplesoftVariables::ZP_ARYTAB);
    WriteZeroPageWord(ApplesoftVariables::ZP_LOWTR, arytab); // LOWTR <- ARYTAB

    // TODO(asm-port): port BLTU movement of array block.
    SET_VARPNT_AND_YA();
}

void SET_VARPNT_AND_YA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SET_VARPNT_AND_YA (inclusive) .. GETARY (exclusive)
    // Name normalization: none (assembler label SET_VARPNT_AND_YA kept verbatim).

    const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    WriteZeroPageWord(ApplesoftVariables::ZP_VARPNT, lowtr.advanced(2u).address); // VARPNT
}

void GETARY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GETARY (inclusive) .. GETARY2 (exclusive)
    // Name normalization: none (assembler label GETARY kept verbatim).

    GETARY2();
}

void GETARY2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GETARY2 (inclusive) .. NEG32768 (exclusive)
    // Name normalization: none (assembler label GETARY2 kept verbatim).

    const std::uint8_t numDim = ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM);
    const ProgramPointer lowtr{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    const std::uint16_t arypntOffset = static_cast<std::uint16_t>(numDim * 2u) + 5u;
    WriteZeroPageWord(ApplesoftVariables::ZP_ARYPNT, lowtr.advanced(arypntOffset).address);
}

void NEG32768() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: NEG32768 (inclusive) .. MAKINT (exclusive)
    // Name normalization: none (assembler label NEG32768 kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_RESULT, kNEG32768Data[0]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 1u), kNEG32768Data[1]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 2u), kNEG32768Data[2]);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_RESULT, 3u), kNEG32768Data[3]);
}

void MAKINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKINT (inclusive) .. MKINT (exclusive)
    // Name normalization: none (assembler label MAKINT kept verbatim).

    CHRGET();
    FRMNUM();
    MKINT();
}

void MKINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MKINT (inclusive) .. AYINT (exclusive)
    // Name normalization: none (assembler label MKINT kept verbatim).

    if ((ReadZeroPageByte(ApplesoftVariables::ZP_FAC_SIGN) & 0x80u) != 0u) {
        MI1();
        return;
    }

    AYINT();
}

void ROUND_FAC() {
    // TODO(asm-port): port ROUND_FAC label.
}

void AYINT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: AYINT (inclusive) .. MI1 (exclusive)
    // Name normalization: none (assembler label AYINT kept verbatim).

    if (ReadZeroPageByte(ApplesoftVariables::ZP_FAC) < 0x90u) {
        MI2();
        return;
    }

    NEG32768();
    if (FCOMP(0x0062u) != 0) {
        MI1();
        return;
    }

    MI2();
}

void MI1() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MI1 (inclusive) .. MI2 (exclusive)
    // Name normalization: none (assembler label MI1 kept verbatim).

    IQERR();
}

void MI2() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MI2 (inclusive) .. ARRAY (exclusive)
    // Name normalization: none (assembler label MI2 kept verbatim).

    QINT();
}

void ARRAY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: ARRAY (inclusive) .. SUBERR (exclusive)
    // Name normalization: none (assembler label ARRAY kept verbatim).

    if (ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) != 0u) {
        USE_OLD_ARRAY();
        return;
    }

    // TODO(asm-port): complete subscript-list parsing and array-table scan.
    MAKE_NEW_ARRAY();
}

void SUBERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: SUBERR (inclusive) .. IQERR (exclusive)
    // Name normalization: none (assembler label SUBERR kept verbatim).

    gJerErrorCode = ERR_BADSUBS;
    JER();
}

void IQERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: IQERR (inclusive) .. JER (exclusive)
    // Name normalization: none (assembler label IQERR kept verbatim).

    gJerErrorCode = ERR_ILLQTY;
    JER();
}

void JER() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: JER (inclusive) .. USE_OLD_ARRAY (exclusive)
    // Name normalization: none (assembler label JER kept verbatim).

    ERROR(gJerErrorCode);
}

void USE_OLD_ARRAY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: USE_OLD_ARRAY (inclusive) .. MAKE_NEW_ARRAY (exclusive)
    // Name normalization: none (assembler label USE_OLD_ARRAY kept verbatim).

    if (ReadZeroPageByte(ApplesoftVariables::ZP_DIMFLG) != 0u) {
        gJerErrorCode = ERR_REDIMD;
        JER();
        return;
    }

    if (ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) == 0u) {
        GETARY();
        FIND_ARRAY_ELEMENT();
    }
}

void MAKE_NEW_ARRAY() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MAKE_NEW_ARRAY (inclusive) .. FIND_ARRAY_ELEMENT (exclusive)
    // Name normalization: none (assembler label MAKE_NEW_ARRAY kept verbatim).

    if (ReadZeroPageByte(ApplesoftVariables::ZP_SUBFLG) != 0u) {
        ERROR(ERR_NODATA);
        return;
    }

    GETARY();

    // TODO(asm-port): complete dynamic allocation, descriptor population, and zeroing.
    if (ReadZeroPageByte(ApplesoftVariables::ZP_DIMFLG) == 0u) {
        FIND_ARRAY_ELEMENT();
    }
}

void FIND_ARRAY_ELEMENT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FIND_ARRAY_ELEMENT (inclusive) .. FAE_1 (exclusive)
    // Name normalization: none (assembler label FIND_ARRAY_ELEMENT kept verbatim).

    WriteZeroPageByte(ApplesoftVariables::ZP_NUMDIM, ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM)); // TODO(asm-port): fetch #dims from descriptor pointer.
    WriteZeroPageWord(ApplesoftVariables::ZP_STRNG2, 0u); // STRNG2 accumulator
    FAE_1();
}

void FAE_1() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FAE_1 (inclusive) .. GSE (exclusive)
    // Name normalization: none (assembler label FAE_1 kept verbatim).

    if (ReadZeroPageByte(ApplesoftVariables::ZP_NUMDIM) == 0u) {
        return;
    }

    // TODO(asm-port): complete per-dimension bounds and offset accumulation.
    GSE();
}

void GSE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GSE (inclusive) .. GME (exclusive)
    // Name normalization: none (assembler label GSE kept verbatim).

    SUBERR();
}

void GME() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GME (inclusive) .. MULTIPLY_SUBSCRIPT (exclusive)
    // Name normalization: none (assembler label GME kept verbatim).

    (void)MEMERR();
}

std::uint16_t MULTIPLY_SUBS_1(std::uint8_t multiplierHigh);

std::uint16_t MULTIPLY_SUBSCRIPT(std::uint8_t descriptorOffset) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MULTIPLY_SUBSCRIPT (inclusive) .. MULTIPLY_SUBS_1 (exclusive)
    // Name normalization: none (assembler label MULTIPLY_SUBSCRIPT kept verbatim).
    // Load the 16-bit array-dimension multiplier from the LOWTR descriptor pointer.

    WriteZeroPageByte(ApplesoftVariables::ZP_INDEX, descriptorOffset);

    const ProgramPointer descriptor{ReadZeroPageWord(ApplesoftVariables::ZP_LOWTR)};
    WriteZeroPageByte(
        static_cast<std::uint8_t>(ApplesoftVariables::ZP_RESULT + 2u),
        descriptor.read(descriptorOffset));

    return MULTIPLY_SUBS_1(descriptor.read(static_cast<std::uint16_t>(descriptorOffset - 1u)));
}

std::uint16_t MULTIPLY_SUBS_1(std::uint8_t multiplierHigh) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: MULTIPLY_SUBS_1 (inclusive) .. FRE (exclusive)
    // Name normalization: none (assembler label MULTIPLY_SUBS_1 kept verbatim).
    // STRNG2 is dual-use elsewhere, but in this slice it is the 16-bit multiplicand.

    WriteZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_RESULT + 3u), multiplierHigh);
    WriteZeroPageByte(ApplesoftVariables::ZP_INDX, 16u);

    const std::uint16_t multiplier = ApplesoftVariables::makeWord(
        ReadZeroPageByte(static_cast<std::uint8_t>(ApplesoftVariables::ZP_RESULT + 2u)),
        multiplierHigh);

    std::uint16_t multiplicand = ReadZeroPageWord(ApplesoftVariables::ZP_STRNG2);
    std::uint16_t product = 0u;

    for (std::uint8_t bitsRemaining = 16u; bitsRemaining > 0u; --bitsRemaining) {
        if ((product & 0x8000u) != 0u) {
            GME();
            return product;
        }

        product = static_cast<std::uint16_t>(product << 1u);

        const bool nextBitSet = (multiplicand & 0x8000u) != 0u;
        multiplicand = static_cast<std::uint16_t>(multiplicand << 1u);
        WriteZeroPageWord(ApplesoftVariables::ZP_STRNG2, multiplicand);

        if (!nextBitSet) {
            continue;
        }

        if (product > static_cast<std::uint16_t>(0xffffu - multiplier)) {
            GME();
            return product;
        }

        product = static_cast<std::uint16_t>(product + multiplier);
    }

    WriteZeroPageByte(ApplesoftVariables::ZP_INDX, 0u);
    return product;
}

void FRE() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FRE (inclusive) .. GIVAYF (exclusive)
    // Name normalization: none (assembler label FRE kept verbatim).
    // FRETOP and STREND are one logical address pair each; model the subtraction
    // as one 16-bit free-space computation before floating the signed result.

    if (ReadZeroPageByte(ApplesoftVariables::ZP_VALTYP) != 0u) {
        (void)FREFAC();
    }

    GARBAG();

    const std::uint16_t fretop = ReadZeroPageWord(ApplesoftVariables::ZP_FRETOP);
    const std::uint16_t strend = ReadZeroPageWord(ApplesoftVariables::ZP_STREND);
    const std::uint16_t freeSpace = static_cast<std::uint16_t>(fretop - strend);
    GIVAYF(static_cast<std::int16_t>(freeSpace));
}

namespace {

void GIVAYF(std::int16_t value) {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: GIVAYF (inclusive) .. POS (exclusive)
    // Name normalization: none (assembler label GIVAYF kept verbatim).
    // The A/Y pair is one signed 16-bit integer on entry; represent it as one
    // C++ value instead of split low/high byte locals.

    const std::uint16_t rawValue = static_cast<std::uint16_t>(value);

    WriteZeroPageByte(ApplesoftVariables::ZP_VALTYP, 0u);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 1u), ApplesoftVariables::lowByte(rawValue));
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 2u), ApplesoftVariables::highByte(rawValue));
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 3u), 0u);
    WriteZeroPageByte(add_u8(ApplesoftVariables::ZP_FAC, 4u), 0u);

    FLOAT_1(0x90u);
}

} // namespace

void UNDFNC() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: UNDFNC (inclusive) .. DEF (exclusive)
    // Name normalization: none (assembler label UNDFNC kept verbatim).

    ERROR(ERR_UNDEFFUNC);
}

void DEF() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: DEF (inclusive) .. FNC_ (exclusive)
    // Name normalization: none (assembler label DEF kept verbatim).
    //
    // "DEF" STATEMENT
    // Parse: DEF FN name (arg) = expression
    // Stacks VARPNT, TXTPTR, and 5 bytes of FAC, then jumps to FNCDATA to store.

    // Parse "FN name"
    FNC_();
    
    // Error if in direct mode
    ERRDIR();
    
    // Require "("
    CHKOPN();
    
    // Set SUBFLG to flag DEF context for PTRGET
    constexpr std::uint8_t kSUBFLG = ApplesoftVariables::ZP_SUBFLG;
    WriteZeroPageByte(kSUBFLG, 0x80u);
    
    // Get pointer to argument variable
    PTRGET();
    
    // Argument must be numeric
    CHKNUM();
    
    // Require ")"
    CHKCLS();
    
    // Require "=" and advance past it
    SYNCHR(static_cast<std::uint8_t>(0xd0u));  // TOKEN_EQUAL = 0xd0
    
    // Stack the argument variable pointer (VARPNT)
    constexpr std::uint8_t kVARPNT = ApplesoftVariables::ZP_VARPNT;
    WriteZeroPageByte(kVARPNT + 1u, ReadZeroPageByte(kVARPNT + 1u));
    WriteZeroPageByte(kVARPNT, ReadZeroPageByte(kVARPNT));
    
    // Stack the text pointer (TXTPTR)
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    const std::uint16_t txtPtr = ReadZeroPageWord(kTXTPTR);
    
    // Scan to next statement
    DATA();
    
    // Fall through to FNCDATA to store 5-byte FAC
}

void FNC_() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNC_ (inclusive) .. FUNCT (exclusive)
    // Name normalization: none (assembler label FNC_ kept verbatim).
    //
    // Common routine for "DEF" and "FN" - parse FN token and function name.
    // Requires "FN" token, sets SUBFLG high bit, parses name to FNCNAM.

    // Require "FN" token
    SYNCHR(static_cast<std::uint8_t>(0xc2u));  // TOKEN_FN = 0xc2
    
    // Set high bit in SUBFLG to signal this is from DEF/FN context
    constexpr std::uint8_t kSUBFLG = ApplesoftVariables::ZP_SUBFLG;
    const std::uint8_t subflg = ReadZeroPageByte(kSUBFLG);
    WriteZeroPageByte(kSUBFLG, subflg | 0x80u);
    
    // Parse function name via PTRGET3
    PTRGET3();
    
    // PTRGET3 leaves A=name_lo, Y=name_hi
    // Store to FNCNAM
    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    const std::uint8_t nameA = ReadZeroPageByte(ApplesoftVariables::ZP_STRNG1);  // Temp storage from PTRGET3
    const std::uint8_t nameY = ReadZeroPageByte(ApplesoftVariables::ZP_STRNG1 + 1u);
    WriteZeroPageByte(kFNCNAM, nameA);
    WriteZeroPageByte(kFNCNAM + 1u, nameY);
    
    // Jump to CHKNUM to validate numeric type
    CHKNUM();
}

void FUNCT() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FUNCT (inclusive) .. FNCDATA (exclusive)
    // Name normalization: none (assembler label FUNCT kept verbatim).
    //
    // "FN" FUNCTION CALL - invoke user-defined function
    // Parse FN name, save old argument value, evaluate expression with new value,
    // restore old value via FNCDATA.

    // Parse "FN name"
    FNC_();
    
    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    constexpr std::uint8_t kVARPNT = ApplesoftVariables::ZP_VARPNT;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;
    
    // Stack function address for nested FN calls
    const std::uint16_t fncAddr = ReadZeroPageWord(kFNCNAM);
    
    // Parse "(expression)" and evaluate
    PARCHK();
    
    // Result in FAC - must be numeric
    CHKNUM();
    
    // Get argument variable pointer from FNCNAM+2,+3
    const std::uint16_t argVarAddr = fncAddr + 2u;
    WriteZeroPageWord(kVARPNT, argVarAddr);
    
    // Save old value of argument variable (5 bytes) to stack
    for (std::uint8_t i = 4u; i >= 0u && i <= 4u; --i) {
        const std::uint8_t byte = variables_const().pointer(argVarAddr).read(i);
        // TODO(asm-port): push byte to stack
    }
    
    // Store FAC to argument variable (rounded)
    STORE_FACDB_YX_ROUNDED();
    
    // Save current TXTPTR
    const std::uint16_t savedTxtPtr = ReadZeroPageWord(kTXTPTR);
    
    // Load function definition address from FNCNAM+0,+1
    const std::uint16_t defAddr = fncAddr;  // Will read via pointer arithmetic
    WriteZeroPageWord(kTXTPTR, defAddr);
    
    // Stack argument variable address for later
    
    // Evaluate function expression
    FRMNUM();
    
    // Validate at ":" or EOL
    if (CHRGOT() != 0u && CHRGOT() != static_cast<std::uint8_t>(':')) {
        SYNERR();
    }
    
    // Restore TXTPTR
    WriteZeroPageWord(kTXTPTR, savedTxtPtr);
    
    // Stack now contains 5 saved bytes - fall through to FNCDATA to restore
}

void FNCDATA() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: FNCDATA (inclusive) .. STR (exclusive)
    // Name normalization: none (assembler label FNCDATA kept verbatim).
    //
    // STORE FIVE BYTES FROM STACK AT (FNCNAM)
    // Pop 5 stack bytes and store to (FNCNAM),Y with Y incrementing.

    constexpr std::uint8_t kFNCNAM = ApplesoftVariables::ZP_FNCNAM;
    const std::uint16_t fncnampnt = ReadZeroPageWord(kFNCNAM);
    
    // Loop 5 times: pop stack and store
    for (std::uint8_t y = 0u; y < 5u; ++y) {
        // TODO(asm-port): pop stack byte
        // Store to (fncnampnt + y)
    }
}




void HANDLERR() {
    // Source: SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst
    // Labels: HANDLERR (inclusive) .. RESUME (exclusive)
    // Name normalization: none (assembler label HANDLERR kept verbatim).
    constexpr std::uint8_t kERRNUM = ApplesoftVariables::ZP_ERRNUM;
    constexpr std::uint8_t kERRLIN = ApplesoftVariables::ZP_ERRLIN;
    constexpr std::uint8_t kERRPOS = ApplesoftVariables::ZP_ERRPOS;
    constexpr std::uint8_t kERRSTK = ApplesoftVariables::ZP_ERRSTK;
    constexpr std::uint8_t kTXTPSV = ApplesoftVariables::ZP_TXTPSV;
    constexpr std::uint8_t kCURLSV = ApplesoftVariables::ZP_CURLSV;
    constexpr std::uint8_t kREMSTK = ApplesoftVariables::ZP_REMSTK;
    constexpr std::uint8_t kCURLIN = ApplesoftVariables::ZP_CURLIN;
    constexpr std::uint8_t kOLDTEXT = ApplesoftVariables::ZP_OLDTEXT;
    constexpr std::uint8_t kTXTPTR = ApplesoftVariables::ZP_TXTPTR;

    WriteZeroPageByte(kERRNUM, gPendingErrorCode);
    WriteZeroPageByte(kERRSTK, ReadZeroPageByte(kREMSTK));

    WriteZeroPageWord(kERRLIN, ReadZeroPageWord(kCURLIN));
    WriteZeroPageWord(kERRPOS, ReadZeroPageWord(kOLDTEXT));

    WriteZeroPageWord(kTXTPTR, ReadZeroPageWord(kTXTPSV));
    WriteZeroPageWord(kCURLIN, ReadZeroPageWord(kCURLSV));

    CHRGOT();
    GOTO();
    NEWSTT();
}


} // namespace applesoft::asm_port
