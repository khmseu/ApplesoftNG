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
3. Include comments that are:
- inline inside the range.
- immediately preceding the first line in range and semantically attached.
4. Infer the behavior and data flow from opcodes, branch patterns, and comments.
5. Implement one primary C++ function that preserves the original assembler name as much as possible.
6. Place the function in the appropriate runtime area:
- interpreter/runtime logic: [src/core](../../src/core) and [include/core](../../include/core)
- console or machine-facing I/O behavior: [src/platform](../../src/platform) and [include/platform](../../include/platform)
7. Add a short provenance comment above the function with:
- source listing path
- start/end labels
- any normalization done to keep name valid in C++
8. If callees are not implemented yet, add dummy implementations in the same subsystem.
9. Ensure build remains green after each increment.

## Naming Rules
- Keep assembler symbol names verbatim when valid in C++.
- If a symbol is not a valid C++ identifier, minimally normalize it (for example `.` to `_`) and document original symbol in a comment.
- Prefer keeping capitalization consistent with source labels.

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
- Function location matches subsystem boundaries.
- Original names preserved or minimally normalized with mapping comments.
- Missing callees have explicit dummy implementations.
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
