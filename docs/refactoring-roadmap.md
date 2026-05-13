# Module Refactoring Guide: asm_port_error.cpp

## Current State

**File Size**: ~4,100 lines, 352 void functions, 42 bool functions  
**Status**: Monolithic module containing all major Applesoft statement handlers and utilities  
**Issue**: Large file makes navigation and modification difficult

## Proposed Modular Structure

The following describes how `asm_port_error.cpp` should be split into logical, maintainable modules. This refactoring should be done **incrementally** as functions become independent of cross-module dependencies.

### Module 1: Display & Graphics Statements (`asm_port_graphics.cpp`)

**Purpose**: All graphics and display mode control  
**Dependencies**: IOPorts, ApplesoftVariables, console abstraction  
**Expected Size**: ~500-600 lines

**Functions to Extract**:

- `HOME()` - Clear screen and home cursor
- `GR()` - Enter lo-res graphics mode
- `TEXT()` - Enter text mode
- `HTAB(column)` - Set horizontal tab (cursor column)
- `VTAB(line)` - Set vertical tab (cursor line)
- `COLOR(colorCode)` - Set current graphics color
- `HIMEM(address)` - Set upper memory limit
- `LOMEM(address)` - Set lower memory limit (for variables)
- `SPEED(value)` - Set animation speed
- `PLOT(x, y)` - Plot lo-res graphics point
- `HLIN(x1, x2, y)` - Draw lo-res horizontal line
- `VLIN(x, y1, y2)` - Draw lo-res vertical line
- `HCOLOR(color)` - Set hi-res color (stub)
- `HPLOT(x, y)` - Plot hi-res point (stub)
- `DRAW(shape)` - Draw hi-res shape (stub)
- `XDRAW(shape)` - XOR draw hi-res shape (stub)
- `FLASH()` - Set flash text mode
- `INVERSE()` - Set inverse video mode
- `NORMAL()` - Set normal video mode
- `PLOTFNS()` - Helper: evaluate plot coordinates
- `LINCOOR()` - Helper: evaluate line coordinates
- Monitor stubs: `MON_HOME()`, `MON_SETGR()`, `MON_SETTXT()`, `MON_SETCOL()`, `MON_TABV()`, `MON_PLOT()`, `MON_HLINE()`, `MON_VLINE()`, `MON_INPORT()`, `MON_OUTPORT()`

**Rationale**: Graphics and display are conceptually separate from statement execution and control flow. They primarily interact with zero-page graphics state and the console/monitor stubs.

---

### Module 2: Control Flow Statements (`asm_port_control_flow.cpp`)

**Purpose**: Conditional execution, jumps, and loops  
**Dependencies**: Program counter management, stack operations  
**Expected Size**: ~800-1000 lines

**Functions to Extract**:

- `IF()` - Conditional statement
- `IF_TRUE()` - Internal conditional executor
- `GOTO(lineNumber)` - Unconditional jump
- `GO_TO_LINE()` - Helper for GOTO
- `GOSUB(lineNumber)` - Subroutine call
- `RETURN()` - Return from subroutine
- `FOR(var, start, end)` - Loop initialization
- `NEXT(var)` - Loop terminator
- `NEXT_shouldTerminateLoop()` - Helper for NEXT logic
- `POP()` - Pop return/FOR stack frame
- `CONT()` - Continue after STOP
- `STOP()` - Pause execution
- `STOP_impl(shouldPrint)` - Internal STOP implementation
- `ENDX()` / `ENDX_impl(shouldPrint)` - End program
- `NEWSTT()` - Execute next statement
- `EXECUTE_STATEMENT()` - Main statement dispatcher
- `EXECUTE_STATEMENT_1()` - Internal dispatcher
- `CONTROL_C_TYPED()` - Interrupt handler
- `TRACE_()` - Trace mode (stub)
- `STEP()` - Step mode (stub)
- `GOEND()` - Go to program end (stub)
- `COLON_()` - Statement separator (stub)

**Rationale**: Control flow is a distinct feature from data manipulation, I/O, and graphics. These functions primarily manage the execution stack and program flow.

---

### Module 3: Program & Data Statements (`asm_port_statements.cpp`)

**Purpose**: Program loading, listing, editing, and data handling  
**Dependencies**: Memory management, program structure  
**Expected Size**: ~1000-1200 lines

**Functions to Extract**:

- `LIST()` - List program lines
- `DEL(startLine, endLine)` - Delete program lines
- `NEW()` - Clear program and variables
- `CLEAR()` - Clear variables only
- `CLEARC()` - Clear variables helper
- `SCRTCH()` - Scratch (synonym for NEW)
- `SAVE()` - Save program to storage (stub)
- `LOAD()` - Load program from storage (stub)
- `DATA()` - Data statement (stub)
- `READ()` - Read from DATA statement
- `RESTORE()` - Reset DATA pointer
- `LET()` - Variable assignment
- `LET2(savedValType)` - Internal assignment helper
- `INPUT()` - Interactive input
- `PARSE_INPUT_LINE()` - Helper: parse input
- `InsertNewLine()` - Helper: insert line into program
- `DeleteExistingLine()` - Helper: delete line from program
- `SETPTRS()` - Set up memory pointers
- `SETPTRS_impl()` - Internal SETPTRS
- `PRINT_ERROR_LINNUM()` - Helper: print line numbers in errors

**Rationale**: Program management and data handling are closely related and separate from expression evaluation and execution control. These functions manage the program structure itself.

---

### Module 4: Expression Evaluation (`asm_port_expressions.cpp`)

**Purpose**: Numeric and string expression evaluation, operators  
**Dependencies**: String literals, numeric coercion, expression evaluation engine  
**Expected Size**: ~1200-1400 lines

**Functions to Extract**:

- `FRMEVL()` - Main expression evaluator (critical, complex)
- `FRMNUM()` - Numeric expression evaluator
- `FRMSTR()` - String expression evaluator
- Operator functions: `OR()`, `ANDOP()`, `RELOPS()`, comparison functions
- Truth value functions: `TRUE()`, `FALSE()`
- Numeric coercion: `CNVNUM()`, `CNVSTR()`
- Token evaluation: `FNC_EVAL()`, `EVAL_PAREN()`, etc.

**Rationale**: Expression evaluation is complex and self-contained, with minimal interaction with other subsystems. Extracting it supports future enhancements to the evaluator.

---

### Module 5: Parsing & Tokenization (`asm_port_parser.cpp`)

**Purpose**: Character input, syntax checking, number parsing  
**Dependencies**: Program buffer, soft switches  
**Expected Size**: ~600-800 lines

**Functions to Extract**:

- `CHRGET()` - Get next character from program
- `CHRGOT()` - Get current character
- `GENERIC_CHRGET()` - Generic character input
- `LINGET()` - Parse line number
- `GETBYT()` - Parse expression → byte
- `GETNUM()` - Parse number
- `GETADR()` - Parse address expression
- Syntax checking: `SYNCHR()`, `CHKCOM()`, `CHKNUM()`, `CHKSTR()`, `CHKVAL()`, `CHKCLS()`, `CHKOPN()`
- Error for syntax: `SYNERR()`

**Rationale**: Text parsing is a distinct subsystem focused on tokenization and syntax validation. These functions have minimal interaction with expression evaluation or statement execution.

---

### Module 6: Stack & Memory Utilities (`asm_port_core.cpp` or rename `asm_port_error.cpp`)

**Purpose**: Core utilities, initialization, error handling  
**Dependencies**: Zero-page memory, stack management  
**Expected Size**: ~400-500 lines

**Functions to Extract/Keep**:

- `RESTART()` - Warm restart
- `COLD_START()` - Cold initialization
- `GENERIC_END()` - Program end
- Stack operations: `PushByteToStack()`, `PopByteFromStack()`, `SetStackPointer()`
- Zero-page access: `ReadZeroPageByte()`, `ReadZeroPageWord()`, `WriteZeroPageByte()`, `WriteZeroPageWord()`
- State helpers: `SetTextPointer()`, `ClearErrFlag()`, `MarkDirectMode()`, etc.
- Error handling: `GOERR()`, `MEMERR()`, error dispatchers
- Initialization: `STKINI()`, `CONINT()`
- Monitor stubs: `MON_INPORT()`, `MON_OUTPORT()` (if not in graphics)

**Rationale**: Core utilities are foundational infrastructure used by all other modules. They should remain stable and tightly integrated.

---

## Implementation Strategy

### Phase 1: Documentation & Preparation (Current)

- ✅ Document proposed module structure (this file)
- ✅ Add section comments to `asm_port_error.cpp` identifying logical boundaries
- ✅ Create `asm_port_graphics.cpp` and extract first graphics/display slice
- ✅ Document function-to-module mapping

### Phase 2: Low-Risk Extraction (Next Iteration)

- ✅ Extract first graphics/display functions (NORMAL..SPEED and MON\_\* stubs)
- ✅ Extract remaining graphics functions (`PLOT`/`HLIN`/`VLIN` and helpers `PLOTFNS`/`LINCOOR`)
- ✅ Update CMakeLists.txt and build wiring
- ✅ Verify compilation and cross-reference regeneration

### Phase 3: Gradual Extraction

- ⏳ Extract parsing/tokenization functions (in progress: `SYNCHR`/`CHKNUM`/`CHKSTR`/`CHKCOM`/`CHKCLS`/`CHKOPN` + `LINGET`/`GTBYTC`/`GETBYT`/`GTNUM`/`COMBYTE`/`GETADR` moved to `asm_port_parser.cpp`)
- Extract core utilities (if not already done)
- Extract statement functions
- Extract control flow
- Extract expression evaluation (most complex, do last)

### Phase 4: Integration Testing

- Verify full build with all modules
- Regenerate cross-reference
- Run any available tests

---

## Dependencies Analysis

### Tight Couplings (Hard to Extract)

- **FRMEVL** ↔ **LINGET, CHRGET**: Expression evaluation depends on character input
- **FOR/NEXT** ↔ **Stack operations**: Loop requires stack frame management
- **Statement dispatch** ↔ **Token table**: Execution requires function pointers

### Moderate Couplings (Require Planning)

- **Graphics** → **Monitor stubs**: Display functions call MON\_\* routines (can move together)
- **Program statements** → **Memory utilities**: NEW/CLEAR require zero-page management
- **Control flow** → **Parser**: Condition evaluation requires expression parsing

### Loose Couplings (Easy to Extract)

- **Display mode** (FLASH, INVERSE, NORMAL): Isolated zero-page writes
- **Cursor position** (HTAB, VTAB): Independent of other statements
- **Memory limits** (HIMEM, LOMEM): Localized parameter setting

---

## Header File Templates

Each module should have a corresponding header file (`include/core/asm_port_<module>.hpp`) with:

1. **Module documentation** explaining purpose and scope
2. **Public function declarations** for statement handlers and major helpers
3. **Dependencies** listed in comments (what other modules it calls)
4. **Rationale** for extraction (why this grouping makes sense)

---

## Success Criteria

### Immediate (After Current Work)

- [ ] asm_port_error.cpp has clear section headers
- [ ] Refactoring roadmap is documented and shared
- [ ] No changes to build system or function interfaces

### Short-term (Next 1-2 sprints)

- [ ] Graphics module extracted successfully
- [ ] Build still passes all tests
- [ ] Cross-reference still accurate
- [ ] Module boundaries validated

### Medium-term (Ongoing)

- [ ] All 6 proposed modules extracted
- [ ] Each module ≤ 1200 lines
- [ ] Clear dependencies between modules
- [ ] Easier to maintain and enhance

---

## Notes

- **Risk Mitigation**: Incremental extraction reduces risk of breaking builds
- **CI/CD**: Each extraction phase should result in a working, tested state
- **Backwards Compatibility**: Public interfaces remain stable; refactoring is internal
- **Future Enhancements**: Module split enables parallel development and easier testing

This roadmap should be revisited and updated as the refactoring progresses.
