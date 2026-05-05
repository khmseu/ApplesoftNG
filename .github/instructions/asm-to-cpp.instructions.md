---
applyTo: "SourceMaterial/Apple-II-Source-slim/**/*.lst,SourceMaterial/Apple-II-Source-slim/**/*.sym,src/**/*.cpp,include/**/*.hpp"
---

# Incremental Assembler To C++ Conversion

Use these instructions when converting a bounded assembler segment into C++.

## Required Inputs
- `start_label`: first label of the conversion window.
- `end_label`: last label of the conversion window.

Treat labels as symbols from the historical listings in [SourceMaterial/Apple-II-Source-slim](../../SourceMaterial/Apple-II-Source-slim).

## Mandatory Workflow
1. Locate both labels and confirm they are in the same source listing region.
2. Extract the inclusive range between `start_label` and `end_label`.
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

## Hard Boundaries
- Do not add runtime dependency on files under SourceMaterial.
- Do not rewrite unrelated files.
- Do not expand scope beyond the two-label window unless required for compile correctness.
