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
3. Identify 16-bit pointer candidates in the window before coding:
   - indirect forms such as `($NN),Y` and `($NN,X)`
   - split-byte construction with `#<label` / `#>label`
   - carry-chain pointer arithmetic (`ADC` on low byte with carry into high byte)

4. For each candidate, plan one unified C++ representation (pointer or pointer abstraction). Do not keep low/high bytes as separate independent locals when they represent one pointer.
5. Consult [docs/function-cross-reference.md](../../../docs/function-cross-reference.md), especially [# Function Cross Reference](../../../docs/function-cross-reference.md#function-cross-reference), to identify existing implementations, current stub placeholders, and source locations for functions in the window.
6. Summarize intent in exactly 3-5 concise bullets before coding.
7. Implement one C++ function for the range.
8. If the range does not end in an `RTS`, `JMP`, or unconditional branch, preserve the fall-through into `end_label` by calling the following function at that point. Add a checklist item to confirm that the next-label fall-through is modeled explicitly.
9. Treat `MON_xyz` labels as monitor aliases for `xyz`; always implement them immediately as forwarding functions to the monitor handler.
10. Route all fixed-address global state access through `ApplesoftVariables` (`variables()` / `variables_const()` accessors). If a required fixed address is missing, add it to `ApplesoftVariables` before use.
11. For fixed-address byte pairs that form pointers, read/write them through one conceptual pointer variable (or an explicit pointer abstraction), and lift carry-chain updates to unified pointer arithmetic.
12. For dual-use integer/pointer storage, use an explicit representation (`union`, `std::variant`, or dedicated wrapper) and document the rationale.
13. Keep label-based naming as-is when legal; otherwise normalize only as needed to satisfy C++ identifier rules and document the mapping.
14. Choose destination by behavior:

- language/runtime semantics -> [src/core](../../../src/core)
- device/console/monitor I/O semantics -> [src/platform](../../../src/platform)

15. Add missing dependency stubs near the new implementation with `TODO(asm-port)` markers.
16. Update [docs/function-cross-reference.md](../../../docs/function-cross-reference.md) to reflect newly ported functions and updated stub/real status.
17. Build and report the exact files changed.

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
- Pointer candidates in the window are identified and mapped to unified C++ representations.
- Carry-chain pointer updates are modeled as unified pointer arithmetic.
- Any dual-use pointer/integer storage is explicit and documented.
- New function compiles.
- Any unresolved dependencies are represented by dummy implementations.
- No runtime reads from SourceMaterial.
- Any fixed-address global state access in the ported slice uses `ApplesoftVariables`.
- [docs/function-cross-reference.md](../../../docs/function-cross-reference.md) is updated for the ported window.

## Notes

- Prefer small, reviewable increments over broad rewrites.
- Preserve existing project conventions from [AGENTS.md](../../../AGENTS.md).
