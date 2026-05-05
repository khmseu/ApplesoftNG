---
mode: ask
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
