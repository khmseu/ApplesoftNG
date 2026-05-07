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
3. Summarize intent in exactly 3-5 concise bullets before coding.
4. Implement one C++ function for the range.
5. If the range does not end in an `RTS`, `JMP`, or unconditional branch, preserve the fall-through into `end_label` by calling the following function at that point. Add a checklist item to confirm that the next-label fall-through is modeled explicitly.
6. Treat `MON_xyz` labels as monitor aliases for `xyz`; always implement them immediately as forwarding functions to the monitor handler.
7. Route all fixed-address global state access through `ApplesoftVariables` (`variables()` / `variables_const()` accessors). If a required fixed address is missing, add it to `ApplesoftVariables` before use.
8. Keep label-based naming as-is when legal; otherwise normalize only as needed to satisfy C++ identifier rules and document the mapping.
9. Choose destination by behavior:
- language/runtime semantics -> [src/core](../../../src/core)
- device/console/monitor I/O semantics -> [src/platform](../../../src/platform)
10. Add missing dependency stubs near the new implementation with `TODO(asm-port)` markers.
11. Build and report the exact files changed.

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
- Any fixed-address global state access in the ported slice uses `ApplesoftVariables`.

## Notes
- Prefer small, reviewable increments over broad rewrites.
- Preserve existing project conventions from [AGENTS.md](../../../AGENTS.md).
