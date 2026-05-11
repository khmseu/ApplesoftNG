#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

class ApplesoftVariables {
public:
    // Canonical zero-page/fixed address names used by assembler ports.
    static constexpr std::uint8_t ZP_CHARAC = 0x0d;
    static constexpr std::uint8_t ZP_ENDCHR = 0x0e;
    static constexpr std::uint8_t ZP_NUMDIM = 0x0f;
    static constexpr std::uint8_t ZP_DIMFLG = 0x10;
    static constexpr std::uint8_t ZP_VALTYP = 0x11;
    static constexpr std::uint8_t ZP_VALTYP_PLUS_1 = 0x12;
    static constexpr std::uint8_t ZP_GARFLG = 0x13;
    static constexpr std::uint8_t ZP_SUBFLG = 0x14;
    static constexpr std::uint8_t ZP_INPUTFLG = 0x15;
    static constexpr std::uint8_t ZP_CPRMASK = 0x16;
    static constexpr std::uint8_t ZP_MON_CH = 0x24;
    static constexpr std::uint8_t ZP_MON_PROMPT = 0x33;
    static constexpr std::uint8_t ZP_MON_A1 = 0x3c;
    static constexpr std::uint8_t ZP_MON_A2 = 0x3e;
    static constexpr std::uint8_t ZP_LINNUM = 0x50;
    static constexpr std::uint8_t ZP_TEMPPT = 0x52;
    static constexpr std::uint8_t ZP_LASTPT = 0x53;
    static constexpr std::uint8_t ZP_TEMPST = 0x55;
    static constexpr std::uint8_t ZP_INDEX = 0x5e;
    static constexpr std::uint8_t ZP_RESULT = 0x62;
    static constexpr std::uint8_t ZP_TXTTAB = 0x67;
    static constexpr std::uint8_t ZP_VARTAB = 0x69;
    static constexpr std::uint8_t ZP_ARYTAB = 0x6b;
    static constexpr std::uint8_t ZP_STREND = 0x6d;
    static constexpr std::uint8_t ZP_FRETOP = 0x6f;
    static constexpr std::uint8_t ZP_FRESPC = 0x71;
    static constexpr std::uint8_t ZP_MEMSIZ = 0x73;
    static constexpr std::uint8_t ZP_CURLIN = 0x75;
    static constexpr std::uint8_t ZP_OLDLIN = 0x77;
    static constexpr std::uint8_t ZP_OLDTEXT = 0x79;
    static constexpr std::uint8_t ZP_DATLIN = 0x7b;
    static constexpr std::uint8_t ZP_DATPTR = 0x7d;
    static constexpr std::uint8_t ZP_INPTR = 0x7f;
    static constexpr std::uint8_t ZP_VARNAM = 0x81;
    static constexpr std::uint8_t ZP_VARPNT = 0x83;
    static constexpr std::uint8_t ZP_FORPNT = 0x85;
    static constexpr std::uint8_t ZP_TXPSV = 0x87;
    static constexpr std::uint8_t ZP_CPRTYP = 0x89;
    static constexpr std::uint8_t ZP_FNCNAM = 0x8a;
    static constexpr std::uint8_t ZP_DSCPTR = 0x8c;
    static constexpr std::uint8_t ZP_JMPADRS = 0x90;
    static constexpr std::uint8_t ZP_ARYPNT = 0x94;
    static constexpr std::uint8_t ZP_HIGHTR = 0x96;
    static constexpr std::uint8_t ZP_INDX = 0x99;
    static constexpr std::uint8_t ZP_LOWTR = 0x9b;
    static constexpr std::uint8_t ZP_FAC = 0x9d;
    static constexpr std::uint8_t ZP_FAC_SIGN = 0xa2;
    static constexpr std::uint8_t ZP_ARG = 0xa5;
    static constexpr std::uint8_t ZP_STRNG1 = 0xab;
    static constexpr std::uint8_t ZP_STRNG2 = 0xad;
    static constexpr std::uint8_t ZP_PRGEND = 0xaf;
    static constexpr std::uint8_t ZP_TXTPTR = 0xb8;
    static constexpr std::uint8_t ZP_LOCK = 0xd6;
    static constexpr std::uint8_t ZP_ERRFLG = 0xd8;
    static constexpr std::uint8_t ZP_FIRST = 0xf0;
    static constexpr std::uint8_t ZP_SPEEDZ = 0xf1;
    static constexpr std::uint8_t ZP_TRCFLG = 0xf2;
    static constexpr std::uint8_t ZP_FLASH_BIT = 0xf3;
    static constexpr std::uint8_t ZP_REMSTK = 0xf8;

    static constexpr std::uint16_t ADDR_INPUT_BUFFER_MINUS_1 = 0x01ffu;
    static constexpr std::uint16_t ADDR_INPUT_BUFFER = 0x0200u;
    static constexpr std::uint16_t ADDR_KEYBOARD = 0xc000u;

    class ConstBytePointer {
    public:
        ConstBytePointer(const ApplesoftVariables* vars, std::uint16_t address)
            : vars_(vars), address_(address) {}

        std::uint8_t read(std::uint16_t offset = 0) const {
            return vars_->readByte(static_cast<std::uint16_t>(address_ + offset));
        }

        std::uint16_t address() const {
            return address_;
        }

        ConstBytePointer advanced(std::uint16_t offset) const {
            return ConstBytePointer(vars_, static_cast<std::uint16_t>(address_ + offset));
        }

    private:
        const ApplesoftVariables* vars_ = nullptr;
        std::uint16_t address_ = 0;
    };

    class BytePointer {
    public:
        BytePointer(ApplesoftVariables* vars, std::uint16_t address)
            : vars_(vars), address_(address) {}

        std::uint8_t read(std::uint16_t offset = 0) const {
            return vars_->readByte(static_cast<std::uint16_t>(address_ + offset));
        }

        void write(std::uint8_t value, std::uint16_t offset = 0) {
            vars_->writeByte(static_cast<std::uint16_t>(address_ + offset), value);
        }

        std::uint16_t address() const {
            return address_;
        }

        BytePointer advanced(std::uint16_t offset) const {
            return BytePointer(vars_, static_cast<std::uint16_t>(address_ + offset));
        }

        ConstBytePointer asConst() const {
            return ConstBytePointer(vars_, address_);
        }

    private:
        ApplesoftVariables* vars_ = nullptr;
        std::uint16_t address_ = 0;
    };

    // Zero-page and fixed-address variables currently used in the C++ ports.
    std::uint8_t NUMDIM = 0;      // $0f  number of dimensions
    std::uint8_t DIMFLG = 0;      // $10  DIM-call flag
    std::uint8_t VALTYP = 0;      // $11  string flag: $ff = string, $00 = float
    std::uint8_t VALTYP_PLUS_1 = 0; // $12  integer type flag
    std::uint8_t GARFLG = 0;      // $13  garbage collection tried flag
    std::uint8_t SUBFLG = 0;      // $14
    std::uint8_t INPUTFLG = 0;    // $15  $00=INPUT, $40=GET, $98=READ
    std::uint8_t CPRMASK = 0;     // $16  relational comparison mask
    std::uint8_t MON_CH = 0;      // $24
    std::uint8_t MON_PROMPT = 0;  // $33

    std::uint16_t MON_A1 = 0;     // $3c/$3d
    std::uint16_t MON_A2 = 0;     // $3e/$3f
    std::uint16_t INDEX = 0;      // $5e/$5f scratch pointer (string data address)
    std::uint16_t LINNUM = 0;     // $50/$51
    std::uint8_t TEMPPT = 0;      // $52
    std::uint8_t LASTPT = 0;      // $53
    std::uint16_t TXTTAB = 0;     // $67/$68
    std::uint16_t VARTAB = 0;     // $69/$6a
    std::uint16_t ARYTAB = 0;     // $6b/$6c
    std::uint16_t STREND = 0;     // $6d/$6e
    std::uint16_t FRETOP = 0;     // $6f/$70
    std::uint16_t FRESPC = 0;     // $71/$72
    std::uint16_t MEMSIZ = 0;     // $73/$74
    std::uint16_t CURLIN = 0;     // $75/$76
    std::uint16_t OLDLIN = 0;     // $77/$78
    std::uint16_t OLDTEXT = 0;    // $79/$7a
    std::uint16_t DATLIN = 0;     // $7b/$7c line number of current DATA statement
    std::uint16_t DATPTR = 0;     // $7d/$7e
    std::uint16_t INPTR = 0;      // $7f/$80 input pointer
    std::uint16_t FORPNT = 0;     // $85/$86
    std::uint16_t TXPSV = 0;      // $87/$88 saved TXTPTR during input parsing
    std::uint8_t CPRTYP = 0;      // $89  relational operator flags
    std::uint16_t DSCPTR = 0;     // $8c/$8d
    std::uint16_t JMPADRS = 0;    // $90/$91 (JMPADRS+1,+2)
    std::uint16_t ARYPNT = 0;     // $94/$95
    std::uint16_t HIGHTR = 0;     // $96/$97
    std::uint8_t INDX = 0;        // $99
    std::uint16_t LOWTR = 0;      // $9b/$9c
    std::array<std::uint8_t, 5> FAC{}; // $9d..$a1
    std::uint8_t FAC_SIGN = 0;    // $a2
    std::array<std::uint8_t, 6> ARG{}; // $a5..$aa
    std::uint16_t STRNG1 = 0;     // $ab/$ac
    std::uint16_t STRNG2 = 0;     // $ad/$ae
    std::uint16_t PRGEND = 0;     // $af/$b0
    std::uint16_t TXTPTR = 0;     // $b8/$b9
    std::uint8_t LOCK = 0;        // $d6
    std::uint8_t ERRFLG = 0;      // $d8
    std::uint16_t VARNAM = 0;     // $81/$82
    std::uint16_t VARPNT = 0;     // $83/$84
    std::array<std::uint8_t, 4> RESULT{}; // $62..$65
    std::uint8_t FIRST = 0;       // $f0
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

    ConstBytePointer pointer(std::uint16_t address) const {
        return ConstBytePointer(this, address);
    }

    BytePointer pointer(std::uint16_t address) {
        return BytePointer(this, address);
    }

    static std::uint8_t lowByte(std::uint16_t value);
    static std::uint8_t highByte(std::uint16_t value);
    static std::uint16_t makeWord(std::uint8_t low, std::uint8_t high);
    static void setLowByte(std::uint16_t& target, std::uint8_t value);
    static void setHighByte(std::uint16_t& target, std::uint8_t value);
};

ApplesoftVariables& variables();
const ApplesoftVariables& variables_const();

} // namespace applesoft::asm_port
