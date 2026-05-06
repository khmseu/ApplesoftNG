---
---

Port one historical assembler segment to C++ using the repo conversion rules.

Required input:
- `start_label` (inclusive)
- `end_label` (exclusive)

Execution contract:
1. Find both labels in [SourceMaterial/Apple-II-Source-slim](../../SourceMaterial/Apple-II-Source-slim).
2. Read the range from `start_label` up to but not including `end_label`, and the comments immediately preceding the range.
3. Explain the behavior briefly.
4. Implement one C++ function that reproduces the behavior.
   - If the range does not end in an `RTS`, `JMP`, or unconditional branch, it falls through into the next label (`end_label`). Model that fall-through in C++ by calling the following function at that point, or by returning continuation state that the caller uses to invoke the next label.
   - Treat `MON_xyz` labels as aliases for `xyz` monitor handlers; implement them by forwarding to the corresponding monitor behavior and document the alias mapping in code comments.
   - **Function address table special case**: if the range is a table of `.word LABEL` or `.word LABEL-1` entries, convert it to a lookup function `<TableName>_fn <TableName>(std::size_t index)` that returns a function pointer the caller invokes. Declare a `using <TableName>_fn = ...` type alias. The `-1` offset is a 6502 RTS-dispatch artifact; omit it in C++. Add stub implementations for any callee not yet ported.
5. Preserve assembler naming as much as possible.
6. Place implementation in correct subsystem:
- [src/core](../../src/core) for interpreter/runtime behavior
- [src/platform](../../src/platform) for console/monitor behavior
7. If required callees are missing, add dummy implementations with `TODO(asm-port)`.
8. Build and report status.

Response format:
- Labels and source file located.
- Behavior summary.
- Files changed.
- Any stubs created.
- Build result.
