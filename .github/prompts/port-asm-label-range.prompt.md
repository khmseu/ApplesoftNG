---
---

# Port one historical assembler segment to C++ using the repo conversion rules

Required input:

- `start_label` (inclusive)
- `end_label` (exclusive)

## Execution contract

### Label Validation & Analysis

1. Find both labels in [SourceMaterial/Apple-II-Source-slim](../../SourceMaterial/Apple-II-Source-slim).
   - If either label is missing, invalid, or not in the same listing region, return an error that names the failing label and stop.
2. Read the range from `start_label` up to but not including `end_label`, and the comments immediately preceding the range.
3. **Identify 16-bit pointer candidates in the range before coding** (priority analysis):
   - indirect addressing uses like `($NN),Y` and `($NN,X)`
   - split-byte address loads/stores (`#<label` and `#>label`)
   - low-byte add and high-byte carry propagation chains (`ADC` + `INC` patterns)
4. Plan one unified C++ representation per pointer candidate—choose based on these criteria: use a direct **pointer variable** for simple memory access, a **wrapper class** for encapsulation and bounds checking, or a **typedef** for semantic aliasing. Do not use separate low/high byte locals.
5. Consult [docs/function-cross-reference.md](../../docs/function-cross-reference.md) to identify which functions in the target range are already ported, which are placeholders, and where each one is currently implemented.
6. Provide a one-sentence summary of the behavior.

### Implementation

1. Implement one C++ function that reproduces the behavior.
2. If the range does not end in an `RTS`, `JMP`, or unconditional branch, model fall-through into `end_label` by calling the following function or by returning continuation state that the caller uses to invoke the next label.
3. All symbols defined in monitor source files (under [`SourceMaterial/Apple-II-Source-slim/src/system/monitor`](../../SourceMaterial/Apple-II-Source-slim/src/system/monitor)) carry a virtual `MON_` prefix. A label `xyz` in a monitor listing is always named `MON_xyz` in C++. When the range references `MON_xyz`, look up label `xyz` in the monitor listings and implement the C++ function as `MON_xyz`.
4. All symbols defined in applesoft source files (under [`SourceMaterial/Apple-II-Source-slim/src/system/applesoft`](../../SourceMaterial/Apple-II-Source-slim/src/system/applesoft)) carry a virtual `AS_` prefix. A label `xyz` in an applesoft listing is always named `AS_xyz` in C++. When the range references `AS_xyz`, look up label `xyz` in the applesoft listings and implement the C++ function as `AS_xyz`.
5. If the range is a table of `.word LABEL` or `.word LABEL-1` entries, convert it to a lookup function `<TableName>_fn <TableName>(std::size_t index)` that returns a function pointer the caller invokes. Declare a `using <TableName>_fn = ...` type alias. The `-1` offset is a 6502 RTS-dispatch artifact; omit it in C++. Add stub implementations for any callee not yet ported.
6. Route all fixed-address global state access through `ApplesoftVariables` (`variables()` / `variables_const()` accessors); if a fixed address is missing there, add it before using it.
7. When a fixed-address byte pair is one logical pointer, represent it as one conceptual pointer variable in C++ (or explicit pointer abstraction), and model carry-chain updates as unified pointer arithmetic.
8. Route all $C000-$CFFF reads/writes through an `IOPorts`-style companion class with named constants and single-byte accessors; do not implement actual device semantics yet.
9. If storage is dual-use (integer in one path, pointer in another), use and document an explicit dual-use representation (`union`, `std::variant`, or dedicated wrapper).
10. Preserve assembler naming unless it conflicts with C++ identifier rules.
11. Place implementation in correct subsystem:
    - [src/core](../../src/core) for interpreter/runtime behavior
    - [src/platform](../../src/platform) for console/monitor behavior

12. If required callees are missing, add dummy implementations with `TODO(asm-port)`.

### Finalization

1. Update [docs/function-cross-reference.md](../../docs/function-cross-reference.md) so the new implementation and current stub/real state are captured.
2. Add or update function-scoped `AS_Labels` claim comments following [.github/skills/writing-claims/SKILL.md](../skills/writing-claims/SKILL.md).
3. Build and report status.

## Response format

- Labels and source file located.
- Behavior summary.
- Pointer candidates identified and how each was lifted/unified.
- Files changed.
- Any stubs created.
- Whether `AS_Labels` claims were added/updated per [.github/skills/writing-claims/SKILL.md](../skills/writing-claims/SKILL.md).
- Build result.
