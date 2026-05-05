# 6502 Assembly Analyzer Project

## Project Goal

Convert 6502 assembly language source code (for Apple II ROMs) into a structured, modular representation that can be used to:
1. Identify all subroutines/functions and their entry points
2. Map the call graph (which functions call which)
3. Organize code by ROM module
4. Generate output suitable for incremental conversion to higher-level languages (C, Python, etc.)

## Input Files

The project processes output from the **xa65 6502 assembler**.

### Module Organization

The Apple II ROM is organized into modules:
- **Applesoft**: Large BASIC interpreter (~10KB)
  - **Applesoft** module
- **Monitor**: Debugger/monitor (~2KB)
  - **Cassette, CMD, Debug, Disasm, Display1, Display2, Keyin, Lores, Math, Paddles, Vectors**: modules

Each module or submodule is a separate `.lst` and `.sym` file pair.

### 1. Listing Files (`.lst`)

**Format**: Line-by-line assembly listing with addresses and opcodes

**Example**:
```
1356 T:0431               PRINT_ERROR_LINNUM 
1357 T:0431  20 3a 0b               jsr STROUT
1358 T:0434  a9 00                  lda #$00
1359 T:0436  85 91                  sta JMPADRS+1
```

or:

```
112 T:0000  a0 4b        HEADR     ldy #$4b          ;WRITE A*256 'LONG 1'
113 T:0002  20 12 00               jsr ZERDLY                ;  HALF CYCLES
114 T:0005  d0 f9                  bne HEADR             ;  (650 USEC EACH)
```

**Structure**:
- Column 1: Line number (sequential)
- Column 2: `T:XXXX` - 16-bit address in hex (T: prefix means "Text" or code section)
- Column 3: Hex bytes of the instruction (variable number)
- Column 4+: Assembly mnemonic and operand
- After `;`: Comment

**Key Elements to Extract**:
- **Labels**: Appear on their own line with an address, e.g., `PRINT_ERROR_LINNUM`, or together with an instruction
- **Instructions**: `jsr`, `jmp`, `bne`, `beq`, `lda`, `sta`, etc.
- **Operands**: Label names, addresses, or immediate values
- **Addresses**: `T:XXXX` format (4 hex digits)

### 2. Symbol Files (`.sym`)

**Format**: Exported symbols with their addresses and a cross reference

**Example**:
```
GOWARM, 0x0000, 0, 0x0000
STROUT, 0x0F3A, 0, 0x0000
CHRGOT, 0x0F86, 0, 0x0000
    applesoft.s65 6372 6373 6378 6379 6380 6381
```

**Structure**:
- `LABEL, 0xADDRESS, 0, 0x0000` - comma-separated, label, address in hex with `0x` prefix, a constant `0`, and flags for linking and relocation again with `0x` prefix; flags&0x0002 = 0 (import) or 2 (export)
- cross reference lines, for example `applesoft.s65 6372 6373 6378 6379 6380 6381` found in applesoft.s65 (applesoft.o65.lst) on lines 6372, 6373, 6378, 6379, 6380, 6381

**Key Elements**:
- **Exported symbols**: Public entry points defined in the symbol table
- **Addresses**: Hex format with `0x` prefix

### 3. Module List (`modules.md`)

**Format**: Markdown list of modules in ROM order

**Example**:
```markdown
# Modules

- [Applesoft](src/system/applesoft)
  - applesoft
- [Autostart-Monitor](src/system/monitor/apple2plus)
  - monitor
  - lores
  - disasm
  - debug
  - paddles
  - display1
  - math
  - display2
  - cassette
  - keyin
  - cmd
  - vectors

For every module, there is a `*.s65` source file, a `*.o65.lst` assembler listing, an `*.o65.err` error summary, and a `*.o65.sym` symbol table.
```

**Purpose**: Defines the order in which modules appear in ROM, used to generate ROM-order output

## Key Concepts

### Subroutine Entry Points

A label is considered a subroutine entry point if it is:

1. **JSR target** - Called via `jsr LABEL` (Jump to SubRoutine)
   - Most reliable indicator of a subroutine
   - Example: `jsr STROUT` means STROUT is a subroutine

2. **JMP target** - Jumped to via `jmp LABEL` (unconditional jump) from outside the subroutine
   - May indicate a subroutine or internal control flow
   - Example: `jmp NEXT_STATE`

3. **Branch target** - Target of conditional branches (`bne`, `beq`, `bcc`, etc.) from outside the subroutine
   - Usually internal control flow, not subroutine entry
   - Example: `bne LOOP_START`

4. **Jump table entry** - Listed in `.word LABEL-1` tables
   - Used for dispatch tables and RTS-based indirect calls
   - Example: `.word HANDLER1-1, HANDLER2-1, HANDLER3-1`

5. **Symbol table entry** - Listed in `.sym` file
   - Official exported public functions
   - Example: `STROUT, 0x0F3A, 0, 0x0002`

6. **Label definition** - Explicitly labeled in code
   - Any line with `LABEL:` or `LABEL =`
   - Example: `PRINT_ERROR_LINNUM`
   - Usually (but not always) data, not a subroutine

## Output

The analyzer generates a report showing:

1. **Exported Functions** - From symbol table
2. **Discovered via JSR** - Subroutines called directly
3. **Discovered via JMP** - Jump targets
4. **Discovered via Jump Tables** - Dispatch table entries
5. **ROM Order** - All labels grouped by module and sorted by address

## xa65 Assembler Documentation

**xa65** is a 6502 cross-assembler. Key features:
- Produces `.lst` (listing) files with addresses and opcodes
- Produces `.sym` (symbol) files with exported symbols
- Uses `T:XXXX` notation for text/code section addresses
- Supports labels, mnemonics, operands, and comments
- Generates hex byte sequences for each instruction

**Reference**: https://www.floodgap.com/retrotech/xa/

## Problem Statement

**Current Challenge**: The analyzer must correctly:
1. Parse both Applesoft (large, single file) and Monitor (multiple small files) formats (all of wich together form the ROM image)
2. Identify all labels and their addresses
3. Track which module each label belongs to
4. Recognize all types of subroutine entry points
5. Generate ROM-order output that respects module sequence and address ordering
