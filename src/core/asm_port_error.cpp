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
void RELOPS();
void STRCMP();
void NUMCMP();
void CMPDONE();
void CHKCOM();
void CHKOPN();
void CHKCLS();
void PDL();
void PTRGET3();
void BADNAM();
void NAMOK();
void BASIC();
void BASIC2();
void FNC_();
void PARCHK();
void STORE_FACDB_YX_ROUNDED();
void GETARY();
void GETARY2();
void NEG32768();
void MAKINT();
void MKINT();
void MI1();
void MI2();
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
void GIVAYF(std::int16_t value);

void PRINT_ERROR_LINNUM();
void PRINT_ERROR_LINNUM(std::string_view prefix);


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

// TODO(asm-port): port FLOAT_1 label.
void FLOAT_1(std::uint8_t exponent) {
    WriteZeroPageByte(ApplesoftVariables::ZP_FAC, exponent);
}

std::uint8_t gPendingErrorCode = ERR_SYNTAX;

// TODO(asm-port): compare temporary ARG and FAC strings and return -1/0/1.
std::int8_t CompareArgAndFacStrings() {
    return 0;
}

std::uint8_t gJerErrorCode = ERR_SYNTAX;
constexpr std::uint8_t kNEG32768Data[4] = {0x90u, 0x80u, 0x00u, 0x00u};
constexpr std::uint8_t kCZeroData[2] = {0x00u, 0x00u};

// TODO(asm-port): port MON_PREAD monitor paddle reader.
std::uint8_t MON_PREAD() {
    return 0;
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





} // namespace applesoft::asm_port
