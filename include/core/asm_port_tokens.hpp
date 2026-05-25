#pragma once

#include <cstddef>
#include <cstdint>

namespace applesoft::asm_port {

enum class ASToken : std::uint8_t {
  END = 0x80u,
  FOR = 0x81u,
  NEXT = 0x82u,
  DATA = 0x83u,
  INPUT = 0x84u,
  DEL = 0x85u,
  DIM = 0x86u,
  READ = 0x87u,
  GR = 0x88u,
  TEXT = 0x89u,
  PR_NUMBER = 0x8au,
  IN_NUMBER = 0x8bu,
  CALL = 0x8cu,
  PLOT = 0x8du,
  HLIN = 0x8eu,
  VLIN = 0x8fu,
  HGR2 = 0x90u,
  HGR = 0x91u,
  HCOLOR = 0x92u,
  HPLOT = 0x93u,
  DRAW = 0x94u,
  XDRAW = 0x95u,
  HTAB = 0x96u,
  HOME = 0x97u,
  ROT = 0x98u,
  SCALE = 0x99u,
  SHLOAD = 0x9au,
  TRACE = 0x9bu,
  NOTRACE = 0x9cu,
  NORMAL = 0x9du,
  INVERSE = 0x9eu,
  FLASH = 0x9fu,
  COLOR = 0xa0u,
  POP = 0xa1u,
  VTAB = 0xa2u,
  HIMEM = 0xa3u,
  LOMEM = 0xa4u,
  ONERR = 0xa5u,
  RESUME = 0xa6u,
  RECALL = 0xa7u,
  STORE = 0xa8u,
  SPEED = 0xa9u,
  LET = 0xaau,
  GOTO = 0xabu,
  RUN = 0xacu,
  IF = 0xadu,
  RESTORE = 0xaeu,
  AMPERSAND = 0xafu,
  GOSUB = 0xb0u,
  RETURN = 0xb1u,
  REM = 0xb2u,
  STOP = 0xb3u,
  ON = 0xb4u,
  WAIT = 0xb5u,
  LOAD = 0xb6u,
  SAVE = 0xb7u,
  DEF = 0xb8u,
  POKE = 0xb9u,
  PRINT = 0xbau,
  CONT = 0xbbu,
  LIST = 0xbcu,
  CLEAR = 0xbdu,
  GET = 0xbeu,
  NEW = 0xbfu,
  TAB = 0xc0u,
  TO = 0xc1u,
  FN = 0xc2u,
  SPC = 0xc3u,
  THEN = 0xc4u,
  AT = 0xc5u,
  NOT = 0xc6u,
  STEP = 0xc7u,
  PLUS = 0xc8u,
  MINUS = 0xc9u,
  MUL = 0xcau,
  DIV = 0xcbu,
  POW = 0xccu,
  AND = 0xcdu,
  OR = 0xceu,
  GREATER = 0xcfu,
  EQUAL = 0xd0u,
  LESS = 0xd1u,
  SGN = 0xd2u,
  INT = 0xd3u,
  ABS = 0xd4u,
  USR = 0xd5u,
  FRE = 0xd6u,
  SCRN = 0xd7u,
  PDL = 0xd8u,
  POS = 0xd9u,
  SQR = 0xdau,
  RND = 0xdbu,
  LOG = 0xdcu,
  EXP = 0xddu,
  COS = 0xdeu,
  SIN = 0xdfu,
  TAN = 0xe0u,
  ATN = 0xe1u,
  PEEK = 0xe2u,
  LEN = 0xe3u,
  STR = 0xe4u,
  VAL = 0xe5u,
  ASC = 0xe6u,
  CHR = 0xe7u,
  LEFT = 0xe8u,
  RIGHT = 0xe9u,
  MID = 0xeau,
};

inline constexpr std::uint8_t kTokenBase =
    static_cast<std::uint8_t>(ASToken::END);

constexpr std::uint8_t token_byte(ASToken token) {
  return static_cast<std::uint8_t>(token);
}

inline constexpr std::size_t kTokenCount = static_cast<std::size_t>(
    token_byte(ASToken::MID) - token_byte(ASToken::END) + 1u);

constexpr bool is_statement_token(std::uint8_t token) {
  return token >= token_byte(ASToken::END) && token <= token_byte(ASToken::NEW);
}

constexpr std::size_t statement_token_index(std::uint8_t token) {
  return static_cast<std::size_t>(token - token_byte(ASToken::END));
}

constexpr bool is_math_token(std::uint8_t token) {
  return token >= token_byte(ASToken::PLUS) &&
         token <= token_byte(ASToken::LESS);
}

constexpr bool is_unary_function_token(std::uint8_t token) {
  return token >= token_byte(ASToken::SGN) && token <= token_byte(ASToken::MID);
}

} // namespace applesoft::asm_port