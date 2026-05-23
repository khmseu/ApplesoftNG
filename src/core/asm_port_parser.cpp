#include "core/applesoft_variables.hpp"
#include "core/asm_port_chrget.hpp"
#include "core/asm_port_error.hpp"
#include "core/asm_port_error_handling.hpp"
#include "core/asm_port_error_messages.hpp"
#include <cstdint>

namespace applesoft::asm_port {

std::uint8_t ReadProgramByte(std::uint16_t address);
void AS_SYNERR();
void AS_FRMEVL();
void AS_FRMNUM();
void AS_CONINT();
void AS_MKINT();
bool AS_ISLETC();
void AS_AYINT();
void AS_PTRGET4();
std::uint8_t AS_COMBYTE();
void AS_GETADR();
void AS_IQERR();
void AS_ROUND_FAC();

void AS_SYNCHR(std::uint8_t expected);
void AS_CHKCLS();
void AS_CHKOPN();

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_SYNCHR (inclusive) .. AS_SYNERR (exclusive)
// Name normalization: none (assembler label AS_SYNCHR kept verbatim).
void AS_SYNCHR(std::uint8_t expected) {
  // Require a specific statement token from the parsed input.
  // Read current character from AS_TXTPTR, compare with expected, advance if
  // match, error if not.

  const std::uint16_t txtPtr = variables_const().AS_TXTPTR;
  const std::uint8_t current = variables_const().pointer(txtPtr).read(0u);

  if (current != expected) {
    AS_ERROR(AS_ERR_SYNTAX);
  }

  // Advance AS_TXTPTR by 1.
  variables().AS_TXTPTR = static_cast<std::uint16_t>(txtPtr + 1u);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CHKNUM (inclusive) .. AS_CHKSTR (exclusive)
// Name normalization: none (assembler label AS_CHKNUM kept verbatim).
void AS_CHKNUM() {
  const bool facIsString = (variables_const().AS_VALTYP & 0x80u) != 0u;
  if (facIsString) {
    AS_ERROR(AS_ERR_BADTYPE);
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CHKSTR (inclusive) .. AS_CHKVAL (exclusive)
// Name normalization: none (assembler label AS_CHKSTR kept verbatim).
void AS_CHKSTR() {
  const bool facIsString = (variables_const().AS_VALTYP & 0x80u) != 0u;
  if (!facIsString) {
    AS_ERROR(AS_ERR_BADTYPE);
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CHKCOM (inclusive) .. AS_SYNCHR (exclusive)
// Name normalization: none (assembler label AS_CHKCOM kept verbatim).
void AS_CHKCOM() { AS_SYNCHR(static_cast<std::uint8_t>(',')); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_FRMNUM (inclusive) .. AS_CHKNUM (exclusive)
// Name normalization: none (assembler label AS_FRMNUM kept verbatim).
void AS_FRMNUM() {
  // AS_FRMNUM does JSR AS_FRMEVL and falls through into AS_CHKNUM in ROM.

  AS_FRMEVL();
  AS_CHKNUM();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_PARCHK (inclusive) .. AS_CHKCLS (exclusive)
// Name normalization: none (assembler label AS_PARCHK kept verbatim).
void AS_PARCHK() {
  // Validates and evaluates a parenthesized expression at AS_TXTPTR:
  //   jsr AS_CHKOPN  — require '(' at current position
  //   jsr AS_FRMEVL  — evaluate the enclosed expression into AS_FAC
  //   fall-through into AS_CHKCLS — require ')' at current position

  AS_CHKOPN();
  AS_FRMEVL();
  AS_CHKCLS();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_STORE_FACDB_YX_ROUNDED (inclusive) .. AS_COPY_ARG_TO_FAC
// (exclusive)
// Name normalization: STORE_FACDB_YX_ROUNDED ->
// AS_STORE_FACDB_YX_ROUNDED virtual Applesoft prefix only.
void AS_STORE_FACDB_YX_ROUNDED() {
  // Pointer candidate lifted: the ROM passes the destination in Y:X; the
  // current C++ path materializes that destination in AS_VARPNT before calling
  // this helper.
  AS_ROUND_FAC();

  const std::uint16_t destinationAddress = variables_const().AS_VARPNT;
  auto destination = variables().pointer(destinationAddress);

  destination.write(variables_const().AS_FAC[0], 0u);
  destination.write(
      static_cast<std::uint8_t>((variables_const().AS_FAC_SIGN | 0x7fu) &
                                variables_const().AS_FAC[1]),
      1u);
  destination.write(variables_const().AS_FAC[2], 2u);
  destination.write(variables_const().AS_FAC[3], 3u);
  destination.write(variables_const().AS_FAC[4], 4u);

  variables().AS_FAC_EXTENSION = 0u;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CHKCLS (inclusive) .. AS_CHKOPN (exclusive)
// Name normalization: none (assembler label AS_CHKCLS kept verbatim).
void AS_CHKCLS() {
  // Check for ')' at current position.
  AS_SYNCHR(static_cast<std::uint8_t>(')'));
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CHKOPN (inclusive) .. AS_CHKCOM (exclusive)
// Name normalization: none (assembler label AS_CHKOPN kept verbatim).
void AS_CHKOPN() {
  // Check for '(' at current position.
  AS_SYNCHR(static_cast<std::uint8_t>('('));
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_LINGET (inclusive) .. AS_LET (exclusive)
// Name normalization: none (assembler label AS_LINGET kept verbatim).
void AS_LINGET() {
  auto is_digit = [](std::uint8_t ch) {
    return ch >= static_cast<std::uint8_t>('0') &&
           ch <= static_cast<std::uint8_t>('9');
  };

  variables().AS_LINNUM = 0u;

  std::uint8_t current = AS_CHRGOT();
  while (is_digit(current)) {
    const std::uint8_t digit =
        static_cast<std::uint8_t>(current - static_cast<std::uint8_t>('0'));
    variables().AS_CHARAC = digit;

    const std::uint8_t lineHigh =
        ApplesoftVariables::highByte(variables_const().AS_LINNUM);
    ApplesoftVariables::setLowByte(variables().AS_INDEX, lineHigh);

    // Preserve ROM overflow guard (line number exceeds 63999).
    if (lineHigh >= 0x19u) {
      AS_SYNERR();
      return;
    }

    const std::uint16_t value = variables_const().AS_LINNUM;
    variables().AS_LINNUM = static_cast<std::uint16_t>(value * 10u + digit);

    current = AS_CHRGET();
  }
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GTBYTC (inclusive) .. AS_GETBYT (exclusive)
// Name normalization: none (assembler label AS_GTBYTC kept verbatim).
//
// Advance text pointer one character, then evaluate as byte (0-255) in
// AS_FAC+4.
//   jsr AS_CHRGET     -- advance AS_TXTPTR
//   fall-through to AS_GETBYT
void AS_GTBYTC() {
  AS_CHRGET();
  (void)AS_GETBYT();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GETBYT (inclusive) .. AS_CONINT (exclusive)
// Name normalization: none (assembler label AS_GETBYT kept verbatim).
//
// Evaluate numeric expression at AS_TXTPTR into AS_FAC, convert to byte (0-255)
// in AS_FAC+4.
//   jsr AS_FRMNUM     -- evaluate numeric expression
//   fall-through to AS_CONINT
std::uint8_t AS_GETBYT() {
  AS_FRMNUM();
  AS_CONINT();
  return variables_const().AS_FAC[4];
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CONINT (inclusive) .. AS_VAL (exclusive)
// Name normalization: none (assembler label AS_CONINT kept verbatim).
//
// Convert AS_FAC to a single-byte integer (0-255) in AS_FAC+4.
//   jsr AS_MKINT      -- truncate AS_FAC to integer
//   ldx AS_FAC+3      -- high byte must be zero (else >255)
//   bne AS_GOIQ       -- illegal quantity
//   ldx AS_FAC+4      -- result byte in X (AS_FAC+4 = low byte)
//   jmp AS_CHRGOT     -- refresh A with current input char and return
void AS_CONINT() {
  AS_MKINT();
  const std::uint8_t facHi = variables_const().AS_FAC[3];
  if (facHi != 0u) {
    AS_IQERR(); // AS_GOIQ: jmp AS_IQERR -- value > 255
    return;
  }
  // AS_FAC+4 already holds the result byte after AS_MKINT.
  (void)AS_CHRGOT();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GTNUM (inclusive) .. AS_COMBYTE (exclusive)
// Name normalization: none (assembler label AS_GTNUM kept verbatim).
std::uint8_t AS_GTNUM() {
  AS_FRMNUM();
  AS_GETADR();
  return AS_COMBYTE();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_COMBYTE (inclusive) .. AS_GETADR (exclusive)
// Name normalization: none (assembler label AS_COMBYTE kept verbatim).
std::uint8_t AS_COMBYTE() {
  AS_CHKCOM();
  return AS_GETBYT();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_QINT (inclusive) .. AS_INT (exclusive)
// Name normalization: none (assembler label AS_QINT kept verbatim).
void AS_QINT() {
  // AS_QINT converts the unpacked AS_FAC mantissa into a signed 32-bit integer
  // in AS_FAC+1..AS_FAC+4 by arithmetic right-shifting according to the
  // exponent. The five-byte AS_FAC is treated as one logical fixed-point value
  // here rather than re-porting the ROM's bytewise shifter helpers separately.

  const std::uint8_t exponent = variables_const().AS_FAC[0];
  if (exponent == 0u) {
    variables().AS_FAC[1] = 0u;
    variables().AS_FAC[2] = 0u;
    variables().AS_FAC[3] = 0u;
    variables().AS_FAC[4] = 0u;
    return;
  }

  const std::uint32_t mantissa =
      (static_cast<std::uint32_t>(variables_const().AS_FAC[1]) << 24u) |
      (static_cast<std::uint32_t>(variables_const().AS_FAC[2]) << 16u) |
      (static_cast<std::uint32_t>(variables_const().AS_FAC[3]) << 8u) |
      static_cast<std::uint32_t>(variables_const().AS_FAC[4]);

  const bool isNegative = (variables_const().AS_FAC_SIGN & 0x80u) != 0u;
  std::int64_t signedMantissa = static_cast<std::int64_t>(mantissa);
  if (isNegative) {
    signedMantissa = -signedMantissa;
    variables().AS_SHIFT_SIGN_EXT = 0xffu;
  }

  const std::uint8_t shiftCount = static_cast<std::uint8_t>(0xa0u - exponent);
  std::int32_t integerValue;
  if (shiftCount >= 32u) {
    integerValue = isNegative ? static_cast<std::int32_t>(-1)
                              : static_cast<std::int32_t>(0);
  } else {
    integerValue = static_cast<std::int32_t>(signedMantissa >> shiftCount);
  }

  const std::uint32_t packedInteger = static_cast<std::uint32_t>(integerValue);
  variables().AS_FAC[1] = static_cast<std::uint8_t>(packedInteger >> 24u);
  variables().AS_FAC[2] = static_cast<std::uint8_t>(packedInteger >> 16u);
  variables().AS_FAC[3] = static_cast<std::uint8_t>(packedInteger >> 8u);
  variables().AS_FAC[4] = static_cast<std::uint8_t>(packedInteger & 0xffu);

  variables().AS_SHIFT_SIGN_EXT = 0u;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_GETADR (inclusive) .. AS_PEEK (exclusive)
// Name normalization: none (assembler label AS_GETADR kept verbatim).
void AS_GETADR() {
  if (variables_const().AS_FAC[0] >= 0x91u) {
    AS_IQERR();
    return;
  }

  AS_QINT();

  const std::uint8_t lo = variables_const().AS_FAC[4];
  const std::uint8_t hi = variables_const().AS_FAC[3];
  ApplesoftVariables::setLowByte(variables().AS_LINNUM, lo);
  ApplesoftVariables::setHighByte(variables().AS_LINNUM, hi);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_EXECUTE_STATEMENT (inclusive) .. AS_EXECUTE_STATEMENT_1
// (exclusive)
// Name normalization: CurrentStatementChar maps to AS_EXECUTE_STATEMENT.
std::uint8_t CurrentStatementChar() {
  // AS_EXECUTE_STATEMENT uses `ldy #0` then `lda (AS_TXTPTR),Y`.
  return ReadProgramByte(variables_const().AS_TXTPTR);
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_END2 (inclusive) .. AS_SAVE (exclusive)
// Name normalization: IsStatementEndOfParsedInput maps to AS_END2 guard.
bool IsStatementEndOfParsedInput() {
  // AS_STOP/END/AS_CONT continue only when parser is at end-of-statement; model
  // the zero-flag check via the current parsed character at AS_TXTPTR.
  return AS_CHRGOT() == 0u;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_CHKVAL (inclusive) .. AS_FRMEVL (exclusive)
// Name normalization: none (assembler label AS_CHKVAL kept verbatim).
bool AS_CHKVAL(std::uint8_t savedValTyp) {
  const bool facIsString = (variables_const().AS_VALTYP & 0x80u) != 0u;
  const bool expectedString = (savedValTyp & 0x80u) != 0u;

  if (facIsString != expectedString) {
    AS_ERROR(AS_ERR_BADTYPE);
    return false;
  }

  return facIsString;
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_NEG32768 (inclusive) .. AS_MAKINT (exclusive)
// Name normalization: none (assembler label AS_NEG32768 kept verbatim).
void AS_NEG32768() {
  variables().AS_RESULT[0] = kAS_NEG32768Data[0];
  variables().AS_RESULT[1] = kAS_NEG32768Data[1];
  variables().AS_RESULT[2] = kAS_NEG32768Data[2];
  variables().AS_RESULT[3] = kAS_NEG32768Data[3];
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_MAKINT (inclusive) .. AS_MKINT (exclusive)
// Name normalization: none (assembler label AS_MAKINT kept verbatim).
void AS_MAKINT() {
  AS_CHRGET();
  AS_FRMNUM();
  AS_MKINT();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_MI1 (inclusive) .. AS_MI2 (exclusive)
// Name normalization: none (assembler label AS_MI1 kept verbatim).
void AS_MI1() { AS_IQERR(); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_MI2 (inclusive) .. AS_ARRAY (exclusive)
// Name normalization: none (assembler label AS_MI2 kept verbatim).
void AS_MI2() { AS_QINT(); }

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_ISLETC (inclusive) .. AS_NAME_NOT_FOUND (exclusive)
// Name normalization: none (assembler label AS_ISLETC kept verbatim).
bool AS_ISLETC() {
  const std::uint8_t ch = AS_CHRGOT();
  return ch >= static_cast<std::uint8_t>('A') &&
         ch <= static_cast<std::uint8_t>('Z');
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_MKINT (inclusive) .. AS_AYINT (exclusive)
// Name normalization: none (assembler label AS_MKINT kept verbatim).
void AS_MKINT() {
  if ((variables_const().AS_FAC_SIGN & 0x80u) != 0u) {
    AS_MI1();
    return;
  }

  AS_AYINT();
}

// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: AS_NAMOK (inclusive) .. AS_BASIC (exclusive)
// Name normalization: none (assembler label AS_NAMOK kept verbatim).
void AS_NAMOK() {
  variables().AS_VALTYP = 0u;        // AS_VALTYP
  variables().AS_VALTYP_PLUS_1 = 0u; // AS_VALTYP+1
  AS_PTRGET4();
}

} // namespace applesoft::asm_port