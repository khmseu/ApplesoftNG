# asm-incremental-port

## Purpose
Incrementally port Applesoft historical assembler ranges into C++ using a strict two-label conversion window.

## Trigger Phrases
- "convert labels X to Y"
- "port asm range"
- "incremental assembler to c++"
- "translate this listing window"

## Inputs
- `start_label` (inclusive)
- `end_label` (exclusive)

Optional:
- preferred destination subsystem (`core` or `platform`)

## Procedure
1. Resolve both labels in the listings under [SourceMaterial/Apple-II-Source-slim/src/system](../../../SourceMaterial/Apple-II-Source-slim/src/system).
2. Read the bounded range from `start_label` up to but not including `end_label`, including inline comments and comments immediately preceding the range.
3. Summarize intent in 2-5 bullets before coding.
4. Implement one C++ function for the range.
   - If the range does not end in an `RTS`, `JMP`, or unconditional branch, preserve the fall-through into `end_label` by calling the following function at that point. Add a checklist item to confirm that the next-label fall-through is modeled explicitly.
5. Keep label-based naming as-is when legal; otherwise minimally normalize and document mapping.
6. Choose destination by behavior:
- language/runtime semantics -> [src/core](../../../src/core)
- device/console/monitor I/O semantics -> [src/platform](../../../src/platform)
7. Add missing dependency stubs near the new implementation with `TODO(asm-port)` markers.
8. Build and report the exact files changed.

## Function Address Table Pattern
If the range is a 6502 jump-table or RTS-dispatch table (a sequence of `.word LABEL` or `.word LABEL-1` entries):
- The `-1` is a 6502 RTS-dispatch artifact; use plain function pointers in C++.
- Declare `using <TableName>_fn = <return>(*)(<params>);` for the common handler signature.
- Implement `<TableName>_fn <TableName>(std::size_t index)` — a lookup that returns the pointer; the **caller** invokes it.
- Body uses a `static constexpr` array of function pointers.
- Preserve the per-entry token/label comments.
- Create stubs for every callee not yet ported, following the standard dummy implementation rules.

## Definition Of Done
- Conversion is bounded by requested labels.
- Behavior and comments from the source range are reflected in code comments.
- New function compiles.
- Any unresolved dependencies are represented by dummy implementations.
- No runtime reads from SourceMaterial.

## Notes
- Prefer small, reviewable increments over broad rewrites.
- Preserve existing project conventions from [AGENTS.md](../../../AGENTS.md).
