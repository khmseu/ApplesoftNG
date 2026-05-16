#pragma once

#include <array>
#include <cstdint>

namespace applesoft::asm_port {

class ApplesoftVariables {
public:
  class ASLengthAlias {
  public:
    explicit ASLengthAlias(ApplesoftVariables *vars = nullptr) : vars_(vars) {}

    void bind(ApplesoftVariables *vars) { vars_ = vars; }

    operator std::uint8_t() const {
      return vars_ == nullptr ? 0u : lowByte(vars_->AS_JMPADRS);
    }

    ASLengthAlias &operator=(std::uint8_t value) {
      if (vars_ != nullptr) {
        setLowByte(vars_->AS_JMPADRS, value);
      }
      return *this;
    }

  private:
    ApplesoftVariables *vars_ = nullptr;
  };

  class SGNCPRAlias {
  public:
    explicit SGNCPRAlias(ApplesoftVariables *vars = nullptr) : vars_(vars) {}

    void bind(ApplesoftVariables *vars) { vars_ = vars; }

    operator std::uint8_t() const {
      return vars_ == nullptr ? 0u : lowByte(vars_->AS_STRNG1);
    }

    SGNCPRAlias &operator=(std::uint8_t value) {
      if (vars_ != nullptr) {
        setLowByte(vars_->AS_STRNG1, value);
      }
      return *this;
    }

  private:
    ApplesoftVariables *vars_ = nullptr;
  };

  class FACExtensionAlias {
  public:
    explicit FACExtensionAlias(ApplesoftVariables *vars = nullptr)
        : vars_(vars) {}

    void bind(ApplesoftVariables *vars) { vars_ = vars; }

    operator std::uint8_t() const {
      return vars_ == nullptr ? 0u : highByte(vars_->AS_STRNG1);
    }

    FACExtensionAlias &operator=(std::uint8_t value) {
      if (vars_ != nullptr) {
        setHighByte(vars_->AS_STRNG1, value);
      }
      return *this;
    }

  private:
    ApplesoftVariables *vars_ = nullptr;
  };

  class ARGExtensionAlias {
  public:
    explicit ARGExtensionAlias(ApplesoftVariables *vars = nullptr)
        : vars_(vars) {}

    void bind(ApplesoftVariables *vars) { vars_ = vars; }

    operator std::uint8_t() const {
      return vars_ == nullptr ? 0u : highByte(vars_->AS_JMPADRS);
    }

    ARGExtensionAlias &operator=(std::uint8_t value) {
      if (vars_ != nullptr) {
        setHighByte(vars_->AS_JMPADRS, value);
      }
      return *this;
    }

  private:
    ApplesoftVariables *vars_ = nullptr;
  };

  ApplesoftVariables() {
    AS_LENGTH.bind(this);
    AS_SGNCPR.bind(this);
    AS_FAC_EXTENSION.bind(this);
    AS_ARG_EXTENSION.bind(this);
  }

  // Canonical zero-page/fixed address names used by assembler ports.
  static constexpr std::uint8_t ZP_AS_GOWARM = 0x00;
  static constexpr std::uint8_t ZP_AS_GOSTROUT = 0x03;
  static constexpr std::uint8_t ZP_AS_USR = 0x0a;
  static constexpr std::uint8_t ZP_AS_CHARAC = 0x0d;
  static constexpr std::uint8_t ZP_AS_ENDCHR = 0x0e;
  static constexpr std::uint8_t ZP_AS_NUMDIM = 0x0f;
  static constexpr std::uint8_t ZP_AS_DIMFLG = 0x10;
  static constexpr std::uint8_t ZP_AS_VALTYP = 0x11;
  static constexpr std::uint8_t ZP_AS_VALTYP_PLUS_1 = 0x12;
  static constexpr std::uint8_t ZP_AS_GARFLG = 0x13;
  static constexpr std::uint8_t ZP_AS_SUBFLG = 0x14;
  static constexpr std::uint8_t ZP_AS_INPUTFLG = 0x15;
  static constexpr std::uint8_t ZP_AS_CPRMASK = 0x16;
  static constexpr std::uint8_t ZP_MON_WNDLFT = 0x20;
  static constexpr std::uint8_t ZP_MON_WNDWDTH = 0x21;
  static constexpr std::uint8_t ZP_MON_WNDTOP = 0x22;
  static constexpr std::uint8_t ZP_MON_WNDBTM = 0x23;
  static constexpr std::uint8_t ZP_MON_CH = 0x24;
  static constexpr std::uint8_t ZP_MON_CV = 0x25;
  static constexpr std::uint8_t ZP_MON_BASL = 0x28;
  static constexpr std::uint8_t ZP_MON_BASH = 0x29;
  static constexpr std::uint8_t ZP_MON_GBASL = 0x26;
  static constexpr std::uint8_t ZP_MON_GBASH = 0x27;
  static constexpr std::uint8_t ZP_MON_H2 = 0x2c;
  static constexpr std::uint8_t ZP_MON_V2 = 0x2d;
  static constexpr std::uint8_t ZP_MON_CHKSUM = 0x2e;
  static constexpr std::uint8_t ZP_MON_HMASK = 0x30;
  static constexpr std::uint8_t ZP_MON_COLOR = 0x30;
  static constexpr std::uint8_t ZP_MON_INVFLG = 0x32;
  static constexpr std::uint8_t ZP_MON_PROMPT = 0x33;
  static constexpr std::uint8_t ZP_MON_CSW = 0x36;
  static constexpr std::uint8_t ZP_MON_KSW = 0x38;
  static constexpr std::uint8_t ZP_MON_A1 = 0x3c;
  static constexpr std::uint8_t ZP_MON_A2 = 0x3e;
  static constexpr std::uint8_t ZP_MON_A4 = 0x40;
  static constexpr std::uint8_t ZP_MON_DEBUG_REG_A = 0x45;
  static constexpr std::uint8_t ZP_MON_DEBUG_REG_X = 0x46;
  static constexpr std::uint8_t ZP_MON_DEBUG_REG_Y = 0x47;
  static constexpr std::uint8_t ZP_MON_STATUS = 0x48;
  static constexpr std::uint8_t ZP_MON_RNDL = 0x4e;
  static constexpr std::uint8_t ZP_MON_RNDH = 0x4f;
  static constexpr std::uint8_t ZP_AS_LINNUM = 0x50;
  static constexpr std::uint8_t ZP_AS_TEMPPT = 0x52;
  static constexpr std::uint8_t ZP_AS_LASTPT = 0x53;
  static constexpr std::uint8_t ZP_AS_TEMPST = 0x55;
  static constexpr std::uint8_t ZP_AS_INDEX = 0x5e;
  static constexpr std::uint8_t ZP_AS_DEST = 0x60;
  static constexpr std::uint8_t ZP_AS_RESULT = 0x62;
  static constexpr std::uint8_t ZP_AS_TXTTAB = 0x67;
  static constexpr std::uint8_t ZP_AS_VARTAB = 0x69;
  static constexpr std::uint8_t ZP_AS_ARYTAB = 0x6b;
  static constexpr std::uint8_t ZP_AS_STREND = 0x6d;
  static constexpr std::uint8_t ZP_AS_FRETOP = 0x6f;
  static constexpr std::uint8_t ZP_AS_FRESPC = 0x71;
  static constexpr std::uint8_t ZP_AS_MEMSIZ = 0x73;
  static constexpr std::uint8_t ZP_AS_CURLIN = 0x75;
  static constexpr std::uint8_t ZP_AS_OLDLIN = 0x77;
  static constexpr std::uint8_t ZP_AS_OLDTEXT = 0x79;
  static constexpr std::uint8_t ZP_AS_DATLIN = 0x7b;
  static constexpr std::uint8_t ZP_AS_DATPTR = 0x7d;
  static constexpr std::uint8_t ZP_AS_INPTR = 0x7f;
  static constexpr std::uint8_t ZP_AS_VARNAM = 0x81;
  static constexpr std::uint8_t ZP_AS_VARPNT = 0x83;
  static constexpr std::uint8_t ZP_AS_FORPNT = 0x85;
  static constexpr std::uint8_t ZP_AS_TXPSV = 0x87;
  static constexpr std::uint8_t ZP_AS_CPRTYP = 0x89;
  static constexpr std::uint8_t ZP_AS_FNCNAM = 0x8a;
  static constexpr std::uint8_t ZP_AS_DSCPTR = 0x8c;
  static constexpr std::uint8_t ZP_AS_DSCLEN = 0x8f;
  static constexpr std::uint8_t ZP_AS_JMPADRS = 0x90;
  static constexpr std::uint8_t ZP_AS_LENGTH =
      0x91; // Dual-use byte in ROM: AS_LENGTH shares storage with AS_JMPADRS
            // low byte.
  static constexpr std::uint8_t ZP_AS_ARYPNT = 0x94;
  static constexpr std::uint8_t ZP_AS_HIGHTR = 0x96;
  static constexpr std::uint8_t ZP_AS_INDX = 0x99;
  static constexpr std::uint8_t ZP_AS_LOWTR = 0x9b;
  static constexpr std::uint8_t ZP_AS_FAC = 0x9d;
  static constexpr std::uint8_t ZP_AS_FAC_MANTISSA =
      0x9e; // Start of 4-byte mantissa (9e, 9f, a0, a1)
  static constexpr std::uint8_t ZP_AS_FAC_EXTENSION = 0xac;
  static constexpr std::uint8_t ZP_AS_FAC_SIGN = 0xa2;
  static constexpr std::uint8_t ZP_AS_SHIFT_SIGN_EXT = 0xa4;
  static constexpr std::uint8_t ZP_AS_ARG = 0xa5;
  static constexpr std::uint8_t ZP_AS_ARG_MANTISSA =
      0xa6; // Start of 4-byte mantissa (a6, a7, a8, a9)
  static constexpr std::uint8_t ZP_AS_ARG_SIGN = 0xaa;
  static constexpr std::uint8_t ZP_AS_SGNCPR = 0xab;
  static constexpr std::uint8_t ZP_AS_ARG_EXTENSION = 0x92;
  static constexpr std::uint8_t ZP_AS_STRNG1 =
      0xab; // Note: AS_SGNCPR and AS_STRNG1 share 0xAB in some contexts?
  static constexpr std::uint8_t ZP_AS_STRNG2 = 0xad;
  static constexpr std::uint8_t ZP_AS_PRGEND = 0xaf;
  static constexpr std::uint8_t ZP_AS_CHRGET = 0xb1;
  static constexpr std::uint8_t ZP_AS_CHRGOT = 0xb7;
  static constexpr std::uint8_t ZP_AS_TXTPTR = 0xb8;
  static constexpr std::uint8_t ZP_AS_LOCK = 0xd6;
  static constexpr std::uint8_t ZP_AS_ERRFLG = 0xd8;
  static constexpr std::uint8_t ZP_AS_ERRLIN = 0xda;
  static constexpr std::uint8_t ZP_AS_ERRPOS = 0xdc;
  static constexpr std::uint8_t ZP_AS_ERRNUM = 0xde;
  static constexpr std::uint8_t ZP_AS_ERRSTK = 0xdf;
  static constexpr std::uint8_t ZP_AS_HGR_BITS = 0x1c;
  static constexpr std::uint8_t ZP_AS_HGR_COUNT = 0x1d;
  static constexpr std::uint8_t ZP_AS_HGR_DX = 0xd0;
  static constexpr std::uint8_t ZP_AS_HGR_DY = 0xd2;
  static constexpr std::uint8_t ZP_AS_HGR_QUADRANT = 0xd3;
  static constexpr std::uint8_t ZP_AS_HGR_E = 0xd4;
  static constexpr std::uint8_t ZP_AS_HGR_X = 0xe0;
  static constexpr std::uint8_t ZP_AS_HGR_Y = 0xe2;
  static constexpr std::uint8_t ZP_AS_HGR_COLOR = 0xe4;
  static constexpr std::uint8_t ZP_AS_HGR_HORIZ = 0xe5;
  static constexpr std::uint8_t ZP_AS_HGR_PAGE = 0xe6;
  static constexpr std::uint8_t ZP_AS_HGR_SCALE = 0xe7;
  static constexpr std::uint8_t ZP_AS_HGR_SHAPE_PNTR = 0xe8;
  static constexpr std::uint8_t ZP_AS_HGR_COLLISIONS = 0xea;
  static constexpr std::uint8_t ZP_AS_HGR_SHAPE = 0x1a;
  static constexpr std::uint8_t ZP_AS_HGR_ROTATION = 0xf9;
  static constexpr std::uint8_t ZP_AS_FIRST = 0xf0;
  static constexpr std::uint8_t ZP_AS_SPEEDZ = 0xf1;
  static constexpr std::uint8_t ZP_AS_TRCFLG = 0xf2;
  static constexpr std::uint8_t ZP_AS_FLASH_BIT = 0xf3;
  static constexpr std::uint8_t ZP_AS_TXTPSV = 0xf4;
  static constexpr std::uint8_t ZP_AS_CURLSV = 0xf6;
  static constexpr std::uint8_t ZP_AS_REMSTK = 0xf8;

  static constexpr std::uint16_t ADDR_AS_INPUT_BUFFER_MINUS_4 = 0x01fcu;
  static constexpr std::uint16_t ADDR_AS_INPUT_BUFFER_MINUS_3 = 0x01fdu;
  static constexpr std::uint16_t ADDR_AS_INPUT_BUFFER_MINUS_1 = 0x01ffu;
  static constexpr std::uint16_t ADDR_AS_INPUT_BUFFER = 0x0200u;
  static constexpr std::uint16_t ADDR_MON_DEBUG_BOOTSTRAP = 0x03efu;
  static constexpr std::uint16_t ADDR_MON_DEBUG_BRK_VECTOR = 0x03f0u;
  static constexpr std::uint16_t ADDR_MON_DEBUG_WARM_VECTOR = 0x03f2u;
  static constexpr std::uint16_t ADDR_MON_NMI_VECTOR = 0x03fbu;
  static constexpr std::uint16_t ADDR_MON_IRQ_VECTOR = 0x03feu;
  static constexpr std::uint16_t ADDR_MON_BRK_VECTOR = 0x03f0u;
  static constexpr std::uint16_t ADDR_MON_DEBUG_SCAN_PAGE = 0x07f8u;

  class ConstBytePointer {
  public:
    ConstBytePointer(const ApplesoftVariables *vars, std::uint16_t address)
        : vars_(vars), address_(address) {}

    std::uint8_t read(std::uint16_t offset = 0) const {
      return vars_->readByte(static_cast<std::uint16_t>(address_ + offset));
    }

    std::uint16_t address() const { return address_; }

    ConstBytePointer advanced(std::uint16_t offset) const {
      return ConstBytePointer(vars_,
                              static_cast<std::uint16_t>(address_ + offset));
    }

  private:
    const ApplesoftVariables *vars_ = nullptr;
    std::uint16_t address_ = 0;
  };

  class BytePointer {
  public:
    BytePointer(ApplesoftVariables *vars, std::uint16_t address)
        : vars_(vars), address_(address) {}

    std::uint8_t read(std::uint16_t offset = 0) const {
      return vars_->readByte(static_cast<std::uint16_t>(address_ + offset));
    }

    void write(std::uint8_t value, std::uint16_t offset = 0) {
      vars_->writeByte(static_cast<std::uint16_t>(address_ + offset), value);
    }

    std::uint16_t address() const { return address_; }

    BytePointer advanced(std::uint16_t offset) const {
      return BytePointer(vars_, static_cast<std::uint16_t>(address_ + offset));
    }

    ConstBytePointer asConst() const {
      return ConstBytePointer(vars_, address_);
    }

  private:
    ApplesoftVariables *vars_ = nullptr;
    std::uint16_t address_ = 0;
  };

  // Zero-page and fixed-address variables currently used in the C++ ports.
  std::uint8_t AS_GOWARM = 0;           // $00 opcode byte
  std::uint16_t AS_GOWARM_TARGET = 0;   // $01/$02
  std::uint8_t AS_GOSTROUT = 0;         // $03 opcode byte
  std::uint16_t AS_GOSTROUT_TARGET = 0; // $04/$05
  std::uint8_t AS_USR = 0;              // $0a opcode byte
  std::uint16_t AS_USR_TARGET = 0;      // $0b/$0c
  std::uint8_t AS_CHARAC = 0;           // $0d
  std::uint8_t AS_ENDCHR = 0;           // $0e
  std::uint8_t AS_NUMDIM = 0;           // $0f  number of dimensions
  std::uint8_t AS_DIMFLG = 0;           // $10  AS_DIM-call flag
  std::uint8_t AS_VALTYP = 0; // $11  string flag: $ff = string, $00 = float
  std::uint8_t AS_VALTYP_PLUS_1 = 0; // $12  integer type flag
  std::uint8_t AS_GARFLG = 0;        // $13  garbage collection tried flag
  std::uint8_t AS_SUBFLG = 0;        // $14
  std::uint8_t AS_INPUTFLG = 0;    // $15  $00=AS_INPUT, $40=AS_GET, $98=AS_READ
  std::uint8_t AS_CPRMASK = 0;     // $16  relational comparison mask
  std::uint16_t MON_GBASL = 0;     // $26/$27  graphics base address low/high
  std::uint16_t MON_BASL = 0;      // $28/$29  text base address low/high
  std::uint8_t MON_CH = 0;         // $24
  std::uint8_t MON_CV = 0;         // $25  cursor vertical
  std::uint8_t MON_COLOR = 0;      // $30
  std::uint8_t MON_HMASK = 0;      // $30  (shares with MON_COLOR in ROM)
  std::uint8_t MON_INVFLG = 0xffu; // $32  $ff=normal, $3f=inverse
  std::uint8_t MON_PROMPT = 0;     // $33
  std::uint8_t MON_CHKSUM = 0;     // $2e
  std::uint16_t MON_A1 = 0;        // $3c/$3d
  std::uint16_t MON_A2 = 0;        // $3e/$3f
  std::uint16_t AS_LINNUM = 0;     // $50/$51
  std::uint8_t AS_TEMPPT = 0;      // $52
  std::uint8_t AS_LASTPT = 0;      // $53
  std::uint16_t AS_INDEX = 0; // $5e/$5f scratch pointer (string data address)
  std::array<std::uint8_t, 4> AS_RESULT{}; // $62..$65
  std::uint16_t AS_TXTTAB = 0;             // $67/$68
  std::uint16_t AS_VARTAB = 0;             // $69/$6a
  std::uint16_t AS_ARYTAB = 0;             // $6b/$6c
  std::uint16_t AS_STREND = 0;             // $6d/$6e
  std::uint16_t AS_FRETOP = 0;             // $6f/$70
  std::uint16_t AS_FRESPC = 0;             // $71/$72
  std::uint16_t AS_MEMSIZ = 0;             // $73/$74
  std::uint16_t AS_CURLIN = 0;             // $75/$76
  std::uint16_t AS_OLDLIN = 0;             // $77/$78
  std::uint16_t AS_OLDTEXT = 0;            // $79/$7a
  std::uint16_t AS_DATLIN =
      0; // $7b/$7c line number of current AS_DATA statement
  std::uint16_t AS_DATPTR = 0; // $7d/$7e
  std::uint16_t AS_INPTR = 0;  // $7f/$80 input pointer
  std::uint16_t AS_VARNAM = 0; // $81/$82
  std::uint16_t AS_VARPNT = 0; // $83/$84
  std::uint16_t AS_FORPNT = 0; // $85/$86
  std::uint16_t AS_TXPSV = 0;  // $87/$88 saved AS_TXTPTR during input parsing
  std::uint8_t AS_CPRTYP = 0;  // $89  relational operator flags
  std::uint16_t AS_FNCNAM = 0; // $8a/$8b
  std::uint16_t AS_DSCPTR = 0; // $8c/$8d
  std::uint8_t AS_DSCLEN = 0;  // $8f
  std::uint8_t AS_JMPADRS_OPCODE = 0; // $90
  std::uint16_t AS_JMPADRS = 0;       // $91/$92 (AS_JMPADRS+1,+2)
  ASLengthAlias AS_LENGTH{};   // Virtual alias for $91 (AS_JMPADRS low byte)
  std::uint16_t AS_ARYPNT = 0; // $94/$95
  std::uint16_t AS_HIGHTR = 0; // $96/$97
  std::uint8_t AS_INDX = 0;    // $99
  std::uint16_t AS_LOWTR = 0;  // $9b/$9c
  std::array<std::uint8_t, 5> AS_FAC{}; // $9d..$a1
  std::uint8_t AS_FAC_SIGN = 0;         // $a2
  std::uint8_t AS_SHIFT_SIGN_EXT = 0;   // $a4
  std::array<std::uint8_t, 6> AS_ARG{}; // $a5..$aa
  std::uint16_t AS_STRNG1 = 0;          // $ab/$ac
  SGNCPRAlias AS_SGNCPR{}; // Virtual alias for $ab (AS_STRNG1 low byte)
  FACExtensionAlias
      AS_FAC_EXTENSION{}; // Virtual alias for $ac (AS_STRNG1 high byte)
  ARGExtensionAlias
      AS_ARG_EXTENSION{};        // Virtual alias for $92 (AS_JMPADRS high byte)
  std::uint16_t AS_STRNG2 = 0;   // $ad/$ae
  std::uint16_t AS_PRGEND = 0;   // $af/$b0
  std::uint16_t AS_TXTPTR = 0;   // $b8/$b9
  std::uint8_t AS_LOCK = 0;      // $d6
  std::uint8_t AS_ERRFLG = 0;    // $d8
  std::uint8_t AS_FIRST = 0;     // $f0
  std::uint8_t AS_SPEEDZ = 0;    // $f1  complement of speed value
  std::uint8_t AS_TRCFLG = 0;    // $f2
  std::uint8_t AS_FLASH_BIT = 0; // $f3  $40=flash mode, $00=normal
  std::uint8_t AS_REMSTK = 0;    // $f8

  // Non-zero-page fixed addresses currently used.
  std::uint8_t AS_INPUT_BUFFER_MINUS_1 = 0;             // $01ff
  std::uint8_t AS_INPUT_BUFFER_MINUS_3 = 0;             // $01fd
  std::uint8_t AS_INPUT_BUFFER_MINUS_4 = 0;             // $01fc
  std::array<std::uint8_t, 256> AS_INPUT_BUFFER_PAGE{}; // $0200..$02ff

  // General flat backing store for arbitrary program-space addresses.
  // Zero page (0x00-0xFF) and the input-buffer page (0x0200-0x02FF) are
  // handled by their named member fields above; all other addresses (stack
  // page, program text, variable/array storage, string heap, etc.) use this
  // array so that ProgramPointer reads/writes work.
  std::array<std::uint8_t, 0x10000> general_memory_{};

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
  static void setLowByte(std::uint16_t &target, std::uint8_t value);
  static void setHighByte(std::uint16_t &target, std::uint8_t value);
};

ApplesoftVariables &variables();
const ApplesoftVariables &variables_const();

} // namespace applesoft::asm_port
