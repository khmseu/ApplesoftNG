---
name: writing-claims
description: "How to link C++ functions to assembly labels."
---

# Writing Symbol Claims

## Purpose

Link C++ functions to 6502 assembly ROM address ranges using `AS_Labels` claims.

## Trigger Phrases

- "write claims for"
- "add label range claims"
- "claim these symbols"
- "map this function to ROM"

## 1. Identification Rules

1.  **Search**: Consult [SourceMaterial/Combo/asrom.lst](../../../SourceMaterial/Combo/asrom.lst) to find the start label, address, and next label.
2.  **Fallback**: If the symbol is not obvious in the listing, use `asrom.sym` to confirm the exact symbol name, then return to the listing to determine the exclusive end label.
3.  **No Match**: If the symbol cannot be found, report `Missing symbol: <symbol_name>`.
4.  **End Label**: Use the next sequential label in the listing file as the exclusive boundary, regardless of its type or context.

## 2. Formatting Rules

Use this exact comment pattern:

```cpp
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: <StartLabel> (inclusive) .. <EndLabel> (exclusive)
```

1.  **<StartLabel>**: Exact inclusive entry point label (e.g., `AS_CHRGET`).
2.  **<EndLabel>**: Exact exclusive boundary label (e.g., `AS_CHRGOT`).
3.  **Name Normalization**: If the C++ name differs from the assembly label, add a note below the claim.
    - _Example_: `// Name normalization: AS_GET_CHAR maps to AS_CHRGET`

## 3. Placement Rules

**CRITICAL**: Claims MUST be **function-scoped**.

1.  **Open Function**: Identify the C++ function that implements the claimed ROM range.
2.  **Check Location**: Confirm you are editing the function definition, not a declaration.
3.  **Insert Claim**: Place the claim block immediately before that function definition.
    - _Valid example_:
      ```cpp
      // AS_Labels: AS_START (inclusive) .. AS_END (exclusive)
      void my_func() {
      }
      ```
4.  **Keep Together**: If you need a name-normalization note, place it directly below the claim block and immediately above the same function definition.
5.  **Invalid Scope**: Do not place claims inside the function body or at file scope far from the function they describe.
6.  **Isolation**: Keep each `AS_Labels` block attached to the function it describes, with no unrelated `AS_Labels` comments in between.

## 4. Validation Rules

1.  **Process**: Run `python3 ./tools/gen_symbol_implementation_map.py`.
2.  **Verify**: Run `rg "<Address>" docs/symbol-implementation-map.tsv`.
3.  **Confirm**: Columns 3 (Asm) and 5 (C++) must correspond to the same functional range and purpose as defined in the claim.
4.  **If Script Fails**: Check for invalid claim ranges or malformed comments, fix the input, rerun the script, and report the concrete failure message.

## Decision Guide

- **Single Function**: Claim as one range.
- **Multiple Entry Points**: Use multiple `AS_Labels` blocks sequentially immediately before the same function definition.
- **Overlap**: Scraper associates overlapping ranges with both functions.
- **Overlap Handling**: If overlap is intentional, keep both claims and ensure each function's comment states its own sub-range clearly. If overlap is accidental, adjust boundaries to remove ambiguity.

## Definition of Done

- Claim is in correct source file.
- Claim is function-scoped.
- Scraper runs without error.
- `.tsv` file shows correct mapping.
