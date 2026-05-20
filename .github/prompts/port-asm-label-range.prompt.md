---
---

# Port one historical assembler segment to C++

Use the canonical conversion rules in [.github/instructions/asm-to-cpp.instructions.md](../instructions/asm-to-cpp.instructions.md).

Required input:

- `start_label` (inclusive)
- `end_label` (exclusive)

## Execution contract

### Label Validation & Analysis

1. Locate both labels in [SourceMaterial/Combo/asrom.lst](../../SourceMaterial/Combo/asrom.lst).
2. If either label is missing, return an error naming the missing label and stop.
3. Read the bounded window (`start_label` inclusive, `end_label` exclusive), including immediately preceding comments.
4. Provide a one-sentence summary of behavior before implementation.

### Implementation

1. Implement the range strictly according to [.github/instructions/asm-to-cpp.instructions.md](../instructions/asm-to-cpp.instructions.md).
2. Keep the full `start_label`..`end_label` window covered, including split-into-multiple-functions behavior when needed.

### Finalization

1. Satisfy the Output Checklist in [.github/instructions/asm-to-cpp.instructions.md](../instructions/asm-to-cpp.instructions.md).
2. Build and report status.

## Response format

- Labels and source file located.
- Behavior summary.
- Files changed.
- Any stubs created.
- Pointer candidates identified and how each was lifted/unified.
- Whether `AS_Labels` claims were added/updated.
- If split, which function maps to which claimed sub-range.
- Build result.
