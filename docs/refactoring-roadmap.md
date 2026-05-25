# Module Refactoring Guide: asm_port_error.cpp

## Current State

`asm_port_error.cpp` is no longer a single monolithic module. The original statement handlers and helpers have been split across dedicated `asm_port_*.cpp` modules, with `asm_port_core.cpp` now carrying most of the remaining shared evaluator and control plumbing.

## Current Split

### Display & Graphics

`src/core/asm_port_graphics.cpp` now owns the display and graphics control path.

Included functionality: `HOME()`, `GR()`, `TEXT()`, `HTAB()`, `VTAB()`, `COLOR()`, `HIMEM()`, `LOMEM()`, `SPEED()`, `PLOT()`, `HLIN()`, `VLIN()`, `FLASH()`, `INVERSE()`, `NORMAL()`, `PLOTFNS()`, `LINCOOR()`, and the monitor stubs.

### Control Flow

`src/core/asm_port_control_flow.cpp` now owns the jump, loop, and dispatch path.

Included functionality: `IF()`, `GOTO()`, `GOSUB()`, `RETURN()`, `FOR()`, `NEXT()`, `POP()`, `CONT()`, `STOP()`, `ENDX()`, `NEWSTT()`, `EXECUTE_STATEMENT()`, `CONTROL_C_TYPED()`, `TRACE_()`, `STEP()`, `GOEND()`, `COLON_()`, `ONGOTO()`, and `RESTART()`.

### Program & Data Statements

`src/core/asm_port_statements.cpp` now owns program listing, editing, and data handling.

Included functionality: `LIST()`, `DEL()`, `NEW()`, `CLEAR()`, `SCRTCH()`, `SAVE()`, `LOAD()`, `DATA()`, `READ()`, `RESTORE()`, `LET()`, `INPUT()`, `PARSE_INPUT_LINE()`, line insertion/deletion helpers, and `SETPTRS()` plumbing.

### Parsing & Tokenization

`src/core/asm_port_chrget.cpp`, `src/core/asm_port_parser.cpp`, and related helpers now own the character-input and syntax-check path.

Included functionality: `CHRGET()`, `CHRGOT()`, `LINGET()`, `GETBYT()`, `GETNUM()`, `GETADR()`, `SYNCHR()`, `CHKCOM()`, `CHKNUM()`, `CHKSTR()`, `CHKVAL()`, `CHKCLS()`, `CHKOPN()`, and `SYNERR()`.

### Expression Evaluation

`FRMEVL()` now lives in `src/core/asm_port_core.cpp`, with string and relational helpers split across `asm_port_strlt2.cpp`, `asm_port_print.cpp`, and `asm_port_parser.cpp`.

### Stack, Error, and Core Utilities

`src/core/asm_port_error_handling.cpp`, `src/core/asm_port_clear.cpp`, `src/core/asm_port_stack.cpp`, and `src/core/asm_port_core.cpp` carry the remaining utility and error-handling surface.

Included functionality: `AS_ERROR()`, `AS_MEMERR()`, `AS_PRINT_ERROR_LINNUM()`, `AS_SYNERR()`, `AS_HANDLERR()`, `AS_SETDA()`, stack helpers, zero-page helpers, and the remaining runtime glue that has not yet been split out.

## Current Direction

- Keep extracting only when a slice has a clear owner and a stable interface.
- Prefer moving behavior to the module that already owns the closest callers.
- Treat `asm_port_core.cpp` as the main consolidation point for shared evaluator and runtime plumbing until the remaining clusters become independently movable.

## Notes

- The roadmap should track the current split, not the original aspirational module list.
- Update this file whenever a new `asm_port_*.cpp` module becomes the canonical owner of a behavior cluster.
