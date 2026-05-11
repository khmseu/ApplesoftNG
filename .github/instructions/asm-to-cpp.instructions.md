---
applyTo: "SourceMaterial/Apple-II-Source-slim/**/*.lst,SourceMaterial/Apple-II-Source-slim/**/*.sym,src/**/*.cpp,include/**/*.hpp"
---

# Incremental Assembler To C++ Conversion

Use these instructions when converting a bounded assembler segment into C++.

## Required Inputs

- `start_label`: first label of the conversion window, inclusive.
- `end_label`: last label of the conversion window, exclusive.

Treat labels as symbols from the historical listings in [SourceMaterial/Apple-II-Source-slim](../../SourceMaterial/Apple-II-Source-slim).

## Mandatory Workflow

1. Locate both labels and confirm they are in the same source listing region.
2. Extract the range that starts at `start_label` and stops immediately before `end_label`.
3. Identify 16-bit pointer candidates in the extracted range before coding:

- indirect addressing operands such as `($NN),Y` and `($NN,X)`
- split-byte address construction such as `#<label` and `#>label` stored to adjacent bytes
- low-byte add plus carry-chain high-byte increment sequences (`ADC`/`INC` patterns)

4. For each pointer candidate, plan a single unified C++ representation (pointer or pointer abstraction) instead of separate low/high byte locals.
5. Before coding, consult [docs/function-cross-reference.md](../../docs/function-cross-reference.md) and the [# Function Cross Reference](../../docs/function-cross-reference.md#function-cross-reference) section to determine which functions in this window are already ported, which are still stubs/placeholders, and where each function is implemented.
6. Include comments that are:

- inline inside the range.
- immediately preceding the first line in range and directly describing the range behavior.

7. Infer the behavior and data flow from opcodes, branch patterns, and comments.
   - If the source slice does not end in an unconditional transfer (`RTS`, `JMP`, or unconditional branch), it falls through into the next label. Model that fall-through in C++ by calling the following function at that point, or by returning state that the caller uses to invoke the next label.

- If a label is `MON_xyz`, treat it as an alias for monitor label `xyz` and implement it immediately as a forwarder function (do not leave alias-only comments without code).

8. Implement one primary C++ function that preserves the original assembler name as much as possible.
9. Place the function in the appropriate runtime area:

- interpreter/runtime logic: [src/core](../../src/core) and [include/core](../../include/core)
- console or machine-facing I/O behavior: [src/platform](../../src/platform) and [include/platform](../../include/platform)

10. Add a short provenance comment above the function with:

- source listing path
- start/end labels
- any normalization done to keep name valid in C++

11. If callees are not implemented yet, add dummy implementations in the same subsystem.
12. Update [docs/function-cross-reference.md](../../docs/function-cross-reference.md) after the port so it reflects new implementations and current stub/real status.
13. Ensure build remains green after each increment.

## 16-bit Pointer Synthesis Rules

- Do not model split-byte pointer flows as two unrelated `uint8_t` locals when they represent one logical address.
- Lift split-byte pointer flows to one unified pointer-oriented representation in C++ (raw pointer, typed wrapper, or equivalent abstraction).
- Translate carry-chain pointer arithmetic to one operation on the unified representation (for example `ptr += offset`) instead of open-coded low/high byte math.
- For dual-use zero-page pairs (sometimes integer, sometimes pointer), use one explicit dual-use representation (for example `union`, `std::variant`, or a dedicated wrapper) and document why.
- For absolute memory references that are not fixed-address state, prefer a consistent base-memory mapping model and index from it.
- If an absolute address names a translated C++ object or a fixed-address zero-page variable, use the translated object or `ApplesoftVariables` accessor instead of a raw memory index.

## Naming Rules

- Keep assembler symbol names verbatim when valid in C++.
- If a symbol is not a valid C++ identifier, minimally normalize it (for example `.` to `_`) and document original symbol in a comment.
- Prefer keeping capitalization consistent with source labels.
- Treat `MON_xyz` labels as monitor aliases for `xyz`; always implement them immediately as direct forwarders to the corresponding monitor handler.

## Fixed-Address Variable Rules

- Route all fixed-address state reads/writes through `ApplesoftVariables` (`variables()` / `variables_const()` and their byte/word accessors).
- Do not mirror fixed addresses with separate globals, file-scope statics, or ad-hoc structs when `ApplesoftVariables` already represents them.
- When a needed fixed address is missing, add it to `ApplesoftVariables` first, then use the accessors from ported code.
- When a fixed-address pair forms one logical pointer, read/write it through one conceptual variable in the ported function (for example via word accessors or a dedicated pointer abstraction), not duplicated low/high temporaries.

## Dummy Implementation Rules

- Create stubs only for missing dependencies required by the converted function.
- Use the original assembler-like name for stub function names.
- Add `TODO(asm-port)` in each stub with the source label to be ported later.
- Return neutral values by type:
  - `void`: empty body
  - pointers: `nullptr`
  - arithmetic types: `0`
  - `bool`: `false`
  - class/struct values: `{}`

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
