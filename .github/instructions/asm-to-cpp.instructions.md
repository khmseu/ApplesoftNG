---
applyTo: "SourceMaterial/Combo/asrom.lst,SourceMaterial/Combo/asrom.sym,src/**/*.cpp,include/**/*.hpp"
---

# Incremental Assembler To C++ Conversion

Use these instructions when converting a bounded assembler segment into C++.

This file is the canonical rule set for asm-range conversion. Prompts and skills should reference this file instead of duplicating its full rules.

## Required Inputs

- `start_label`: first label of the conversion window, inclusive.
- `end_label`: last label of the conversion window, exclusive.

Treat only label-definition entries bound to addresses in [SourceMaterial/Combo/asrom.lst](../../SourceMaterial/Combo/asrom.lst) as valid symbols.

## Mandatory Workflow

1. Locate both labels in the authoritative listing. If either label is not found, stop and report which label is missing before proceeding.
1. Extract the range that starts at `start_label` and stops immediately before `end_label`.
1. If a found label has no associated instructions in the extracted range, skip it and continue with the next valid label in range.
1. Identify 16-bit pointer candidates in the extracted range before coding using this checklist:
   - indirect addressing operands such as `($NN),Y` and `($NN,X)`
   - split-byte address construction such as `#<label` and `#>label` stored to adjacent bytes
   - low-byte add plus carry-chain high-byte increment sequences (`ADC`/`INC` patterns)
1. For each pointer candidate, plan a single unified C++ representation (pointer or pointer abstraction) instead of separate low/high byte locals.
1. Before coding, consult [docs/function-cross-reference.md](../../docs/function-cross-reference.md) to determine which functions in this window are already ported, which are still stubs/placeholders, and where each function is implemented.
1. Include comments that are:
   - inline inside the range.
   - immediately preceding the first line in range and directly describing the range behavior.

1. Infer the behavior and data flow from opcodes, branch patterns, and comments.
   - If the source slice does not end in an unconditional transfer (`RTS`, `JMP`, or unconditional branch), it falls through into the next label. Model that fall-through in C++ by calling the following function at that point, or by returning state that the caller uses to invoke the next label.

   - If the extracted range contains invalid or unsupported opcodes, log an error, skip only the affected instructions, and continue porting the remaining valid instructions in range.

   - All symbols in the authoritative sources already include context-appropriate prefixes (`AS_` or `MON_`). Use these exact labels verbatim in C++.

1. Implement one primary C++ function that preserves the original assembler name as much as possible.
   - If the bounded window contains instruction groups with independent control flow or independent state transitions, split the port into multiple C++ functions so the entire `start_label`..`end_label` window is covered.
   - Keep each split function focused on a coherent sub-range, and preserve original label naming where possible.
1. Place the function in the appropriate runtime area:
   - interpreter/runtime logic: [src/core](../../src/core) and [include/core](../../include/core)
   - console or machine-facing I/O behavior: [src/platform](../../src/platform) and [include/platform](../../include/platform)

1. Add a short provenance comment above the function with:
   - source listing path
   - start/end labels
   - any normalization done to keep name valid in C++

1. If callees are not implemented yet, add dummy implementations in the same subsystem.
1. Update [docs/function-cross-reference.md](../../docs/function-cross-reference.md) and [docs/symbol-implementation-map.tsv](../../docs/symbol-implementation-map.tsv) after the port so it reflects new implementations and current stub/real status.
1. Add or update function-scoped `AS_Labels` claim comments for the new implementation following [.github/skills/writing-claims/SKILL.md](../skills/writing-claims/SKILL.md).

- When the range is split into multiple C++ functions, add separate claims for each function's covered sub-range.

1. Run a build after each increment; use the Output Checklist below as the acceptance gate for all detailed constraints.

## Implementation Rules

Quick reference:

| Area                 | Required action                                                               |
| -------------------- | ----------------------------------------------------------------------------- |
| Memory handling      | Unify 16-bit pointers and route fixed-address state through project accessors |
| Naming               | Keep source labels; minimally normalize only when needed for C++              |
| Missing dependencies | Add only required stubs and mark with `TODO(asm-port)`                        |
| Scope discipline     | Keep changes bounded to the two-label window unless compile requires more     |

### 16-bit Pointer Synthesis

- Represent each logical 16-bit pointer with one unified C++ value (pointer/wrapper), not split low/high locals.
- Convert carry-chain pointer math (`ADC`/`INC` patterns) into one unified pointer operation (for example `ptr += offset`).
- For dual-use zero-page pairs (integer or pointer), use one explicit dual-use representation and document it.
- For absolute references, use translated C++ objects or `ApplesoftVariables` accessors when available; otherwise use one consistent base-memory mapping model.

### Naming

- Keep assembler symbol names verbatim when valid in C++.
- If a symbol is not a valid C++ identifier, minimally normalize it (for example `.` to `_`) and document original symbol in a comment.

### Fixed-Address Variables

- Route fixed-address state reads/writes through `ApplesoftVariables` accessors (`variables()` / `variables_const()`).
- Do not duplicate fixed addresses with separate globals/statics/structs when `ApplesoftVariables` already covers them.
- If a fixed address is missing, add it to `ApplesoftVariables` first, then use the accessor from ported code.
- Route `$C000-$CFFF` reads/writes through `IOPorts` named single-byte accessors.

### Dummy Implementations

Create stubs only for missing dependencies required by the converted function.

| Rule                | Detail                                                                    |
| ------------------- | ------------------------------------------------------------------------- |
| Name                | Use the source label; if invalid in C++, apply only minimal normalization |
| Marker              | Add `TODO(asm-port)` with the source label                                |
| `void` return       | Empty body                                                                |
| Pointer return      | `return nullptr;`                                                         |
| Arithmetic return   | `return 0;`                                                               |
| `bool` return       | `return false;`                                                           |
| Class/struct return | `return {};`                                                              |

## Output Checklist

- Exactly one bounded behavior slice was ported.
- If the slice does not end in an unconditional transfer, the implementation explicitly models the fall-through to `end_label` by calling the next function or returning continuation state.
- 16-bit pointer candidates in the window were identified and lifted to unified C++ representations.
- Any carry-chain pointer arithmetic in the window was represented as unified pointer arithmetic.
- Any dual-use integer/pointer storage in the window has an explicit, documented dual-use representation.
- Function location matches subsystem boundaries.
- Original names preserved or minimally normalized with mapping comments.
- Missing callees have explicit dummy implementations.
- Any fixed-address global state access uses `ApplesoftVariables` accessors.
- [docs/function-cross-reference.md](../../docs/function-cross-reference.md) has been updated to reflect this port.
- Function-scoped `AS_Labels` claims are present and follow [.github/skills/writing-claims/SKILL.md](../skills/writing-claims/SKILL.md).
- If the port window was split, each generated function has a separate claim covering its own sub-range.
- Project still configures and builds.

## Function Address Table Pattern

When the conversion window contains a table of `.word LABEL` or `.word LABEL-1` entries (a 6502 jump-table or RTS-dispatch table):

- The `-1` offset is a 6502 RTS-dispatch artifact; in C++ use the plain function pointer without adjustment.
- Declare a type alias for the common function signature: `using <TableName>_fn = <return>(*)(<params>);`
- Implement a single lookup function named after the table label:

  ```cpp
  <TableName>_fn <TableName>(std::size_t index);
  ```

- The body holds a `static constexpr` array of function pointers indexed from 0.
- The function returns the pointer; the **caller** is responsible for invoking it.
- For any callee not yet ported, add a dummy stub with `TODO(asm-port)` following the Dummy Implementation Rules above, with signature matching the type alias.
- Include a comment mapping each index to the original token/label comment from the source.

## Hard Boundaries

- Do not add runtime dependency on files under SourceMaterial.
- Do not rewrite unrelated files.
- Do not expand scope beyond the two-label window unless required for compile correctness.
- Do not bypass `ApplesoftVariables` for fixed-address global state.
