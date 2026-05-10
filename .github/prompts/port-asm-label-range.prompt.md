---
---

Port one historical assembler segment to C++ using the repo conversion rules.

Required input:
- `start_label` (inclusive)
- `end_label` (exclusive)

Execution contract:
1. Find both labels in [SourceMaterial/Apple-II-Source-slim](../../SourceMaterial/Apple-II-Source-slim).
   - If either label is missing, invalid, or not in the same listing region, return an error that names the failing label and stop.
2. Read the range from `start_label` up to but not including `end_label`, and the comments immediately preceding the range.
3. Consult [docs/function-cross-reference.md](../../docs/function-cross-reference.md), specifically [# Function Cross Reference](../../docs/function-cross-reference.md#function-cross-reference), to identify which functions in the target range are already ported, which are placeholders, and where each one is currently implemented.
4. Provide a one-sentence summary of the behavior.
5. Implement one C++ function that reproduces the behavior.
6. If the range does not end in an `RTS`, `JMP`, or unconditional branch, model fall-through into `end_label` by calling the following function or by returning continuation state that the caller uses to invoke the next label.
7. Treat `MON_xyz` labels as aliases for `xyz` monitor handlers; always implement them immediately as forwarding functions to the corresponding monitor behavior and document the alias mapping in code comments.
8. If the range is a table of `.word LABEL` or `.word LABEL-1` entries, convert it to a lookup function `<TableName>_fn <TableName>(std::size_t index)` that returns a function pointer the caller invokes. Declare a `using <TableName>_fn = ...` type alias. The `-1` offset is a 6502 RTS-dispatch artifact; omit it in C++. Add stub implementations for any callee not yet ported.
9. Route all fixed-address global state access through `ApplesoftVariables` (`variables()` / `variables_const()` accessors); if a fixed address is missing there, add it before using it.
10. Preserve assembler naming unless it conflicts with C++ identifier rules.
11. Place implementation in correct subsystem:
- [src/core](../../src/core) for interpreter/runtime behavior
- [src/platform](../../src/platform) for console/monitor behavior
12. If required callees are missing, add dummy implementations with `TODO(asm-port)`.
13. Update [docs/function-cross-reference.md](../../docs/function-cross-reference.md) so the new implementation and current stub/real state are captured.
14. Build and report status.

Response format:
- Labels and source file located.
- Behavior summary.
- Files changed.
- Any stubs created.
- Build result.
