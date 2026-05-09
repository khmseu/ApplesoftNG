#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

class ApplesoftVariables {
public:
    // Zero-page and fixed-address variables currently used in the C++ ports.
    std::uint8_t VALTYP = 0;      // $11  string flag: $ff = string, $00 = float
    std::uint8_t SUBFLG = 0;      // $14
    std::uint8_t INPUTFLG = 0;    // $15  $00=INPUT, $40=GET, $98=READ
    std::uint8_t MON_CH = 0;      // $24
    std::uint8_t MON_PROMPT = 0;  // $33

    std::uint16_t MON_A1 = 0;     // $3c/$3d
    std::uint16_t MON_A2 = 0;     // $3e/$3f
    std::uint16_t INDEX = 0;      // $5e/$5f scratch pointer (string data address)
    std::uint16_t LINNUM = 0;     // $50/$51
    std::uint8_t TEMPPT = 0;      // $52
    std::uint16_t TXTTAB = 0;     // $67/$68
    std::uint16_t VARTAB = 0;     // $69/$6a
    std::uint16_t ARYTAB = 0;     // $6b/$6c
    std::uint16_t STREND = 0;     // $6d/$6e
    std::uint16_t FRETOP = 0;     // $6f/$70
    std::uint16_t MEMSIZ = 0;     // $73/$74
    std::uint16_t CURLIN = 0;     // $75/$76
    std::uint16_t OLDLIN = 0;     // $77/$78
    std::uint16_t OLDTEXT = 0;    // $79/$7a
    std::uint16_t DATLIN = 0;     // $7b/$7c line number of current DATA statement
    std::uint16_t DATPTR = 0;     // $7d/$7e
    std::uint16_t INPTR = 0;      // $7f/$80 input pointer
    std::uint16_t FORPNT = 0;     // $85/$86
    std::uint16_t TXPSV = 0;      // $87/$88 saved TXTPTR during input parsing
    std::uint16_t LOWTR = 0;      // $9b/$9c
    std::array<std::uint8_t, 5> FAC{}; // $9d..$a1
    std::uint8_t FAC_SIGN = 0;    // $a2
    std::uint16_t STRNG1 = 0;     // $ab/$ac
    std::uint16_t STRNG2 = 0;     // $ad/$ae
    std::uint16_t PRGEND = 0;     // $af/$b0
    std::uint16_t TXTPTR = 0;     // $b8/$b9
    std::uint8_t LOCK = 0;        // $d6
    std::uint8_t ERRFLG = 0;      // $d8
    std::uint8_t TRCFLG = 0;      // $f2
    std::uint8_t SPEEDZ = 0;      // $f1  complement of speed value
    std::uint8_t FLASH_BIT = 0;   // $f3  $40=flash mode, $00=normal
    std::uint8_t REMSTK = 0;      // $f8
    std::uint8_t CHARAC = 0;      // $0d
    std::uint8_t ENDCHR = 0;      // $0e

    // Non-zero-page fixed addresses currently used.
    std::uint8_t INPUT_BUFFER_MINUS_1 = 0; // $01ff
    std::uint8_t KEYBOARD = 0;    // $c000 (memory-mapped keyboard latch)
    std::array<std::uint8_t, 256> INPUT_BUFFER_PAGE{}; // $0200..$02ff

    // Small amount of emulated CPU status currently needed by ports.
    bool carryFlag = false;

    std::uint8_t readByte(std::uint16_t address) const;
    void writeByte(std::uint16_t address, std::uint8_t value);

    std::uint16_t readWord(std::uint16_t address) const;
    void writeWord(std::uint16_t address, std::uint16_t value);

    static std::uint8_t lowByte(std::uint16_t value);
    static std::uint8_t highByte(std::uint16_t value);
    static void setLowByte(std::uint16_t& target, std::uint8_t value);
    static void setHighByte(std::uint16_t& target, std::uint8_t value);
};

ApplesoftVariables& variables();
const ApplesoftVariables& variables_const();

} // namespace applesoft::asm_port
