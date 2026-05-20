---
name: writing-claims
description: "Use when: documenting or linking C++ functions to original assembly ROM address ranges (writing AS_Labels claims)."
---

# Writing Symbol Claims

## Purpose

Link C++ implementation functions to their original 6502 assembly ROM address ranges using a specific comment metadata format readable by `gen_symbol_implementation_map.py`.

## Trigger Phrases

- "write claims for"
- "add label range claims"
- "claim these symbols"
- "map this function to ROM"

## Procedure

### 1. Label Identification

- Find the starting label (and its address) in the authoritative listing: [SourceMaterial/Combo/asrom.lst](../../../SourceMaterial/Combo/asrom.lst).
- Identify the exclusive end label (the first label after the range).

### 2. Format Metadata

Use the standard comment block:

```cpp
// Source:
// SourceMaterial/Combo/asrom.lst
// AS_Labels: <StartLabel> (inclusive) .. <EndLabel> (exclusive)
```

- `<StartLabel>`: The inclusive entry point label.
- `<EndLabel>`: The exclusive boundary label (usually the start of the next routine).
- Use exact labels from the assembly (e.g., `AS_CHRGET`, `MON_COUT`).
- If there is a name mapping or normalization, add a `// Name normalization:` note below the claim.

### 3. Proper Placement

**CRITICAL**: Claims must be **function-scoped**.

- Place the comment block **immediately preceding** the function signature or **inside** the function body.
- **NEVER** use file-scoped claims at the top of a file; the map scraper assumes a claim applies to the very next function it encounters.
- Ensure no other `AS_Labels` comments exist between the claim and the function.

### 4. Validation

- Run the scraper: `python3 ./tools/gen_symbol_implementation_map.py`.
- Verify the output: `grep "<Address>" docs/symbol-implementation-map.tsv`.
- Ensure the third column (Original Symbol) and fifth column (C++ Function) match your intent.

## Decision Guide

- **Single Function vs. Fall-through**: If a routine is a single functional unit, claim it as one range.
- **Multiple Entry Points**: The map scraper supports multiple claims for a single C++ function. If a function implements multiple original assembly entry points (e.g., via fall-through or internal dispatch), you can provide multiple `AS_Labels` comment blocks. These can be "stacked" immediately preceding the function or placed inside the function body.
- **Overlapping Ranges**: If two functions claim overlapping ranges, the scraper will correctly associate the symbols in that overlap with both functions.

## Definition of Done

- Claim comment is in the correct C++ source file.
- Claim is placed at function scope (not file scope).
- `gen_symbol_implementation_map.py` runs successfully.
- `docs/symbol-implementation-map.tsv` shows correctly attributed symbols for the claimed range.
