---
description: This document serves as a technical specification for an AI agent tasked with converting 6502 Assembly code into modern 64-bit C++. The primary challenge is the "lifting" of 16-bit split-byte addresses into high-level C++ pointers while maintaining the semantics of the original 8-bit logic.
---

# Specification: 6502 to C++ Pointer Synthesis

## 1. Context & Objective

The goal is to translate 6502 assembly (8-bit registers, 16-bit address space) into C++ code that runs natively on 64-bit architectures.

Instead of a simple "emulator" approach (where memory is just a `uint8_t[65536]` array), we want a **Refactored Translation**. We aim to identify when two 8-bit memory locations or registers are being used together to represent a 16-bit pointer, and convert those into C++ pointer types (`T*`) or references.

## 2. The Core Challenge: Pointer Fragmentation

On the 6502, a 16-bit address (e.g., `$C000`) is handled as two separate 8-bit values:

- **Low Byte (BAL):** The least significant 8 bits.
- **High Byte (BAH):** The most significant 8 bits.

**AI Task:** You must recognize patterns where the code loads, stores, or manipulates these bytes in tandem and "re-unify" them into a single C++ pointer variable.

## 3. Heuristics for Pointer Recognition

The AI agent should identify a "Pointer Candidate" when it sees the following patterns:

### A. Indirect Addressing Modes

Any memory location used as an operand for indirect instructions is a pointer by definition.

- **Pattern:** `LDA ($NN), Y` or `STA ($NN, X)`
- **Logic:** The 16-bit value stored at Zero Page address `$NN` and `$NN+1` is a pointer. In C++, this should be treated as a `uint8_t*`.

### B. High/Low Byte Loading (Immediate)

- **Pattern:**

```assembly
    LDA #<label  ; Load low byte of address
    STA pointer_lo
    LDA #>label  ; Load high byte of address
    STA pointer_hi
```

- **Logic:** The symbol `label` is the base of the pointer. The variables `pointer_lo` and `pointer_hi` should be consolidated into a single `uint8_t* ptr_variable`.

### C. Pointer Arithmetic

- **Pattern:** Adding a value to the low byte and propagating the carry to the high byte.

```assembly
    CLC
    LDA ptr_lo
    ADC #$10
    STA ptr_lo
    BCC skip
    INC ptr_hi
    skip:
```

- **Logic:** This is equivalent to `ptr_variable += 0x10;` in C++.

## 4. Handling Dual-Use Memory (The "Ambiguity" Problem)

A specific memory location (especially in Zero Page) may be used as a 16-bit integer in one subroutine and a pointer in another.

**AI Implementation Strategy:**

1. **Type Analysis:** Perform a data-flow analysis on every 16-bit pair.
2. **The `union` Approach:** If a location is used interchangeably as an integer and a pointer, represent it in C++ using a `union` or `std::variant`, or map it to a "Smart Pointer" class that allows byte-level access.
3. **Shadow Memory:** In cases where 16-bit math is performed on a pointer, translate it to C++ pointer arithmetic rather than raw integer math to preserve 64-bit compatibility.

## 5. Translation Rules for the AI

### Rule 1: Consolidation

When the AI detects a 16-bit pair being used for addressing, it must not create two `uint8_t` variables. It must create one `uint8_t*`.

### Rule 2: Address Space Mapping

Since the 6502 has a 64KB limit and the 64-bit target does not, the AI should:

- Define a `uint8_t* RAM_BASE` representing the start of the 6502's memory map.
- Convert 16-bit absolute addresses (e.g., `STA $D010`) into `RAM_BASE[0xD010]`.

However, if the absolute address is of some object in code space, use the address of the translated object instead.

### Rule 3: Zero Space Variable Lifting

If a subroutine expects a pointer in Zero Page (e.g., `$20` and `$21`), the C++ Zero page emulation (class ApplesoftVariables) the variable should be implemented as a `uint8_t*` or some more specialized pointer type.

## 6. Example Conversion Logic

**Source Assembler:**

```assembly
; Copy 10 bytes from SRC to DEST
    LDA #<SRC
    STA $20
    LDA #>SRC
    STA $21
    LDY #$00
loop
    LDA ($20), Y
    STA DEST, Y
    INY
    CPY #$0A
    BNE loop
```

**Desired C++ Output (Conceptual):**

```cpp
uint8_t* src_ptr = &RAM[SRC_ADDR];
for (int y = 0; y < 10; ++y) {
    RAM[DEST_ADDR + y] = src_ptr[y];
}
```

## 7. Summary for AI Instruction

"Search the assembly listing for split-byte manipulation of 16-bit values. Identify those used in indirect addressing or `ADC/INC` carry-chains. Translate these into unified 64-bit pointers in C++. If a memory location is used as both an integer and a pointer, implement a type-safe mechanism (like a union or casting) to allow both 8-bit math and 64-bit memory referencing."
